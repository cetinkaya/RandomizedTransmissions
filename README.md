# RandomizedTransmissions
RandomizedTransmissions provides Contiki-ng client and server example code that enable transmission of sensor data at random time instants. It is based on stochastic-control-theoretic ideas developed over 2020-2023.


## Devices

Code in this project is developed for Zolertia Re-Mote Rev-B development board with external temperature sensor. This board is specifically designed by Zolertia for Internet of Things applications. 

## Compiling and uploading


### Client-side
Connect Zolertia Re-Mote Rev-B that will be used as clients over usb, and use the following commands in a terminal window.

```sh
make distclean
make TARGET=zoul BOARD=remote-revb PORT=/dev/ttyUSB0 WERROR=0
make randomized-client.upload
```

### Server-side

Connect Zolertia Re-Mote Rev-B that will be used as a server over usb, and use the following commands in a terminal window.

```sh
make distclean
make TARGET=zoul BOARD=remote-revb PORT=/dev/ttyUSB0 WERROR=0
make randomized-server.upload
```

## Running

For generating plots, you can run

```sh
sudo python plottemperaturedata.py
```

## Acknowledgements

This project was supported by JSPS KAKENHI Grant No. 20K14771 (Project title: Stochastic Control-Theoretic Approach to Development of Simultaneously Cyber-Secure and Energy-Efficient Randomized Transmission Methods for Dependable IoT).
