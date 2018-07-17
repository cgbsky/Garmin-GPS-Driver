# !/bin/bash
set line /dev/ttyUSB0
set speed 38400
set carrier-watch off
set handshake none
set flow-control none

set file type bin
set fine name lit
set rec pack 1000
set send pack 1000
set window 5
