/*
 * Copyright (c) 2014 ARM. All rights reserved.
 */

#include "eventOS_event_timer.h"
#include "ns_address.h"
#include "socket_api.h"
#include "common_functions.h"
#include "ip6string.h"  //ip6tos
#include "net_interface.h"
#include "nsdynmemLIB.h"
#include "randLIB.h"
#include "platform/arm_hal_timer.h"
#include "eventOS_scheduler.h"
#include "mesh/mesh_tasklet.h"

// For tracing we need to define flag, have include and define group
#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "mTask"

int8_t network_interface_id = -1;
static link_layer_address_s app_link_address_info;
static network_layer_address_s app_nd_address_info;

uint8_t access_point_status = 0;
/** Configurable channel list for beacon scan*/
//static uint32_t channel_list = 0x00001000; // channel 12
static uint32_t channel_list = 0x00000010; // channel 4, sub GHz
//static uint32_t channel_list = 0x00000800; // channel 11
//static uint32_t channel_list = 0x07fff800; // all channels

/* Heap for NanoStack */
#define APP_DEFINED_HEAP_SIZE 32500
static uint8_t app_stack_heap[APP_DEFINED_HEAP_SIZE +1];


static int8_t node_main_tasklet_id = -1;
static void (*mesh_api_cb)(mesh_status_t nwk_status) = NULL;

/* private function prototypes */
void send_network_state_changed(mesh_status_t status);
static void app_parse_network_event(arm_event_s *event);

/*
 * \brief A function which will be eventually called by NanoStack OS when ever the OS has an event to deliver.
 * @param event, describes the sender, receiver and event type.
 *
 * NOTE: Interrupts requested by HW are possible during this function!
 */
void mesh_tasklet_main(arm_event_s *event) {
	int8_t retval;
	if (event->sender == 0)
	{
		arm_library_event_type_e event_type;
		event_type = (arm_library_event_type_e) event->event_type;
		switch (event_type)
		{
			//This event is delivered every and each time when there is new information of network connectivity.
			case ARM_LIB_NWK_INTERFACE_EVENT:
				/* Network Event state event handler */
				app_parse_network_event(event);
				break;

			case ARM_LIB_TASKLET_INIT_EVENT:
				/*Event with type EV_INIT is an initialiser event of NanoStack OS.
				 * The event is delivered when the NanoStack OS is running fine. This event should be delivered ONLY ONCE.
				 */
				node_main_tasklet_id = event->receiver;
				if (arm_nwk_mac_address_read(network_interface_id, &app_link_address_info)!= 0)
				{
					tr_error("MAC Address read fail");
				}

				arm_nwk_interface_configure_6lowpan_bootstrap_set(network_interface_id, NET_6LOWPAN_ROUTER, NET_6LOWPAN_ND_WITH_MLE);

				arm_nwk_6lowpan_link_scan_paramameter_set(network_interface_id, channel_list, 5);
				//arm_nwk_6lowpan_link_nwk_id_filter_for_nwk_scan(net_rf_id, (const uint8_t*)cfg_string(global_config, "NETWORKID", "NETWORK000000000"));

				arm_nwk_link_layer_security_mode(network_interface_id, NET_SEC_MODE_NO_LINK_SECURITY, 0, 0);

				retval = arm_nwk_interface_up(network_interface_id);
				if (retval != 0)
				{
					tr_warn("Start Fail code %d", retval);
				}
				else
				{
					tr_info("6LoWPAN IP Bootstrap started");
				}
				break;

				case ARM_LIB_SYSTEM_TIMER_EVENT:
					eventOS_event_timer_cancel(event->event_id, node_main_tasklet_id);

					if(event->event_id == 1)
					{
						if(arm_nwk_interface_up(network_interface_id) == 0)
						{
							tr_debug("Start Network bootstrap Again");
						}

					}
				break;
				default:
				break;
		}
	}
}

/**
  * \brief Network state event handler.
  * \param event show network start response or current network state.
  *
  * - ARM_NWK_BOOTSTRAP_READY: Save NVK peristant data to NVM and Net role
  * - ARM_NWK_NWK_SCAN_FAIL: Link Layer Active Scan Fail, Stack is Already at Idle state
  * - ARM_NWK_IP_ADDRESS_ALLOCATION_FAIL: No ND Router at current Channel Stack is Already at Idle state
  * - ARM_NWK_NWK_CONNECTION_DOWN: Connection to Access point is lost wait for Scan Result
  * - ARM_NWK_NWK_PARENT_POLL_FAIL: Host should run net start without any PAN-id filter and all channels
  * - ARM_NWK_AUHTENTICATION_FAIL: Pana Authentication fail, Stack is Already at Idle state
  */
