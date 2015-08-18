# mbed mesh API
The ARM mbed mesh API allows the client to use the IPv6 mesh network.

The client can use the `Mesh6LoWPAN_ND` or `MeshThread`object for connecting to the 
mesh network and when once successfully connected, the client can create a socket by 
using the [mbed C++ socket API](https://github.com/ARMmbed/sockets) to start 
communication with a remote peer. When the connection is no longer needed the client 
can close the connection with the `disconnect` method.

### Supported mesh networking modes
Currently, 6LoWPAN-ND (neighbour discovery) and Thread bootstrap modes are supported.

### Mesh socket
The mbed mesh API cooperates with module [mbed mesh socket](https://github.com/ARMmbed/sal-iface-6lowpan). 
The module lists the known mesh socket limitations. Full documentation of the IPv6/6LoWPAN 
stack can be found in [IPv6/6LoWPAN stack](https://github.com/ARMmbed/sal-stack-nanostack).

## Usage notes
This module should not be used directly by the applications. The applications should 
use the following modules when they are available:

* `connection manager`for handling network connections.
* `device config API` for configuring the network.

This module is under construction and therefore, there are some limitations as follows:

* A node is statically configured to router mode.
* In 6LoWPAN-ND mode beacon scan takes place on channel 12 (2.4Ghz). You can change 
 this by setting `DEFAULT_SCAN_CHANNEL` to some other channel in `./source/include/static_config.h`.
* In Thread bootstrap mode RF channel is set to channel 12 (2.4GHz). You can change
 this by updating `THREAD_RF_CHANNEL` in `./source/include/static_config.h`

### Network connection states
After the initialization, the network state is `MESH_DISCONNECTED`. After a successful connection, 
the state changes to `MESH_CONNECTED` and when disconnected from the network the 
state is changed back to `MESH_DISCONNECTED`.

In case of connection errors, the state is changed to some of the connection error 
states. In an error state, there is no need to make a `disconnect` request and the
client is allowed to attempt connecting again.

## Getting started
This module contains an example application in the `./test/6lowpan_nd/main.cpp` folder. 
To build and run the example, see the [instructions](https://github.com/ARMmbed/mbed-mesh-api/tree/master/test/6lowpan_nd).

## Usage example for 6LoWPAN ND mode

Create a callback function to catch the network status:
```
void mesh_api_callback(mesh_connection_status_t mesh_status)
{
    network_state = mesh_status;
}
```
Create a Mesh6LoWPAN_ND class:
```C++
Mesh6LoWPAN_ND *meshApi = (Mesh6LoWPAN_ND*)MeshInterfaceFactory::createInterface(MESH_TYPE_6LOWPAN_ND);
```
Initialize the object with a registered RF device and callback:
```C++
meshApi->init(rf_device_register(), mesh_api_callback);
```
Connect to the mesh network:
```C++
meshApi->connect();
```
Wait for the callback to be called and when successfully connected to the mesh network, create a socket and 
start communication with the remote end:
```
if (network_state == MESH_CONNECTED) {
    UDPaSocket s(SOCKET_STACK_NANOSTACK_IPV6);
    s.open(SOCKET_AF_INET6);
	/* start communication */
	s.send_to(data, dlen, addr, port);
}

```
## Usage example for 6LoWPAN Thread mode

Create a callback function to catch the network status:
```
void mesh_api_callback(mesh_connection_status_t mesh_status)
{
    network_state = mesh_status;
}
```
Create a MeshThread class:
```C++
MeshThread *meshApi = (MeshThread*)MeshInterfaceFactory::createInterface(MESH_TYPE_THREAD);
```
Initialize the object with a registered RF device ID, callback, RF device MAC address and private shared key:
```
uint8_t eui64[8];
rf_read_mac_address(eui64);
char *pskd = (char*)"Secret password";
status = meshApi->init(rf_id, mesh_network_callback, eui64, pskd);
```
Connect to the mesh network:
```C++
meshApi->connect();
```
Wait for the callback to be called and once successfully connected to the mesh network, create a socket and 
start communication with the remote end:
```
if (network_state == MESH_CONNECTED) {
    UDPaSocket s(SOCKET_STACK_NANOSTACK_IPV6);
    s.open(SOCKET_AF_INET6);
	/* start communication */
	s.send_to(data, dlen, addr, port);
}

```

## Testing
The test application is located in `./test/system_test`.

Use the same setup as in the example application. 

Compile the test:
```
yt target frdm-k64f-gcc
yt build
```
1. Flash the software to the FRDM-K64F board.
2. Start the serial terminal emulator.
3. Press the **Reset** button on the board.
4. Check the test results in the traces.

