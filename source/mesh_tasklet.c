/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#include <string.h> //memset
#include "eventOS_event_timer.h"
#include "eventOS_scheduler.h"
#include "common_functions.h"
#include "ip6string.h"  //ip6tos
#include "net_interface.h"
#include "nsdynmemLIB.h"
#include "randLIB.h"
#include "platform/arm_hal_timer.h"
#include "mesh/mesh_tasklet.h"
#include <mbed-6lowpan-adaptor/ns_sal.h>
// For tracing we need to define flag, have include and define group
#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "meshT"

/* Channel list definitions for a beacon scan
 * */
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

// currently used channel
#define SCAN_CHANNEL    CHANNEL_4

// Tasklet timer events
#define TIMER_EVENT_START_BOOTSTRAP   1

/*
 * Tasklet states.
 */
typedef enum {
    TASKLET_STATE_INIT = 0,
    TASKLET_STATE_BOOTSTRAP_STARTED,
    TASKLET_STATE_BOOTSTRAP_FAILED,
    TASKLET_STATE_BOOTSTRAP_READY
} tasklet_state_t;

/*
 * Tasklet data structure.
 */
typedef struct
{
    void (*mesh_api_cb)(mesh_connection_status_t nwk_status);
    uint32_t channel_list;
    tasklet_state_t tasklet_state;
    int8_t node_main_tasklet_id;
    int8_t network_interface_id;
} tasklet_data_str_t;

/* Heap for NanoStack */
#define MESH_HEAP_SIZE 32500
static uint8_t app_stack_heap[MESH_HEAP_SIZE + 1];
static tasklet_data_str_t *tasklet_data_ptr = NULL;

/* private function prototypes */
void send_network_state_changed(mesh_connection_status_t status);
static void app_parse_network_event(arm_event_s *event);
#define TRACE_MESH_TASKLET
#ifndef TRACE_MESH_TASKLET
#define trace_bootstrap_ready_info() ((void) 0)
#else
void trace_bootstrap_ready_info(void);
#endif

/*
 * \brief A function which will be eventually called by NanoStack OS when ever the OS has an event to deliver.
 * @param event, describes the sender, receiver and event type.
 *
 * NOTE: Interrupts requested by HW are possible during this function!
 */
void mesh_tasklet_main(arm_event_s *event)
{
    arm_library_event_type_e event_type;
    int8_t status;
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
        app_parse_network_event(event);
    break;

    case ARM_LIB_TASKLET_INIT_EVENT:
        /* Event with type EV_INIT is an initializer event of NanoStack OS.
         * The event is delivered when the NanoStack OS is running fine.
         * This event should be delivered ONLY ONCE.
         */
        tasklet_data_ptr->node_main_tasklet_id = event->receiver;

        arm_nwk_interface_configure_6lowpan_bootstrap_set(
                tasklet_data_ptr->network_interface_id, NET_6LOWPAN_ROUTER,
                NET_6LOWPAN_ND_WITH_MLE);

        arm_nwk_6lowpan_link_scan_paramameter_set(
                tasklet_data_ptr->network_interface_id,
                tasklet_data_ptr->channel_list, 5);
        //arm_nwk_6lowpan_link_nwk_id_filter_for_nwk_scan(net_rf_id, (const uint8_t*)cfg_string(global_config, "NETWORKID", "NETWORK000000000"));

        arm_nwk_link_layer_security_mode(
                tasklet_data_ptr->network_interface_id,
                NET_SEC_MODE_NO_LINK_SECURITY, 0, 0);

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
            send_network_state_changed(MESH_BOOTSTRAP_START_FAILED);
        }
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
void app_parse_network_event(arm_event_s *event)
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
            send_network_state_changed(MESH_CONNECTED);
            trace_bootstrap_ready_info();
        }
    break;
    case ARM_NWK_NWK_SCAN_FAIL:
        /* Link Layer Active Scan Fail, Stack is Already at Idle state */
        tr_debug("Link Layer Scan Fail: No Beacons\n");
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
    break;
    case ARM_NWK_IP_ADDRESS_ALLOCATION_FAIL:
        /* No ND Router at current Channel Stack is Already at Idle state */
        tr_debug("ND Scan/ GP REG fail\n");
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
    break;
    case ARM_NWK_NWK_CONNECTION_DOWN:
        /* Connection to Access point is lost wait for Scan Result */
        tr_debug("ND/RPL scan new network\n");
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
    break;
    case ARM_NWK_NWK_PARENT_POLL_FAIL:
        tasklet_data_ptr->tasklet_state = TASKLET_STATE_BOOTSTRAP_FAILED;
    break;
    case ARM_NWK_AUHTENTICATION_FAIL:
        tr_debug("Network authentication fail\n");
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
 * Inform application about network state change
 */
