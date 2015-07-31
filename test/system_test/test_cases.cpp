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
#include "minar/minar.h"
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
    if (mesh_network_state == MESH_CONNECTED) {
        tr_info("Connected to mesh network!");
        int8_t err = mesh_api->disconnect();
        TEST_EQ(err, MESH_ERROR_NONE);
    } else if (mesh_network_state == MESH_DISCONNECTED) {
        tr_info("Disconnected from mesh network!");
        minar::Scheduler::stop();
    } else {
        // untested branch, catch erros by bad state checking...
        TEST_EQ(mesh_network_state, MESH_CONNECTED);
        tr_error("Networking error!");
        minar::Scheduler::stop();
    }
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

    for(int i=0; i<loop_count; i++) {
        err = mesh_api->connect();
        if (!TEST_EQ(err, MESH_ERROR_NONE)) {
            break;
        }
        // use mesh_network_callback for disconnecting
        minar::Scheduler::start();
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

/*
 * Callback from mesh network for mesh_api_connect test
 */
void mesh_network_callback_connect_test(mesh_connection_status_t mesh_state)
{
    tr_info("mesh_network_callback_connect_test() %d", mesh_state);
    mesh_network_state = mesh_state;
    if (mesh_network_state == MESH_CONNECTED) {
        tr_info("Connected to mesh network!");
        // stop minar, to let test to go further
        minar::Scheduler::stop();
    } else if (mesh_network_state == MESH_DISCONNECTED) {
        tr_info("Disconnected from mesh network!");
        minar::Scheduler::stop();
    } else {
        // untested branch, catch erros by bad state checking...
        TEST_EQ(mesh_network_state, MESH_CONNECTED);
        tr_error("Networking error!");
        minar::Scheduler::stop();
    }
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
        if (!TEST_EQ(err, MESH_ERROR_UNKNOWN)) {
            break;
        }

        err = mesh_api->init(rf_device_id, mesh_network_callback_connect_test);
        if (!TEST_EQ(err, MESH_ERROR_NONE)) {
            break;
        }

        // successful connect
        mesh_network_state = MESH_DISCONNECTED;
        err = abstractMesh->connect();
        if (!TEST_EQ(err, MESH_ERROR_NONE)) {
            break;
        }

        minar::Scheduler::start();

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

        minar::Scheduler::start();
        break;
    } while(1);

    delete mesh_api;
    TEST_RETURN();
}

/*
 * Callback from mesh network for mesh_api_disconnect test
 */
void mesh_network_callback_disconnect_test(mesh_connection_status_t mesh_state)
{
    tr_info("mesh_network_callback_disconnect_test() %d", mesh_state);
    mesh_network_state = mesh_state;
    if (mesh_network_state == MESH_CONNECTED) {
        tr_info("Connected to mesh network!");
    } else if (mesh_network_state == MESH_DISCONNECTED) {
        tr_info("Disconnected from mesh network!");
        minar::Scheduler::stop();
    } else {
        // untested branch, catch erros by bad state checking...
        TEST_EQ(mesh_network_state, MESH_CONNECTED);
        tr_error("Networking error!");
    }
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

        err = mesh_api->init(rf_device_id, mesh_network_callback_disconnect_test);
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
        minar::Scheduler::start();
        if (!TEST_EQ(mesh_network_state, MESH_DISCONNECTED))
        {
            break;
        }
        break;
    } while(1);

    delete mesh_api;
    TEST_RETURN();
}

