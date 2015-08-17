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
 * Thread network interface.
 * Class provides methods for 6LoWPAN (Thread mode) network handling.
 */

#include <stdio.h>
#include "mbed.h"
#include "mbed-mesh-api/MeshThread.h"
#include "include/thread_tasklet.h"
#include "include/callback_handler.h"

#define HAVE_DEBUG 1
#include "ns_trace.h"

#define TRACE_GROUP  "m6LTh"

MeshThread::MeshThread() : AbstractMesh(MESH_TYPE_THREAD)
{
}

MeshThread::~MeshThread()
{
    tr_debug("~MeshThread()");
}

int8_t MeshThread::init(int8_t registered_device_id, MeshNetworkHandler_t callbackHandler, uint8_t *eui64, char* pskd)
{

    if (eui64 == NULL || pskd == NULL)
    {
        return MESH_ERROR_PARAM;
    }

    int8_t status = AbstractMesh::init(registered_device_id, callbackHandler);

    if (status >= 0 )
    {
        thread_tasklet_set_device_config(eui64, pskd);
    }

    return status;
}

int8_t MeshThread::init(int8_t registered_device_id, MeshNetworkHandler_t callbackHandler)
{
    // TODO: Use test values for device configuration
    return init(registered_device_id, callbackHandler, NULL, NULL);
}

int8_t MeshThread::connect()
{
    return AbstractMesh::connect();
}

int8_t MeshThread::disconnect()
{
    return AbstractMesh::disconnect();
}

