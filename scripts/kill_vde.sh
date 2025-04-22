#!/bin/sh

echo "Unplugging switches"

pkill vde_switch
sudo pkill vde_plug2tap 

#delete control files

rm -rf /tmp/net1.vde /tmp/net2.vde /tmp/net3.vde

