/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

/*
 * 6LoWPAN ND (neighbor discovery) network interface.
 * Class provides methods for 6LoWPAN (ND) network handling.l
 */

#include <stdio.h>
#include "mesh/mesh_tasklet.h"
#include "mbed-mesh-api/Mesh6LoWPAN_ND.h"

#define HAVE_DEBUG 1
#include "ns_trace.h"

#define TRACE_GROUP  "m6lpND"

Mesh6LoWPAN_ND::Mesh6LoWPAN_ND() : device_id(-1)
{
}

Mesh6LoWPAN_ND::~Mesh6LoWPAN_ND()
{
    // TODO
    tr_debug("~Mesh6LoWPAN()");
}

int8_t Mesh6LoWPAN_ND::init(int8_t registered_device_id, mesh_interface_cb callback_func)
{
    tr_debug("init()");
    device_id = registered_device_id;
    callback = callback_func;
    // Create network interface
    network_interface_id = mesh_tasklet_network_init((char*)"6LoWPAN_ND", device_id);
    if (network_interface_id >= 0)
    {
        return MESH_ERROR_NONE;
    }
    else if (network_interface_id == -2)
    {
        return MESH_ERROR_PARAM;
    }
    else if (network_interface_id == -3)
    {
        return MESH_ERROR_MEMORY;
    }

    return MESH_ERROR_UNKOWN;
}

int8_t Mesh6LoWPAN_ND::init()
{
    tr_debug("init()");
    return -1;
}

int8_t Mesh6LoWPAN_ND::connect(void)
{
    int8_t status;
    tr_debug("connect()");
    status = mesh_tasklet_start(callback, network_interface_id);
    if (status >= 0)
    {
        return MESH_ERROR_NONE;
    }
    else if (status == -1)
    {
        return MESH_ERROR_PARAM;
    }
    else if (status == -2)
    {
        return MESH_ERROR_MEMORY;
    }
    else
    {
        return MESH_ERROR_UNKOWN;
    }
}

int8_t Mesh6LoWPAN_ND::disconnect(void)
{
    //TODO
    tr_debug("disconnect()");
    return 0;
}

bool Mesh6LoWPAN_ND::getOwnIpAddress(char *address, int8_t len)
{
    tr_debug("getOwnIpAddress()");
    if (mesh_tasklet_get_ip_address(address, len) == 0)
    {
        return true;
    }
    return false;
}

bool Mesh6LoWPAN_ND::getRouterIpAddress(char *address, int8_t len)
{
    tr_debug("getRouterIpAddress()");
    if (mesh_tasklet_get_router_ip_address(address, len) == 0)
    {
        return true;
    }
    return false;
}
