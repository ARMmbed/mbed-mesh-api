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
#include "atmel-rf-driver/driverRFPhy.h"    // rf_device_register
#include "test_cases.h"
#include "mbed/test_env.h"
#define HAVE_DEBUG 1
#include "ns_trace.h"

#define TRACE_GROUP  "main"     // for traces

/**
 * NOTE! Reconnecting to mesh network fails after one connect-disconnect sequence,
 * some tests are flagged out due this bug.
 * Thread and ND tests can't be executed at the same time.
 */

//#define TEST_THREAD
//#define CONNECT_RECONNECT

class TestMeshApi;
TestMeshApi *testMeshApi;

static int8_t rf_device_id =-1;

class TestMeshApi {
private:
    int testId;
    int tests_pass;

public:
    TestMeshApi() : testId(0), tests_pass(1)
    {
        tr_info("TestMeshApi");
    }

    void runTests()
    {
        switch(testId){
        case 0:
            test_mesh_api_init(rf_device_id);
            break;
        case 1:
            test_mesh_api_init_thread(rf_device_id);
            break;
        case 2:
            test_mesh_api_disconnect(rf_device_id, MESH_TYPE_6LOWPAN_ND);
            break;
        case 3:
            test_mesh_api_disconnect(rf_device_id, MESH_TYPE_THREAD);
            break;

#ifdef TEST_THREAD
        case 4:
            test_mesh_api_connect_disconnect_loop_thread(rf_device_id, 1 /*5*/);
            break;
#else
        case 4:
            test_mesh_api_connect_disconnect_loop(rf_device_id, 1 /*5*/);
            break;
#endif

#ifdef CONNECT_RECONNECT
        case 5:
            // interface connect/disconnect/connect loop is not working properly,
            // therefore skip tests that require re-connecting
            test_mesh_api_connect(rf_device_id);
            break;
#endif


        default:
            endTest(tests_pass);
            break;
        }

        testId++;
    }

    void testResult(int status)
    {
        tests_pass = tests_pass && status;
    }

    void endTest(bool status)
    {
        MBED_HOSTTEST_RESULT(status);
    }
};


void test_result_notify(int result, AbstractMesh *meshAPI)
{
    if (meshAPI != NULL) {
        delete meshAPI;
    }
    /* inform results to main class */
    FunctionPointer1<void, int> fpResult(testMeshApi, &TestMeshApi::testResult);
    minar::Scheduler::postCallback(fpResult.bind(result));

    /* Schedule next test to run */
    FunctionPointer0<void> fpNextTest(testMeshApi, &TestMeshApi::runTests);
    minar::Scheduler::postCallback(fpNextTest.bind());
}


void app_start(int, char**) {
    MBED_HOSTTEST_TIMEOUT(5);
    MBED_HOSTTEST_SELECT(default);
    MBED_HOSTTEST_DESCRIPTION(Mesh network API tests);
    MBED_HOSTTEST_START("mbed-mesh-api");

    // before registering RF device mesh network needs to be created
    Mesh6LoWPAN_ND *mesh_api = (Mesh6LoWPAN_ND*)MeshInterfaceFactory::createInterface(MESH_TYPE_6LOWPAN_ND);
    delete mesh_api;
    rf_device_id = rf_device_register();

    testMeshApi = new TestMeshApi;

    FunctionPointer0<void> fp(testMeshApi, &TestMeshApi::runTests);
    minar::Scheduler::postCallback(fp.bind());
}
