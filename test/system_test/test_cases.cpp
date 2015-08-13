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
#include "mbed-mesh-api/MeshThread.h"
#include "mbed-mesh-api/MeshInterfaceFactory.h"
#include <mbed-net-socket-abstract/test/ctest_env.h>
#include "atmel-rf-driver/driverRFPhy.h"    // rf_device_register
#include "test_cases.h"
#include "mbed/test_env.h"
#define HAVE_DEBUG 1
#include "ns_trace.h"

#define TRACE_GROUP  "main"     // for traces

static uint8_t mesh_network_state = MESH_DISCONNECTED;
AbstractMesh *mesh_api = NULL;

extern void test_result_notify(int result, AbstractMesh *meshAPI);

/*
 * Callback from mesh network. Called when network state changes.
 */
void test_callback_connect_disconnect(mesh_connection_status_t mesh_state)
{
    tr_info("test_callback_connect_disconnect() %d", mesh_state);
    mesh_network_state = mesh_state;
    if (mesh_network_state == MESH_CONNECTED) {
        tr_info("Connected to mesh network!");
        int8_t err = mesh_api->disconnect();
        TEST_EQ(err, MESH_ERROR_NONE);
    } else if (mesh_network_state == MESH_DISCONNECTED) {
        tr_info("Disconnected from mesh network!");
        test_result_notify(test_pass_global, mesh_api);
    } else {
        // untested branch, catch erros by bad state checking...
        TEST_EQ(mesh_network_state, MESH_CONNECTED);
        tr_error("Networking error!");
        test_result_notify(test_pass_global, mesh_api);
    }
}

void test_mesh_api_connect_disconnect_loop(int8_t rf_device_id, uint8_t loop_count)
{
    int8_t err;
    mesh_api = MeshInterfaceFactory::createInterface(MESH_TYPE_6LOWPAN_ND);
    err = mesh_api->init(rf_device_id, test_callback_connect_disconnect);

    if (!TEST_EQ(err, MESH_ERROR_NONE))
    {
        test_result_notify(test_pass_global, mesh_api);
        return;
    }

    for(int i=0; i<loop_count; i++) {
        err = mesh_api->connect();
        if (!TEST_EQ(err, MESH_ERROR_NONE)) {
            test_result_notify(test_pass_global, mesh_api);
            break;
        }
        // control goes to test callback
    }
}

void test_mesh_api_connect_disconnect_loop_thread(int8_t rf_device_id, uint8_t loop_count)
{
    int8_t err;
    mesh_api = (MeshThread*)MeshInterfaceFactory::createInterface(MESH_TYPE_THREAD);
    uint8_t eui64[8];
    char *pskd;
    rf_read_mac_address(eui64);
    pskd = (char*)"Secret password";
    err = ((MeshThread*)mesh_api)->init(rf_device_id, test_callback_connect_disconnect, eui64, pskd);

    if (!TEST_EQ(err, MESH_ERROR_NONE))
    {
        test_result_notify(test_pass_global, mesh_api);
        return;
    }

    for(int i=0; i<loop_count; i++) {
        err = mesh_api->connect();
        if (!TEST_EQ(err, MESH_ERROR_NONE)) {
            test_result_notify(test_pass_global, mesh_api);
            break;
        }
        // control goes to test callback
    }
}

void test_callback_init(mesh_connection_status_t mesh_state)
{
    tr_info("test_callback_init() %d", mesh_state);
}

void test_mesh_api_init(int8_t rf_device_id)
{
    int8_t err;
    mesh_api = (Mesh6LoWPAN_ND*)MeshInterfaceFactory::createInterface(MESH_TYPE_6LOWPAN_ND);

    do
    {
        // no callback set
        err = mesh_api->init(rf_device_id, NULL);
        if (!TEST_EQ(err, MESH_ERROR_PARAM))
        {
            break;
        }

        // bad rf-device_id
        err = mesh_api->init(rf_device_id+1, test_callback_init);
        if (!TEST_EQ(err, MESH_ERROR_MEMORY))
        {
            break;
        }

        // successful re-initialization
        err = mesh_api->init(rf_device_id, test_callback_init);
        if (!TEST_EQ(err, MESH_ERROR_NONE))
        {
            break;
        }

        /* Thread can't be instantiated if ND is initialized */
        MeshThread *meshThread = (MeshThread*)MeshInterfaceFactory::createInterface(MESH_TYPE_THREAD);
        if (!TEST_EQ(meshThread, NULL))
        {
            break;
        }
        break;
    } while(1);

    test_result_notify(test_pass_global, mesh_api);
}

/*
 * Callback from mesh network for mesh_api_connect test
 */
void test_callback_connect(mesh_connection_status_t mesh_state)
{
    int8_t err;
    tr_info("test_callback_connect() %d", mesh_state);
    mesh_network_state = mesh_state;

    if (mesh_network_state == MESH_CONNECTED) {
        tr_info("Connected to mesh network!");
        // try to connect again, shold fail
        err = mesh_api->connect();
        if (!TEST_EQ(err, MESH_ERROR_STATE))
        {
            test_result_notify(test_pass_global, mesh_api);
        }
        else
        {
            // disconnect
            err = mesh_api->disconnect();
            if (!TEST_EQ(err, MESH_ERROR_NONE))
            {
                test_result_notify(test_pass_global, mesh_api);
            }
        }
    } else if (mesh_network_state == MESH_DISCONNECTED) {
        tr_info("Disconnected from mesh network!");
        test_result_notify(test_pass_global, mesh_api);
    } else {
        // untested branch, catch erros by bad state checking...
        TEST_EQ(mesh_network_state, MESH_CONNECTED);
        tr_error("Networking error!");
    }
}

void test_mesh_api_connect(int8_t rf_device_id)
{
    int8_t err;
    mesh_api = (Mesh6LoWPAN_ND*)MeshInterfaceFactory::createInterface(MESH_TYPE_6LOWPAN_ND);

    do
    {
        // connect uninitialized
        err = mesh_api->connect();
        if (!TEST_EQ(err, MESH_ERROR_UNKNOWN)) {
            test_result_notify(test_pass_global, mesh_api);
            break;
        }

        err = mesh_api->init(rf_device_id, test_callback_connect);
        if (!TEST_EQ(err, MESH_ERROR_NONE)) {
            test_result_notify(test_pass_global, mesh_api);
            break;
        }

        // successful connect
        mesh_network_state = MESH_DISCONNECTED;
        err = mesh_api->connect();
        if (!TEST_EQ(err, MESH_ERROR_NONE)) {
            test_result_notify(test_pass_global, mesh_api);
            break;
        }

        break;
    } while(1);
}