void app_parse_network_event(arm_event_s *event )
{
	arm_nwk_interface_status_type_e status = (arm_nwk_interface_status_type_e)event->event_data;
	switch (status)
	{
		  case ARM_NWK_BOOTSTRAP_READY:
			  /* Network is ready and node is connect to Access Point */
			  if(access_point_status==0)
			  {
				  uint8_t temp_ipv6[16];
				  tr_info("Network bootstrap ready");
				  access_point_status=1;
				  send_network_state_changed(MESH_CONNECTED);

				  if( arm_nwk_nd_address_read(network_interface_id,&app_nd_address_info) != 0)
				  {
					  tr_error("ND Address read fail");
				  }
				  else
				  {
					  tr_debug("ND Access Point:");
						printf_ipv6_address(app_nd_address_info.border_router);

						tr_debug("ND Prefix 64:");
						printf_array(app_nd_address_info.prefix, 8);

						if(arm_net_address_get(network_interface_id,ADDR_IPV6_GP,temp_ipv6) == 0)
						{
						    tr_debug("GP IPv6:");
							printf_ipv6_address(temp_ipv6);
						}
					}

				  if( arm_nwk_mac_address_read(network_interface_id,&app_link_address_info) != 0)
					{
						tr_error("MAC Address read fail\n");
					}
					else
					{
						uint8_t temp[2];
						tr_debug("MAC 16-bit:");
						common_write_16_bit(app_link_address_info.PANId,temp);
						tr_debug("PAN ID:");
						printf_array(temp, 2);
						tr_debug("MAC 64-bit:");
						printf_array(app_link_address_info.mac_long, 8);
						tr_debug("IID (Based on MAC 64-bit address):");
						printf_array(app_link_address_info.iid_eui64, 8);
					}
			  }

			  break;
		  case ARM_NWK_NWK_SCAN_FAIL:
			  /* Link Layer Active Scan Fail, Stack is Already at Idle state */
		      tr_debug("Link Layer Scan Fail: No Beacons\n");
			  access_point_status=0;
			  break;
		  case ARM_NWK_IP_ADDRESS_ALLOCATION_FAIL:
			  /* No ND Router at current Channel Stack is Already at Idle state */
		      tr_debug("ND Scan/ GP REG fail\n");
			  access_point_status=0;
			  break;
		  case ARM_NWK_NWK_CONNECTION_DOWN:
			  /* Connection to Access point is lost wait for Scan Result */
		      tr_debug("ND/RPL scan new network\n");
			  access_point_status=0;
			  break;
		  case ARM_NWK_NWK_PARENT_POLL_FAIL:
			  access_point_status=0;
			  break;
		  case ARM_NWK_AUHTENTICATION_FAIL:
		      tr_debug("Network authentication fail\n");
			  access_point_status=0;
			  break;
		  default:
			  debug_hex(status);
			  tr_debug("Unknow event");
			  break;
	 }

	if(access_point_status == 0)
	{
		//Set Timer for new network scan
		eventOS_event_timer_request(1, ARM_LIB_SYSTEM_TIMER_EVENT,node_main_tasklet_id,5000); // 5 sec timer started
	}
}

/*
 * Inform application about network state change
 */
void send_network_state_changed(mesh_status_t status)
{
    if (NULL != mesh_api_cb)
    {
        (mesh_api_cb)(status);
    }
}

static void mesh_tasklet_heap_error_handler(heap_fail_t event)
{
    tr_error("Error, ns_wrapper_heap_error_handler() %d", event);
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
    while(1);
}

/* Public functions */
int8_t mesh_tasklet_get_ip_address(char *address, int8_t len)
{
    uint8_t binary_ipv6[16];

    if ((len >= 40) && (0 == arm_net_address_get(network_interface_id, ADDR_IPV6_GP, binary_ipv6)))
    {
        ip6tos(binary_ipv6, address);
        tr_info("IP address: %s", address);
        return 0;
    } else
    {
        return -1;
    }
}

int8_t mesh_tasklet_get_router_ip_address(char *address, int8_t len)
{
    network_layer_address_s nd_address;

    if ((len >= 40) && (0 == arm_nwk_nd_address_read(network_interface_id, &nd_address)))
    {
        ip6tos(nd_address.border_router, address);
        tr_info("Router IP address: %s", address);
        return 0;
    } else
    {
        return -1;
    }
}

int8_t mesh_tasklet_start(mesh_interface_cb callback, int8_t nwk_interface_id)
{
    int8_t error_status = eventOS_event_handler_create(&mesh_tasklet_main, ARM_LIB_TASKLET_INIT_EVENT);
    if (error_status < 0)
    {
        // -1 handler already used by other tasklet
        // -2 memory allocation failure
        return error_status;
    }

    mesh_api_cb = callback;
    network_interface_id = nwk_interface_id;

    return 0;   // no error
}

void mesh_tasklet_process_event(void)
{
    extern void event_dispatch_cycle();
    event_dispatch_cycle();
}

void mesh_tasklet_system_init()
{
    ns_dyn_mem_init(app_stack_heap, APP_DEFINED_HEAP_SIZE, mesh_tasklet_heap_error_handler, 0);
    randLIB_seed_random();
    platform_timer_enable();
    eventOS_scheduler_init();
    trace_init(); // trace system needs to be initialized right after eventOS_scheduler_init
    net_init_core();
}

int8_t mesh_tasklet_network_init(char *description, int8_t device_id)
{
    return arm_nwk_interface_init(NET_INTERFACE_RF_6LOWPAN, device_id, (char*)description);
}
