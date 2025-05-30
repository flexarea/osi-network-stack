#!/bin/sh

echo "Unplugging switches"

pkill vde_switch
sudo pkill vde_plug2tap

#delete control files

rm -rf /tmp/net*.vde
rm -rf ~/cs432/cs431/tmp/net*.vde
rm -rf /home/entuyenabo/cs432/cs431/tmp/net*.vde
