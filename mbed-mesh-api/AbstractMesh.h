/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

#ifndef __ABSTRACTMESH_H_
#define __ABSTRACTMESH_H_

#include "mesh_interface_types.h"
#include "AbstractNetworkInterface.h"

/**
 * \brief Mesh networking interface.
 * This class can't be instantiated directly.
 */

class AbstractMesh : public AbstractNetworkInterface {

public:

    /*
     * Constructor
     */
    AbstractMesh();

    // Destructor, force derived classes implement own destructors
    // and prevent class creation.
    virtual ~AbstractMesh() = 0;

    /**
     * Process event in network stack.
     * Note! This method will be obsolete once energy scheduler is ready
     * */
    void processEvent(void);

protected:
    /**
     * Mesh callback function
     */
    mesh_interface_cb callback;

    /*
     * Network interface ID
     */
    int8_t network_interface_id;
};

#endif /* __ABSTRACTMESH_H_ */
