#!/bin/sh

rm -rf /tmp/net1.vde /tmp/net2.vde

#start the vde switches

vde_switch -s ~/cs432/cs431/tmp/net1.vde -d
vde_switch -s ~/cs432/cs431/tmp/net2.vde -d
vde_switch -s ~/cs432/cs431/tmp/net3.vde -d

echo "launching network"
