/*
 * Copyright (c) 2015 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h> //memset
#include "eventOS_event_timer.h"
#include "common_functions.h"
#include "net_interface.h"
#include "ip6string.h"  //ip6tos
#include "nsdynmemLIB.h"
#include "include/nd_tasklet.h"
// For tracing we need to define flag, have include and define group
#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "m6LND"

/*
 * Channel list definitions for a beacon scan.
 * TODO: use device config api to fetch these values
 */
#define CHANNEL_1       1<<1
#define CHANNEL_2       1<<2
#define CHANNEL_3       1<<3
#define CHANNEL_4       1<<4
#define CHANNEL_5       1<<5
#define CHANNEL_6       1<<6
#define CHANNEL_7       1<<7
#define CHANNEL_8       1<<8
#define CHANNEL_9       1<<9
#define CHANNEL_10      1<<10
#define CHANNEL_11      1<<11
#define CHANNEL_12      1<<12
#define ALL_CHANNELS    0x07fff800

// TODO: proper configuration from device config api
#define CONFIGURED_SCAN_CHANNEL     CHANNEL_4
#define CONFIGURED_NODE_MODE        NET_6LOWPAN_ROUTER
#define CONFIGURED_SEC_MODE         NET_SEC_MODE_NO_LINK_SECURITY
#define CONFIGURED_INTERFACE_NAME   "6LND"

// Tasklet timer events
#define TIMER_EVENT_START_BOOTSTRAP   1

#define INVALID_INTERFACE_ID        (-1)

/*
 * Mesh tasklet states.
 */
typedef enum {
    TASKLET_STATE_CREATED = 0,
    TASKLET_STATE_INITIALIZED,
    TASKLET_STATE_BOOTSTRAP_STARTED,
    TASKLET_STATE_BOOTSTRAP_FAILED,
    TASKLET_STATE_BOOTSTRAP_READY
} tasklet_state_t;

/*
 * Mesh tasklet data structure.
 */
typedef struct
{
    void (*mesh_api_cb)(mesh_connection_status_t nwk_status);
    uint32_t channel_list;
    tasklet_state_t tasklet_state;
    net_6lowpan_mode_e mode;
    net_6lowpan_link_layer_sec_mode_e sec_mode;
    net_link_layer_psk_security_info_s psk_sec_info;
    int8_t node_main_tasklet_id;
    int8_t network_interface_id;
} tasklet_data_str_t;

/* Tasklet data */
static tasklet_data_str_t *tasklet_data_ptr = NULL;

/* private function prototypes */
void nd_tasklet_main(arm_event_s *event);
void nd_tasklet_network_state_changed(mesh_connection_status_t status);
void nd_tasklet_parse_network_event(arm_event_s *event);
void nd_tasklet_configure_network(void);
#define TRACE_ND_TASKLET
#ifndef TRACE_ND_TASKLET
#define nd_tasklet_trace_bootstrap_info() ((void) 0)
#else
void nd_tasklet_trace_bootstrap_info(void);
#endif

/*
 * \brief A function which will be eventually called by NanoStack OS when ever the OS has an event to deliver.
 * @param event, describes the sender, receiver and event type.
 *
 * NOTE: Interrupts requested by HW are possible during this function!
 */
void nd_tasklet_main(arm_event_s *event)
{
    arm_library_event_type_e event_type;
    event_type = (arm_library_event_type_e) event->event_type;

    if (event->sender != 0)
    {
        // only handling stack events
        return;
    }

    switch (event_type)
    {
    case ARM_LIB_NWK_INTERFACE_EVENT:
        /* This event is delivered every and each time when there is new
         * information of network connectivity.
         */
        nd_tasklet_parse_network_event(event);
    break;

    case ARM_LIB_TASKLET_INIT_EVENT:
        /* Event with type EV_INIT is an initializer event of NanoStack OS.
         * The event is delivered when the NanoStack OS is running fine.
         * This event should be delivered ONLY ONCE.
         */
        tasklet_data_ptr->node_main_tasklet_id = event->receiver;
        nd_tasklet_configure_network();
    break;

    case ARM_LIB_SYSTEM_TIMER_EVENT:
        eventOS_event_timer_cancel(event->event_id,
                tasklet_data_ptr->node_main_tasklet_id);

        if (event->event_id == TIMER_EVENT_START_BOOTSTRAP)
        {
            tr_debug("Restart bootstrap");
            arm_nwk_interface_up(tasklet_data_ptr->network_interface_id);
        }
    break;

    default:
        break;
    } // switch(event_type)
}

