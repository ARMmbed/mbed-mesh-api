/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */
#ifndef _NODE_TASKLET_MAIN_
#define _NODE_TASKLET_MAIN_
#include "ns_types.h"
#include "eventOS_event.h"
#include "mbed-mesh-api/mesh_interface_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * \brief Read own IP address
 *
 * \param address where own ip address will be written
 * \param len length of provided address buffer
 *
 * \return 0 on success
 * \return -1 if address reading fails
 */
int8_t mesh_tasklet_get_ip_address(char *address, int8_t len);

/*
 * \brief Read border router IP address
 *
 * \param address where own ip address will be written
 * \param len length of provided address buffer
 *
 * \return 0 on success
 * \return -1 if address reading fails
 */
int8_t mesh_tasklet_get_router_ip_address(char *address, int8_t len);

/*
 * \brief Start Mesh networking state machine
 *
 * \param callback to be called when network state changes
 * \param nwk_interface_id to use for networking
 *
 * \return 0 on success
 * \return -1 if callback function is used in another tasklet
 * \return -2 if memory allocation fails
 */
int8_t mesh_tasklet_start(mesh_interface_cb callback, int8_t nwk_interface_id);

/*
 * \brief Handle events to keep network operational.
 *
 * Note! This method will be removed once energy scheduler is operational.
 */
void mesh_tasklet_process_event(void);

/*
 * \brief Initialize mesh system.
 * Memory pool, timers, traces and support are initialized.
 */
void mesh_tasklet_system_init();

/*
 * \brief Create network interface.
 *
 * \param description name for the interface
 * \param device_id registered physical device
 * \return interface ID that can be used to communication with this interface
 */
int8_t mesh_tasklet_network_init(char *description, int8_t device_id);

#ifdef __cplusplus
}
#endif
#endif /* _NODE_TASKLET_MAIN_ */
