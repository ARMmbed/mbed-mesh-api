/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#include <stdio.h>

#include "mbed.h"
#include "test_env.h"
#include "atmel-rf-driver/driverRFPhy.h"    // rf_device_register
#include "mbed-mesh-api/Mesh6LoWPAN_ND.h"

static mesh_connection_status_t network_state = MESH_DISCONNECTED;

void mesh_api_callback(mesh_connection_status_t mesh_status)
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

    // initialize the interface with registered device and callback
    status = meshApi->init(rf_device_register(), mesh_api_callback);
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

    meshApi->disconnect();
//    do
//    {
//        meshApi->processEvent();
//    } while(network_state != MESH_DISCONNECTED);

    delete meshApi;

    printf("Mesh networking done!");

    return 0;
}