/**
 * \brief Network state event handler.
 * \param event show network start response or current network state.
 *
 * - ARM_NWK_BOOTSTRAP_READY: Save NVK persistent data to NVM and Net role
 * - ARM_NWK_NWK_SCAN_FAIL: Link Layer Active Scan Fail, Stack is Already at Idle state
 * - ARM_NWK_IP_ADDRESS_ALLOCATION_FAIL: No ND Router at current Channel Stack is Already at Idle state
 * - ARM_NWK_NWK_CONNECTION_DOWN: Connection to Access point is lost wait for Scan Result
 * - ARM_NWK_NWK_PARENT_POLL_FAIL: Host should run net start without any PAN-id filter and all channels
 * - ARM_NWK_AUHTENTICATION_FAIL: Pana Authentication fail, Stack is Already at Idle state
 */
void nd_tasklet_parse_network_event(arm_event_s *event)
{
    arm_nwk_interface_status_type_e status = (arm_nwk_interface_status_type_e) event->event_data;
    tr_debug("app_parse_network_event() %d", status);
    switch (status)
    {
    case ARM_NWK_BOOTSTRAP_READY:
        /* Network is ready and node is connected to Access Point */
        if (tasklet_data_ptr->tasklet_state != TASKLET_STATE_BOOTSTRAP_READY)
        {
            tr_info("Network bootstrap ready");
            tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_READY;
            nd_tasklet_trace_bootstrap_info();
            nd_tasklet_network_state_changed(MESH_CONNECTED);
        }
    break;
    case ARM_NWK_NWK_SCAN_FAIL:
        /* Link Layer Active Scan Fail, Stack is Already at Idle state */
        tr_debug("Link Layer Scan Fail: No Beacons");
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
    break;
    case ARM_NWK_IP_ADDRESS_ALLOCATION_FAIL:
        /* No ND Router at current Channel Stack is Already at Idle state */
        tr_debug("ND Scan/ GP REG fail");
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
    break;
    case ARM_NWK_NWK_CONNECTION_DOWN:
        /* Connection to Access point is lost wait for Scan Result */
        tr_debug("ND/RPL scan new network");
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
    break;
    case ARM_NWK_NWK_PARENT_POLL_FAIL:
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
    break;
    case ARM_NWK_AUHTENTICATION_FAIL:
        tr_debug("Network authentication fail");
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
    break;
    default:
        tr_warn("Unknown event %d", status);
    break;
    }

    if (tasklet_data_ptr->tasklet_state != TASKLET_STATE_BOOTSTRAP_READY)
    {
        // Set 5s timer for new network scan
        eventOS_event_timer_request(TIMER_EVENT_START_BOOTSTRAP,
                ARM_LIB_SYSTEM_TIMER_EVENT,
                tasklet_data_ptr->node_main_tasklet_id,
                5000);
    }
}

/*
 * \brief Configure mesh network
 *
 */
void nd_tasklet_configure_network(void)
{
    int8_t status;

    // configure bootstrap
    arm_nwk_interface_configure_6lowpan_bootstrap_set(
            tasklet_data_ptr->network_interface_id, tasklet_data_ptr->mode,
            NET_6LOWPAN_ND_WITH_MLE);

    // configure link layer security
    arm_nwk_link_layer_security_mode(
            tasklet_data_ptr->network_interface_id,
            tasklet_data_ptr->sec_mode, tasklet_data_ptr->psk_sec_info.key_id, &tasklet_data_ptr->psk_sec_info);

    // configure scan parameters
    arm_nwk_6lowpan_link_scan_paramameter_set(
            tasklet_data_ptr->network_interface_id,
            tasklet_data_ptr->channel_list, 5);

    // Configure scan options (NULL disables filter)
    arm_nwk_6lowpan_link_nwk_id_filter_for_nwk_scan(
            tasklet_data_ptr->network_interface_id, NULL);

    status = arm_nwk_interface_up(tasklet_data_ptr->network_interface_id);
    if (status >= 0)
    {
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_STARTED;
        tr_info("6LoWPAN IP Bootstrap started");
    }
    else
    {
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
        tr_err("Bootstrap start failed, %d", status);
        nd_tasklet_network_state_changed(MESH_BOOTSTRAP_START_FAILED);
    }
}

/*
 * Inform application about network state change
 */
void nd_tasklet_network_state_changed(mesh_connection_status_t status)
{
    if (tasklet_data_ptr->mesh_api_cb) {
        (tasklet_data_ptr->mesh_api_cb)(status);
    }
}

/*
 * Trace bootstrap information.
 */
