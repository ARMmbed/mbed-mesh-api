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
 * Mesh networking interface.
 */

#include "mesh/mesh_tasklet.h"
#include "mbed-mesh-api/AbstractMesh.h"

#define HAVE_DEBUG 1
#include "ns_trace.h"

#define TRACE_GROUP  "m6La"

AbstractMesh::AbstractMesh() :
    callback(NULL), network_interface_id(-1), device_id(-1)
{
    // initialize mesh networking resources, memory, timers, etc...
    mesh_tasklet_system_init();
}

AbstractMesh::~AbstractMesh()
{
}

int8_t AbstractMesh::init()
{
    tr_debug("init()");
    return -1;
}

int8_t AbstractMesh::init(int8_t registered_device_id, mesh_interface_cb callback_func)
{
    tr_debug("init()");

    if (callback_func == NULL)
    {
        return MESH_ERROR_PARAM;
    }

    device_id = registered_device_id;
    callback = callback_func;

    // Create network interface
    network_interface_id = mesh_tasklet_network_init(device_id);
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

    return MESH_ERROR_UNKNOWN;
}

int8_t AbstractMesh::connect(void)
{
    int8_t status;
    tr_debug("connect()");

    if (callback == NULL)
    {
        // initialization hasn't been made
        return MESH_ERROR_UNKNOWN;
    }

    status = mesh_tasklet_connect(callback, network_interface_id);
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
    else if (status == -3)
    {
        return MESH_ERROR_STATE;
    }
    else
    {
        return MESH_ERROR_UNKNOWN;
    }
}

int8_t AbstractMesh::disconnect()
{

   int8_t status = mesh_tasklet_disconnect();

    if (status >= 0)
    {
        return MESH_ERROR_NONE;
    }
    else
    {
        return MESH_ERROR_UNKNOWN;
    }
}

void AbstractMesh::processEvent(void)
{
    mesh_tasklet_process_event();
}