void send_network_state_changed(mesh_connection_status_t status)
{
    (tasklet_data_ptr->mesh_api_cb)(status);
}

/*
 * Heap error handled, called when heap problem detected.
 * Function is for-ever loop.
 */
static void mesh_tasklet_heap_error_handler(heap_fail_t event)
{
    tr_error("Heap error, ns_wrapper_heap_error_handler() %d", event);
    switch (event)
    {
    case NS_DYN_MEM_NULL_FREE:
        case NS_DYN_MEM_DOUBLE_FREE:
        case NS_DYN_MEM_ALLOCATE_SIZE_NOT_VALID:
        case NS_DYN_MEM_POINTER_NOT_VALID:
        case NS_DYN_MEM_HEAP_SECTOR_CORRUPTED:
        case NS_DYN_MEM_HEAP_SECTOR_UNITIALIZED:
        break;
    default:
        break;
    }
    while (1);
}

#ifdef TRACE_MESH_TASKLET
/*
 * Trace bootstrap information.
 */
void trace_bootstrap_ready_info()
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
}
#endif /* #define TRACE_MESH_TASKLET */

/* Public functions */
int8_t mesh_tasklet_get_ip_address(char *address, int8_t len)
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

int8_t mesh_tasklet_get_router_ip_address(char *address, int8_t len)
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

int8_t mesh_tasklet_start(mesh_interface_cb callback, int8_t nwk_interface_id)
{
    int8_t status = eventOS_event_handler_create(&mesh_tasklet_main,
            ARM_LIB_TASKLET_INIT_EVENT);
    if (status < 0)
    {
        // -1 handler already used by other tasklet
        // -2 memory allocation failure
        return status;
    }

    memset(tasklet_data_ptr, 0, sizeof(tasklet_data_str_t));
    tasklet_data_ptr->mesh_api_cb = callback;
    tasklet_data_ptr->network_interface_id = nwk_interface_id;
    tasklet_data_ptr->tasklet_state = TASKLET_STATE_INIT;
    tasklet_data_ptr->channel_list = SCAN_CHANNEL;
    tr_debug("Scan channel %d", (SCAN_CHANNEL>>2));

    return status;
}

void mesh_tasklet_process_event(void)
{
    extern void event_dispatch_cycle();
    event_dispatch_cycle();
}

void mesh_tasklet_system_init()
{
    if (tasklet_data_ptr == NULL)
    {
        /*
         * Initialization can happen only once!
         */

        ns_dyn_mem_init(app_stack_heap, MESH_HEAP_SIZE,
                mesh_tasklet_heap_error_handler, 0);
        randLIB_seed_random();
        platform_timer_enable();
        eventOS_scheduler_init();
        trace_init(); // trace system needs to be initialized right after eventOS_scheduler_init
        net_init_core();
        // memory allocation will not fail as memory was just initialized
        tasklet_data_ptr = ns_dyn_mem_alloc(sizeof(tasklet_data_str_t));
        /* initialize socket adaptation layer */
        ns_sal_init_stack();
    }
}

int8_t mesh_tasklet_network_init(char *description, int8_t device_id)
{
    return arm_nwk_interface_init(NET_INTERFACE_RF_6LOWPAN, device_id,
            description);
}
