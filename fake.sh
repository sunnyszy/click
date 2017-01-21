#!/bin/bash
#sudo ifconfig fake0 down
#sudo ifconfig fake0 172.17.2.135
sudo route add default gw 172.17.2.68 fake0
sudo ifconfig fake0 hw ether 44:c3:06:31:5b:07
#sudo ifconfig fake0 hw ether 00:11:22:33:44:55
sudo ifconfig fake0 up
