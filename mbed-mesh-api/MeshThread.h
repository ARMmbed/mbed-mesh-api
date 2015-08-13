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

#ifndef __MESH_THREAD_H_
#define __MESH_THREAD_H_

#include "AbstractMesh.h"

class MeshThread : public AbstractMesh {

public:

    ~MeshThread();

    /*
     * \brief Initialization of the interface.
     * \param registered device is physical device registered
     * \param callbackHandler is callback that is called when network state changes
     * \param eui64 MAC address of the registered RF driver
     * \param pskd private shared key
     * \return >=0 on success.
     * \return -2 Driver is already associated to other interface.
     * \return -3 No memory for interface.
     */
    int8_t init(int8_t registered_device_id, MeshNetworkHandler_t callbackHandler, uint8_t *eui64, char* pskd);

    // virtual methods from AbstractMesh
    int8_t init(int8_t registered_device_id, MeshNetworkHandler_t callbackHandler);
    int8_t connect();
    int8_t disconnect();

    friend class MeshInterfaceFactory;

private:
    /*
     * \brief private constructor for the MeshThread
     */
    MeshThread();

    /*
     * avoid copy/assign object
     */
    MeshThread(MeshThread const& copy);
    MeshThread & operator=(MeshThread const& copy);
};

#endif /* __MESH_THREAD_H_ */
