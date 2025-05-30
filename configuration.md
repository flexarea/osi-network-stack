#Run Procedure (To simulate IP packet forwarding):
## start vde script
./vde_s.sh

## run script to connect tap0 interface  to vde switches
./tap0_.sh

## run stack (main entry program simulating network router)
./stack

## run sender
./sender

## configure ethernet frame or ip packet field in sender (use `./test/stack_test` as reference for packet configurations based on protocol)
vim sender.c 

# Establish a TCP connection
## launch client (netcat)
nc 192.168.0.5 1234

## use different port for multiple client
nc -p 8082 192.168.0.5 1234

## add stack interface ip to freebsd arp table
sudo arp -s 192.168.0.5 12:9f:41:0d:0e:64

## verify interface configuration
ifconfig

## verify arp table
arp -a

