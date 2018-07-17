# !/bin/bash

if [ -e /dev/ttyUSB0 ]; then
	echo "start deamon"
	#start gps deamon thread 
	sudo gpsd /dev/ttyUSB0
else
	echo "insert driver"
	#insert usbserial driver
	sudo modprobe usbserial
	#insert garmin_gps.ko
	sudo insmod garmin_gps.ko
	
	echo "start deamon"
	sudo gpsd /dev/vfio/vfio
fi

