# SPARKS App for Crazyflie 2.x

This folder contains the app layer application for the Crazyflie implementation of the SPARKS protocol. 

The provided example allow to enrol and then authenticate two drones using the protocol. `main.cpp` contains the code for the client and for the server drone. To run these examples, comment either the client or server part, build the program using `make`, then flash the drone. Restart this process to put the other code (client/server) on the other drone. 

See App layer API guide and build instructions [here](https://www.bitcraze.io/documentation/repository/crazyflie-firmware/master/userguides/app_layer/)