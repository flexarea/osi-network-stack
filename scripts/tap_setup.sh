#!/bin/sh
#
# Creates and configures a tap device on FreeBSD along with a vde_switch
# connected to it.

network="192.168.0.0"
netmask="255.255.255.0"

vde_switch_ctrl_file="/tmp/net0.vde"
tap_device="tap0"
kernel_endpoint_ip="192.168.0.4"

# Use -hub so Wireshark can see unicast frames.
vde_switch -sock "$vde_switch_ctrl_file" -hub -daemon

sudo ifconfig tap0 destroy
sudo ifconfig tap0 create
sudo ifconfig tap0 "$kernel_endpoint_ip" netmask "$netmask" up

sudo vde_plug2tap --sock "$vde_switch_ctrl_file" "$tap_device" --daemon
