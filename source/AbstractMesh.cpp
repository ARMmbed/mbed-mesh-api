/*
 * Copyright (c) 2015 ARM. All rights reserved.
 */

/*
 * Mesh networking interface.
 */

#include "mesh/mesh_tasklet.h"
#include "mbed-mesh-api/AbstractMesh.h"

AbstractMesh::~AbstractMesh()
{
}

void AbstractMesh::processEvent(void)
{
    mesh_tasklet_process_event();
}

