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

#include <stdio.h>

#include "mbed.h"
#include "minar/minar.h"
#include "test_env.h"
#include "atmel-rf-driver/driverRFPhy.h"    // rf_device_register
#include "mbed-mesh-api/MeshInterfaceFactory.h"
#include "mbed-mesh-api/Mesh6LoWPAN_ND.h"
#include "mbed-mesh-api/MeshThread.h"
// For tracing we need to define flag, have include and define group
#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "mesh_appl"

#define THREAD
// mesh network state
static mesh_connection_status_t mesh_network_state = MESH_DISCONNECTED;
#ifdef THREAD
static MeshThread *meshApi;
#else
static Mesh6LoWPAN_ND *meshApi;
#endif

// mesh network callback, called when network state changes
void mesh_network_callback(mesh_connection_status_t mesh_state)
{
    tr_info("mesh_network_callback() %d", mesh_state);
    mesh_network_state = mesh_state;
    if (mesh_network_state == MESH_CONNECTED) {
        tr_info("Connected to mesh network!");
        //meshApi->disconnect();
    } else if (mesh_network_state == MESH_DISCONNECTED) {
        tr_info("Disconnected from mesh network!");
        minar::Scheduler::stop();
        delete meshApi;
        tr_info("Mesh networking done!");
    } else {
        tr_error("Networking error!");
    }
}

void app_start(int, char**) {
    int8_t status;

    // create 6LoWPAN_ND interface
#ifdef THREAD
    meshApi = (MeshThread*)MeshInterfaceFactory::createInterface(MESH_TYPE_THREAD);
#else
    meshApi = (Mesh6LoWPAN_ND*)MeshInterfaceFactory::createInterface(MESH_TYPE_6LOWPAN_ND);
#endif
    tr_info("Mesh API test application");


    set_trace_config(TRACE_ACTIVE_LEVEL_INFO|TRACE_MODE_COLOR|TRACE_CARRIAGE_RETURN);

    // register device after creating interface as interface initializes system rf device is using
    int8_t rf_id = rf_device_register();

#ifdef THREAD
    uint8_t eui64[8];
    rf_read_mac_address(eui64);
    char *pskd = (char*)"Secret password";

    // initialize the interface with registered device and callback
    status = meshApi->init(rf_id, mesh_network_callback, eui64, pskd);
#else
    // initialize the interface with registered device and callback
    status = meshApi->init(rf_id, mesh_network_callback);
#endif
    if (status < 0) {
        tr_error("Can't initialize mesh network");
        return;
    }

    // connect interface to the network
    status = meshApi->connect();
    if (status != 0) {
        tr_error("Can't connect to Mesh network!");
        return;
    }
}
