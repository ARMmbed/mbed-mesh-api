/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#ifndef __MESH_INTERFACE_TYPES_H__
#define __MESH_INTERFACE_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Network connection status codes
 */
typedef enum {
    MESH_ERROR_NONE = 0,    /*<! No error */
    MESH_ERROR_UNKNOWN,     /*<! Unspecified error */
    MESH_ERROR_MEMORY,      /*<! Memory error */
    MESH_ERROR_STATE,       /*<! Illegal state */
    MESH_ERROR_PARAM,       /*<! Illegal parameter */
} mesh_error_t;

/*
 * Network connection status codes
 */
typedef enum {
    MESH_CONNECTED = 0,             /*<! connected to network */
    MESH_DISCONNECTED,              /*<! disconnected from network */
    MESH_BOOTSTRAP_START_FAILED,    /*<! error during bootstrap start */
    MESH_BOOTSTRAP_FAILED           /*<! error in bootstrap */
} mesh_connection_status_t;

/**
 * Type of the network status callback
 */
typedef void (*mesh_interface_cb)(mesh_connection_status_t mesh_status);


#ifdef __cplusplus
}
#endif

#endif /* __MESH_INTERFACE_TYPES_H__ */
