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

#define TRACE_GROUP  "m6LND"

Mesh6LoWPAN_ND* Mesh6LoWPAN_ND::instance = 0;

Mesh6LoWPAN_ND *Mesh6LoWPAN_ND::getInstance()
{
    if(instance == 0)
    {
        instance = new Mesh6LoWPAN_ND();
    }
    return instance;
}

Mesh6LoWPAN_ND::Mesh6LoWPAN_ND()
{
}

Mesh6LoWPAN_ND::~Mesh6LoWPAN_ND()
{
    tr_debug("~Mesh6LoWPAN()");
    instance = NULL;
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
