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
#include "mbed-mesh-api/Mesh6LoWPAN_ND.h"
// For tracing we need to define flag, have include and define group
#define HAVE_DEBUG 1
#include "ns_trace.h"
#define TRACE_GROUP  "mesh_appl"

// mesh network state
static mesh_connection_status_t mesh_network_state = MESH_DISCONNECTED;
static Mesh6LoWPAN_ND *meshApi;

// mesh network callback, called when network state changes
void mesh_network_callback(mesh_connection_status_t mesh_state)
{
    tr_info("mesh_network_callback() %d", mesh_state);
    mesh_network_state = mesh_state;
    if (mesh_network_state == MESH_CONNECTED) {
        tr_info("Connected to mesh network!");
        meshApi->disconnect();
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
    meshApi = Mesh6LoWPAN_ND::getInstance();
    tr_info("Mesh API test application");

    // initialize the interface with registered device and callback
    status = meshApi->init(rf_device_register(), mesh_network_callback);
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
