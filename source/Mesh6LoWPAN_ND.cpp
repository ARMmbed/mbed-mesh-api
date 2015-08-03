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
