# mbed mesh api
The ARM mbed mesh API allows client to use IPv6 mesh network.

Client can use ```Mesh6LoWPAN_ND``` object for connecting to the mesh network 
and once successfully connected the client can create a socket by using the mbed
C++ socket API to start communication with remote peer.

Supported mesh network modes are:

* 6LoWPAN ND (neighbor discovery)

## Usage Notes
This module should not be used directly by applications. Applications should 
use:

* ```connection manager```for handling network connections
* ```device config API``` for configuring the network

This module is under construction and limitations exists:

* Node is configured to router mode.
* Beacon scan happens on channel 4 (subGhz). This can be changed by setting 
  CONFIGURED_SCAN_CHANNEL to some other channel in ```mesh_tasklet.c```

### Network connection states
Originally network is in state MESH_DISCONNECTED. Once successfully connected 
the state changes to MESH_CONNECTED and when disconnected from the network the 
state is changed back to MESH_DISCONNECTED.

If case of connection errors the state is changed to some of the connection error 
states. In the error state there is no need to make disconnect request and 
client is allowed to attempt connecting again.

## Getting Started
Module contains example application in:

* ./test/6lowpan_nd/main.cpp

In short: usage will be like:

Create callback function to catch network status
```
void mesh_api_callback(mesh_connection_status_t mesh_status)
{
    network_state = mesh_status;
}
```
Create a Mesh6LoWPAN_ND class
```C++
Mesh6LoWPAN_ND *meshApi = Mesh6LoWPAN_ND::getInstance();
```
Initialize object with registered RF device and callback created
```C++
meshApi->init(rf_device_register(), mesh_api_callback);
```
Connect to mesh network
```C++
meshApi->connect();
```
Wait callback to be called and once connected create a socket and start communication
```
if (network_state == MESH_CONNECTED) {
    UDPaSocket s(SOCKET_STACK_NANOSTACK_IPV6);
    s.open(SOCKET_AF_INET6);
	/* start communication */
	s.send_to(data, dlen, addr, port);
}

```

## Testing
Test application can be found under ./test/system_test

preconditions:

* A frdm-k64f development board with connected 6LoWPAN RF shield
* 6LoWPAN border router
* a serial terminal emulator for tracing

Compile the test
```
yotta target frdm-k64f-gcc
yt build
```
1. Flash the frdm-k64f with the software
2. Start the serial terminal emulator
3. Press the reset button on the board
4. Check test results from the traces

