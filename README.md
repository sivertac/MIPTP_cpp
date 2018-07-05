# MIPTP_cpp
Mini Internet Protocol Transport Protocol (MIPTP)
A small project to port a project that was origninally written in C to C++, and to explore/use C++ features.

## Disclaimer
This project is for educational purposes only and should not be used in any real life application.

## Goal
To implement the necessary network layers ontop of the linux raw socket interface (man 7 raw) to achieve:
* Link layer
	* Use ethernet interfaces as transmission medium.
	* Implement address resolution protocol ([ARP](https://en.wikipedia.org/wiki/Address_Resolution_Protocol)) to discover immediate neighbours.
	* Detect nodes going offline with timeouts.
* Network layer
	* Implement distance-vector routing ([DVR](https://en.wikipedia.org/wiki/Distance-vector_routing_protocol)) with split horizon to discover the network topology.
	* Every node can reach all nodes in the network.
	* Nodes can forward transport packets.
* Transport layer
	* Two nodes can reliably transmit data between eachother over the network (even with lossy links).

* Application layer
	* Provide an interface for Application layer programs.
	* Create a file receiving program that can receive files over the network.
	* Create a file sending program that can send files over the network.

## Design
The original C project was from a networking course I took, I've made some changes to the design requirements and added some features.

The system is divided in 3 parts; MIP_deamon, routing_deamon and transport_deamon.
* MIP_deamon
	*
* routing_deamon
* transport_deamon


## Test environment


## Demo


