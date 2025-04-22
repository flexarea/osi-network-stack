#!/bin/sh

sudo ifconfig tap0 up
#sudo vde_plug2tap tap0
sudo vde_plug2tap -s /home/entuyenabo/cs432/cs431/tmp/net1.vde tap0 -d
sudo vde_plug2tap -s /home/entuyenabo/cs432/cs431/tmp/net2.vde tap1 -d
sudo vde_plug2tap -s /home/entuyenabo/cs432/cs431/tmp/net2.vde tap2 -d