#ifdef TRACE_ND_TASKLET
void nd_tasklet_trace_bootstrap_info()
{
    network_layer_address_s app_nd_address_info;
    link_layer_address_s app_link_address_info;
    uint8_t temp_ipv6[16];
    if (arm_nwk_nd_address_read(tasklet_data_ptr->network_interface_id,
            &app_nd_address_info) != 0)
    {
        tr_error("ND Address read fail");
    }
    else
    {
        tr_debug("ND Access Point:");
        printf_ipv6_address(app_nd_address_info.border_router);

        tr_debug("ND Prefix 64:");
        printf_array(app_nd_address_info.prefix, 8);

        if (arm_net_address_get(tasklet_data_ptr->network_interface_id,
                ADDR_IPV6_GP, temp_ipv6) == 0)
        {
            tr_debug("GP IPv6:");
            printf_ipv6_address(temp_ipv6);
        }
    }

    if (arm_nwk_mac_address_read(tasklet_data_ptr->network_interface_id,
            &app_link_address_info) != 0)
    {
        tr_error("MAC Address read fail\n");
    }
    else
    {
        uint8_t temp[2];
        tr_debug("MAC 16-bit:");
        common_write_16_bit(app_link_address_info.PANId, temp);
        tr_debug("PAN ID:");
        printf_array(temp, 2);
        tr_debug("MAC 64-bit:");
        printf_array(app_link_address_info.mac_long, 8);
        tr_debug("IID (Based on MAC 64-bit address):");
        printf_array(app_link_address_info.iid_eui64, 8);
    }
    tr_debug("traced bootstrap info");
}
#endif /* #define TRACE_ND_TASKLET */

/* Public functions */
int8_t nd_tasklet_get_ip_address(char *address, int8_t len)
{
    uint8_t binary_ipv6[16];

    if ((len >= 40) && (0 == arm_net_address_get(
            tasklet_data_ptr->network_interface_id, ADDR_IPV6_GP, binary_ipv6)))
    {
        ip6tos(binary_ipv6, address);
        //tr_debug("IP address: %s", address);
        return 0;
    }
    else
    {
        return -1;
    }
}

int8_t nd_tasklet_get_router_ip_address(char *address, int8_t len)
{
    network_layer_address_s nd_address;

    if ((len >= 40) && (0 == arm_nwk_nd_address_read(
            tasklet_data_ptr->network_interface_id, &nd_address)))
    {
        ip6tos(nd_address.border_router, address);
        //tr_debug("Router IP address: %s", address);
        return 0;
    }
    else
    {
        return -1;
    }
}

int8_t nd_tasklet_connect(mesh_interface_cb callback, int8_t nwk_interface_id)
{
    int8_t status = 0;
    int8_t re_connecting = true;

    if (tasklet_data_ptr->network_interface_id != INVALID_INTERFACE_ID)
    {
        return -3;  // already connected to network
    }

    if (tasklet_data_ptr->tasklet_state == TASKLET_STATE_CREATED)
    {
        re_connecting = false;
    }

    memset(tasklet_data_ptr, 0, sizeof(tasklet_data_str_t));
    tasklet_data_ptr->mesh_api_cb = callback;
    tasklet_data_ptr->network_interface_id = nwk_interface_id;
    tasklet_data_ptr->tasklet_state = TASKLET_STATE_INITIALIZED;

    //TODO: Fetch these values from device config api
    tasklet_data_ptr->channel_list = CONFIGURED_SCAN_CHANNEL;
    tasklet_data_ptr->mode = CONFIGURED_NODE_MODE;
    tasklet_data_ptr->sec_mode = CONFIGURED_SEC_MODE;
    //tasklet_data_ptr->psk_sec_info.key_id = 0;

    tr_debug("Scan channel %d", (CONFIGURED_SCAN_CHANNEL>>2));

    if (re_connecting == false)
    {
        int8_t status = eventOS_event_handler_create(&nd_tasklet_main,
                ARM_LIB_TASKLET_INIT_EVENT);
        if (status < 0)
        {
            // -1 handler already used by other tasklet
            // -2 memory allocation failure
            return status;
        }
    }
    else
    {
        nd_tasklet_configure_network();
    }

    return status;
}

int8_t nd_tasklet_disconnect()
{
    int8_t status = -1;
    if (tasklet_data_ptr->network_interface_id != INVALID_INTERFACE_ID)
    {
        status = arm_nwk_interface_down(tasklet_data_ptr->network_interface_id);
        tasklet_data_ptr->network_interface_id = INVALID_INTERFACE_ID;
    }
    // in any case inform client that we are in disconnected state
    nd_tasklet_network_state_changed(MESH_DISCONNECTED);
    return status;
}

void nd_tasklet_init()
{
    if (tasklet_data_ptr == NULL)
    {
        // memory allocation will not fail as memory was just initialized
        tasklet_data_ptr = ns_dyn_mem_alloc(sizeof(tasklet_data_str_t));
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_CREATED;
        tasklet_data_ptr->network_interface_id = INVALID_INTERFACE_ID;
    }
}

int8_t nd_tasklet_network_init(int8_t device_id)
{
    // TODO, read interface name from configuration
    return arm_nwk_interface_init(NET_INTERFACE_RF_6LOWPAN, device_id,
            CONFIGURED_INTERFACE_NAME);
}

