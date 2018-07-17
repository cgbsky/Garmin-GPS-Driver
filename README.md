#
#This is a simple setup manual user guide
#You can also use it as a shell script to 
#setup environment
#

#1.
#before start install gps lib support
#./lib_dep_ins.sh
sudo apt-get install libgps-dev gpsd


#2.
#make Garmin USB GPS module driver
cd garmin_gps_driver;
cp garmin_gps.ko ../;

#3.
#insert module driver
#depend usbserial.ko
sudo ./demo.sh

#4.
#run GPS app to get GPS information

