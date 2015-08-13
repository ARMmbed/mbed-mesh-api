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

#ifndef __ABSTRACTMESH_H_
#define __ABSTRACTMESH_H_

#include "mesh_interface_types.h"
#include "AbstractNetworkInterface.h"
#include "mbed.h"
#include "FunctionPointer.h"

/**
 * \brief Mesh networking interface.
 * This class can't be instantiated directly.
 */

using namespace mbed;

class AbstractMesh : public AbstractNetworkInterface {

public:
    typedef FunctionPointer1<void, mesh_connection_status_t> MeshNetworkHandler_t;
    /*
     * Constructor
     * \param type network type
     */
    AbstractMesh(MeshNetworkType type);

    // Destructor, force derived classes implement own destructors
    // and prevent class creation.
    virtual ~AbstractMesh() = 0;

    /*
     * \brief Initialization of the interface.
     * \param registered device is physical device registered
     * \param callbackHandler is callback that is called when network state changes
     * \return >=0 on success.
     * \return -2 Driver is already associated to other interface.
     * \return -3 No memory for interface.
     */
    virtual int8_t init(int8_t registered_device_id, MeshNetworkHandler_t callbackHandler);

    virtual int8_t connect();
    virtual int8_t disconnect();

    /*
     * \brief Callback from C-layer
     * \param state state of the network
     * */
    void callback(mesh_connection_status_t state);

protected:

    /**
     * Mesh callback function
     */
    MeshNetworkHandler_t _mesh_network_handler;

    /*
     * Network interface ID
     */
    int8_t _network_interface_id;

    /*
     * Registered device ID
     */
    int8_t _device_id;

    /*
     * Mesh network type
     */
    MeshNetworkType _type;
};

#endif /* __ABSTRACTMESH_H_ */
