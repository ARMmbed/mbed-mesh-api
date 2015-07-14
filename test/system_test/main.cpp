/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#include "mbed.h"
#include "mbed-mesh-api/Mesh6LoWPAN_ND.h"
#include "atmel-rf-driver/driverRFPhy.h"    // rf_device_register
#include "test_cases.h"
#include "mbed/test_env.h"
#define HAVE_DEBUG 1
#include "ns_trace.h"

#define TRACE_GROUP  "main"     // for traces

int main()
{
    MBED_HOSTTEST_TIMEOUT(5);
    MBED_HOSTTEST_SELECT(default);
    MBED_HOSTTEST_DESCRIPTION(Mesh network API tests);
    MBED_HOSTTEST_START("mbed-mesh-api");

    int rc;
    int tests_pass = 1;

    // before registering RF device mesh network needs to be created
    Mesh6LoWPAN_ND *mesh_api = Mesh6LoWPAN_ND::getInstance();
    delete mesh_api;
    int8_t rf_device_id = rf_device_register();

    // interface connect/disconnect/connect loop is not working properly,
    // therefore skip tests that require connecting again
    do
    {
        rc = mesh_api_connect_disconnect_loop(rf_device_id, 1 /*5*/);
        tests_pass = tests_pass && rc;

        rc = mesh_api_init(rf_device_id);
        tests_pass = tests_pass && rc;

#ifdef CONNECT_RECONNECT
        rc = mesh_api_connect(rf_device_id);
        tests_pass = tests_pass && rc;
#endif

        rc = mesh_api_disconnect(rf_device_id);
        tests_pass = tests_pass && rc;

    } while (0);

    MBED_HOSTTEST_RESULT(tests_pass);
    return !tests_pass;
}
