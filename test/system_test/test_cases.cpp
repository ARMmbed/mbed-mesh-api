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

#include "mbed.h"
#include "mbed-mesh-api/Mesh6LoWPAN_ND.h"
#include <mbed-net-socket-abstract/test/ctest_env.h>
#include "atmel-rf-driver/driverRFPhy.h"    // rf_device_register
#include "test_cases.h"
#include "mbed/test_env.h"
#define HAVE_DEBUG 1
#include "ns_trace.h"

#define TRACE_GROUP  "main"     // for traces

static uint8_t mesh_network_state = MESH_DISCONNECTED;
Mesh6LoWPAN_ND *mesh_api = NULL;

/*
 * Callback from mesh network. Called when network state changes.
 */
void mesh_network_callback(mesh_connection_status_t mesh_state)
{
    tr_info("mesh_network_callback() %d", mesh_state);
    mesh_network_state = mesh_state;
}

int mesh_api_connect_disconnect_loop(int8_t rf_device_id, uint8_t loop_count)
{
    int8_t err;
    mesh_api = Mesh6LoWPAN_ND::getInstance();
    err = mesh_api->init(rf_device_id, mesh_network_callback);

    if (!TEST_EQ(err, MESH_ERROR_NONE))
    {
        TEST_RETURN();
    }

    for(int i=0; i<loop_count; i++)
    {
        err = mesh_api->connect();
        if (!TEST_EQ(err, MESH_ERROR_NONE))
        {
            break;
        }

        do
        {
            mesh_api->processEvent();
        } while (mesh_network_state != MESH_CONNECTED);

        err = mesh_api->disconnect();
        if (!TEST_EQ(err, MESH_ERROR_NONE))
        {
            break;
        }

        do
        {
            mesh_api->processEvent();
        } while (mesh_network_state != MESH_DISCONNECTED);
    }
    delete mesh_api;
    TEST_RETURN();
}

int mesh_api_init(int8_t rf_device_id)
{
    int8_t err;
    mesh_api = Mesh6LoWPAN_ND::getInstance();

    do
    {
        // no callback
        err = mesh_api->init(rf_device_id, NULL);
        if (!TEST_EQ(err, MESH_ERROR_PARAM))
        {
            break;
        }

        // bad rf-device_id
        err = mesh_api->init(rf_device_id+1, mesh_network_callback);
        if (!TEST_EQ(err, MESH_ERROR_MEMORY))
        {
            break;
        }

        // successful re-initialization
        err = mesh_api->init(rf_device_id, mesh_network_callback);
        if (!TEST_EQ(err, MESH_ERROR_NONE))
        {
            break;
        }
        break;
    } while(1);

    delete mesh_api;
    TEST_RETURN();
}

int mesh_api_connect(int8_t rf_device_id)
{
    int8_t err;
    mesh_api = Mesh6LoWPAN_ND::getInstance();
    AbstractMesh *abstractMesh = (AbstractMesh*) mesh_api;

    do
    {
        // connect uninitialized
        err = abstractMesh->connect();
        if (!TEST_EQ(err, MESH_ERROR_UNKNOWN))
        {
            break;
        }

        err = mesh_api->init(rf_device_id, mesh_network_callback);
        if (!TEST_EQ(err, MESH_ERROR_NONE))
        {
            break;
        }

        // successful connect
        mesh_network_state = MESH_DISCONNECTED;
        err = abstractMesh->connect();
        if (!TEST_EQ(err, MESH_ERROR_NONE))
        {
            break;
        }

        do
        {
            abstractMesh->processEvent();
        } while (mesh_network_state != MESH_CONNECTED);


        // try to connect again
        err = abstractMesh->connect();
        if (!TEST_EQ(err, MESH_ERROR_STATE))
        {
            break;
        }

        err = abstractMesh->disconnect();
        if (!TEST_EQ(err, MESH_ERROR_NONE))
        {
            break;
        }

        do
        {
            abstractMesh->processEvent();
        } while (mesh_network_state != MESH_DISCONNECTED);
        break;
    } while(1);

    delete mesh_api;
    TEST_RETURN();
}

int mesh_api_disconnect(int8_t rf_device_id)
{
    int8_t err;
    mesh_api = Mesh6LoWPAN_ND::getInstance();

    mesh_network_state = MESH_BOOTSTRAP_FAILED;

    do
    {
        // disconnect not initialized
        err = mesh_api->disconnect();
        if (!TEST_EQ(err, MESH_ERROR_UNKNOWN))
        {
            break;
        }

        // check that network state changes
        if (!TEST_EQ(mesh_network_state, MESH_DISCONNECTED))
        {
            break;
        }

        err = mesh_api->init(rf_device_id, mesh_network_callback);
        if (!TEST_EQ(err, MESH_ERROR_NONE))
        {
            break;
        }

        // disconnect not connected
        mesh_network_state = MESH_BOOTSTRAP_FAILED;
        err = mesh_api->disconnect();
        if (!TEST_EQ(err, MESH_ERROR_UNKNOWN))
        {
            break;
        }
        if (!TEST_EQ(mesh_network_state, MESH_DISCONNECTED))
        {
            break;
        }
        break;
    } while(1);

    delete mesh_api;
    TEST_RETURN();
}

