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
#include "nsdynmemLIB.h"
#include "thread_management_if.h"
#include "include/thread_tasklet.h"
#include "include/static_config.h"
#include "include/mesh_system.h"
// For tracing we need to define flag, have include and define group
#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "m6Thread"

#define INTERFACE_NAME   "6L-THREAD"

// Tasklet timer events
#define TIMER_EVENT_START_BOOTSTRAP   1

#define INVALID_INTERFACE_ID        (-1)

/*
 * Thread tasklet states.
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
typedef struct {
    void (*mesh_api_cb)(mesh_connection_status_t nwk_status);
    uint32_t channel_list;
    tasklet_state_t tasklet_state;
    int8_t tasklet;

    net_6lowpan_mode_e operating_mode;
    int8_t nwk_if_id;
    link_configuration_s link_config;

    /** Default network ID*/
    uint8_t networkid[16];
    uint8_t extented_panid[8];
    uint32_t pan_id;
    uint32_t rfChannel;
    uint8_t scan_time;
    net_6lowpan_gp_address_mode_e address_mode;
} thread_tasklet_data_str_t;


/* Tasklet data */
static thread_tasklet_data_str_t *thread_tasklet_data_ptr = NULL;
static device_configuration_s device_configuration;

/* private function prototypes */
void thread_tasklet_main(arm_event_s *event);
void thread_tasklet_network_state_changed(mesh_connection_status_t status);
void thread_tasklet_parse_network_event(arm_event_s *event);
void thread_tasklet_configure_and_connect_to_network(void);
#define TRACE_THREAD_TASKLET
#ifndef TRACE_THREAD_TASKLET
#define thread_tasklet_trace_bootstrap_info() ((void) 0)
#else
void thread_tasklet_trace_bootstrap_info(void);
#endif

/*
 * \brief A function which will be eventually called by NanoStack OS when ever the OS has an event to deliver.
 * @param event, describes the sender, receiver and event type.
 *
 * NOTE: Interrupts requested by HW are possible during this function!
 */
