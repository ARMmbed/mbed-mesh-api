/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#ifndef __MESH_INTERFACE_TYPES_H__
#define __MESH_INTERFACE_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Network status codes
 */
typedef enum {
    MESH_CONNECTED = 0,
    MESH_DISCONNECTED
} mesh_status_t;

/**
 * Type of the network status callback
 */
typedef void (*mesh_interface_cb)(mesh_status_t mesh_status);


#ifdef __cplusplus
}
#endif

#endif /* __MESH_INTERFACE_TYPES_H__ */
