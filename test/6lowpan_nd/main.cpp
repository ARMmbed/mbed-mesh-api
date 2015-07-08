/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#include <stdio.h>

#include "mbed.h"
#include "test_env.h"
#include "atmel-rf-driver/driverRFPhy.h"    // rf_device_register
#include "mbed-mesh-api/Mesh6LoWPAN_ND.h"

static mesh_status_t network_state = MESH_DISCONNECTED;

void mesh_api_callback(mesh_status_t mesh_status)
{
    printf("mesh_api_callback %d", mesh_status);
    network_state = mesh_status;
}

int main() {
    Mesh6LoWPAN_ND *meshApi;
    int8_t status;

    printf("Mesh API test application");

    // create 6LoWPAN_ND interface
    meshApi = new Mesh6LoWPAN_ND();

    // initialize the interface
    status = meshApi->init("6LoWPAN node", rf_device_register(), mesh_api_callback);
    if (status < 0)
    {
        printf("Can't initialize mesh network");
        return -1;
    }

    // connect interface to the network
    status = meshApi->connect();
    if (status != 0)
    {
        printf("Can't connect to Mesh network!");
        return -1;
    }

    // wait until network is connected
    do
    {
        meshApi->processEvent();
    } while (network_state != MESH_CONNECTED);

    printf("Connected to mesh network!");

//    meshApi->disconnect();
//    do
//    {
//        mesh_api->processEvent();
//    } while(network_state != MESH_DISCONNECTED);

    delete meshApi;

    printf("All done!");

    return 0;
}