void thread_tasklet_main(arm_event_s *event)
{
    arm_library_event_type_e event_type;
    event_type = (arm_library_event_type_e) event->event_type;

    if (event->sender != 0) {
        // only handling stack events
        return;
    }

    switch (event_type) {
        case ARM_LIB_NWK_INTERFACE_EVENT:
            /* This event is delivered every and each time when there is new
             * information of network connectivity.
             */
            thread_tasklet_parse_network_event(event);
            break;

        case ARM_LIB_TASKLET_INIT_EVENT:
            /* Event with type EV_INIT is an initializer event of NanoStack OS.
             * The event is delivered when the NanoStack OS is running fine.
             * This event should be delivered ONLY ONCE.
             */
            mesh_system_send_connect_event(thread_tasklet_data_ptr->tasklet);
            break;

        case ARM_LIB_SYSTEM_TIMER_EVENT:
            eventOS_event_timer_cancel(event->event_id,
                                       thread_tasklet_data_ptr->tasklet);

            if (event->event_id == TIMER_EVENT_START_BOOTSTRAP) {
                tr_debug("Restart bootstrap");
                arm_nwk_interface_up(thread_tasklet_data_ptr->nwk_if_id);
            }
            break;

        case APPLICATION_EVENT:
            if (event->event_id == APPL_EVENT_CONNECT) {
                thread_tasklet_configure_and_connect_to_network();
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
void thread_tasklet_parse_network_event(arm_event_s *event)
{
    arm_nwk_interface_status_type_e status = (arm_nwk_interface_status_type_e) event->event_data;
    tr_debug("app_parse_network_event() %d", status);
    switch (status) {
        case ARM_NWK_BOOTSTRAP_READY:
            /* Network is ready and node is connected to Access Point */
            if (thread_tasklet_data_ptr->tasklet_state != TASKLET_STATE_BOOTSTRAP_READY) {
                tr_info("Thread bootstrap ready");
                thread_tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_READY;
                thread_tasklet_trace_bootstrap_info();
                thread_tasklet_network_state_changed(MESH_CONNECTED);
            }
            break;
        case ARM_NWK_NWK_SCAN_FAIL:
            /* Link Layer Active Scan Fail, Stack is Already at Idle state */
            tr_debug("Link Layer Scan Fail: No Beacons");
            thread_tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
            break;
        case ARM_NWK_IP_ADDRESS_ALLOCATION_FAIL:
            /* No ND Router at current Channel Stack is Already at Idle state */
            tr_debug("ND Scan/ GP REG fail");
            thread_tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
            break;
        case ARM_NWK_NWK_CONNECTION_DOWN:
            /* Connection to Access point is lost wait for Scan Result */
            tr_debug("ND/RPL scan new network");
            thread_tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
            break;
        case ARM_NWK_NWK_PARENT_POLL_FAIL:
            thread_tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
            break;
        case ARM_NWK_AUHTENTICATION_FAIL:
            tr_debug("Network authentication fail");
            thread_tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
            break;
        default:
            tr_warn("Unknown event %d", status);
            break;
    }

    if (thread_tasklet_data_ptr->tasklet_state != TASKLET_STATE_BOOTSTRAP_READY) {
        // Set 5s timer for a new network scan
        eventOS_event_timer_request(TIMER_EVENT_START_BOOTSTRAP,
                                    ARM_LIB_SYSTEM_TIMER_EVENT,
                                    thread_tasklet_data_ptr->tasklet,
                                    5000);
    }
}

/*
 * \brief Configure mesh network
 *
 */
void thread_tasklet_configure_and_connect_to_network(void)
{
    int8_t status;

    thread_tasklet_data_ptr->operating_mode = NET_6LOWPAN_ROUTER;

    arm_nwk_interface_configure_6lowpan_bootstrap_set(
        thread_tasklet_data_ptr->nwk_if_id,
        thread_tasklet_data_ptr->operating_mode,
        NET_6LOWPAN_THREAD);

    arm_nwk_6lowpan_gp_address_mode(thread_tasklet_data_ptr->nwk_if_id, NET_6LOWPAN_GP16_ADDRESS, 0xffff, 1);

    // Link configuration
    thread_tasklet_data_ptr->link_config.steering_data_len = 0;
    memcpy(thread_tasklet_data_ptr->link_config.name, "Arm Powered Core", 16);
    thread_tasklet_data_ptr->link_config.panId =  THREAD_PANID;
    thread_tasklet_data_ptr->link_config.rfChannel = THREAD_RF_CHANNEL;
    thread_tasklet_data_ptr->channel_list = SCAN_CHANNEL_LIST;

    //Beacon data setting
    thread_tasklet_data_ptr->link_config.Protocol_id = 0x03;
    thread_tasklet_data_ptr->link_config.version = 1;
    memcpy(thread_tasklet_data_ptr->link_config.extended_random_mac, device_configuration.eui64, 8);
    thread_tasklet_data_ptr->link_config.extended_random_mac[0] |= 0x02;

    memcpy(thread_tasklet_data_ptr->link_config.mesh_local_ula_prefix, "\xFD\0\x0d\xb8\0\0\0\0", 8);
    const uint8_t default_net_security_key[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    memcpy(thread_tasklet_data_ptr->link_config.master_key, default_net_security_key, 16);
    thread_tasklet_data_ptr->link_config.key_rotation = 3600;
    thread_tasklet_data_ptr->link_config.key_sequence = 0;

    thread_managenet_node_init(thread_tasklet_data_ptr->nwk_if_id,
                               thread_tasklet_data_ptr->channel_list,
                               NULL /*link_filter_ptr */,
                               &device_configuration,
                               &thread_tasklet_data_ptr->link_config);

    status = arm_nwk_interface_up(thread_tasklet_data_ptr->nwk_if_id);

    if (status >= 0) {
        thread_tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_STARTED;
        tr_info("Start 6LoWPAN Thread bootstrap");
    } else {
        thread_tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
        tr_err("Bootstrap start failed, %d", status);
        thread_tasklet_network_state_changed(MESH_BOOTSTRAP_START_FAILED);
    }
}

/*
 * Inform application about network state change
 */
void thread_tasklet_network_state_changed(mesh_connection_status_t status)
{
    if (thread_tasklet_data_ptr->mesh_api_cb) {
        (thread_tasklet_data_ptr->mesh_api_cb)(status);
    }
}

/*
 * Trace bootstrap information.
 */
#ifdef TRACE_THREAD_TASKLET
void thread_tasklet_trace_bootstrap_info()
{
    link_layer_address_s app_link_address_info;
    uint8_t temp_ipv6[16];
    if (arm_net_address_get(thread_tasklet_data_ptr->nwk_if_id,
                            ADDR_IPV6_GP, temp_ipv6) == 0) {
        tr_debug("GP IPv6:");
        printf_ipv6_address(temp_ipv6);
    } else {
        tr_error("Own IP Address read fail\n");
    }

    if (arm_nwk_mac_address_read(thread_tasklet_data_ptr->nwk_if_id,
                                 &app_link_address_info) != 0) {
        tr_error("MAC Address read fail\n");
    } else {
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
#endif /* #define TRACE_THREAD_TASKLET */

int8_t thread_tasklet_connect(mesh_interface_cb callback, int8_t nwk_interface_id)
{
    int8_t re_connecting = true;
    int8_t tasklet = thread_tasklet_data_ptr->tasklet;

    if (thread_tasklet_data_ptr->nwk_if_id != INVALID_INTERFACE_ID) {
        return -3;  // already connected to network
    }

    if (thread_tasklet_data_ptr->tasklet_state == TASKLET_STATE_CREATED) {
        re_connecting = false;
    }

    memset(thread_tasklet_data_ptr, 0, sizeof(thread_tasklet_data_str_t));
    thread_tasklet_data_ptr->mesh_api_cb = callback;
    thread_tasklet_data_ptr->nwk_if_id = nwk_interface_id;
    thread_tasklet_data_ptr->tasklet_state = TASKLET_STATE_INITIALIZED;

    if (re_connecting == false) {
        thread_tasklet_data_ptr->tasklet = eventOS_event_handler_create(&thread_tasklet_main,
                        ARM_LIB_TASKLET_INIT_EVENT);
        if (thread_tasklet_data_ptr->tasklet < 0) {
            // -1 handler already used by other tasklet
            // -2 memory allocation failure
            return thread_tasklet_data_ptr->tasklet;
        }
    } else {
        thread_tasklet_data_ptr->tasklet = tasklet;
        mesh_system_send_connect_event(thread_tasklet_data_ptr->tasklet);
    }

    return thread_tasklet_data_ptr->tasklet;
}

int8_t thread_tasklet_disconnect(bool send_cb)
{
    int8_t status = -1;
    // check that module is initialized
    if (thread_tasklet_data_ptr != NULL) {
        if (thread_tasklet_data_ptr->nwk_if_id != INVALID_INTERFACE_ID) {
            status = arm_nwk_interface_down(thread_tasklet_data_ptr->nwk_if_id);
            thread_tasklet_data_ptr->nwk_if_id = INVALID_INTERFACE_ID;
            if (send_cb == true) {
                thread_tasklet_network_state_changed(MESH_DISCONNECTED);
            }
        }

        // Clear callback, it will be set again in next connect
        thread_tasklet_data_ptr->mesh_api_cb = NULL;
    }
    return status;
}

void thread_tasklet_init(void)
{
    if (thread_tasklet_data_ptr == NULL) {
        thread_tasklet_data_ptr = ns_dyn_mem_alloc(sizeof(thread_tasklet_data_str_t));
        thread_tasklet_data_ptr->tasklet_state = TASKLET_STATE_CREATED;
        thread_tasklet_data_ptr->nwk_if_id = INVALID_INTERFACE_ID;
    }
}

int8_t thread_tasklet_network_init(int8_t device_id)
{
    // TODO, read interface name from configuration
    return arm_nwk_interface_init(NET_INTERFACE_RF_6LOWPAN, device_id,
                                  INTERFACE_NAME);
}

void thread_tasklet_set_device_config(uint8_t *eui64, char *pskd)
{
    device_configuration.PSKd_ptr = ns_dyn_mem_alloc(strlen(pskd) + 1);
    strcpy(device_configuration.PSKd_ptr, pskd);
    memcpy(device_configuration.eui64, eui64, 8);
    device_configuration.leaderCap = true;
}

