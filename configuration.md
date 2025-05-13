#launch client
nc 192.168.0.5 1234

#add stack interface ip to freebsd arp table
sudo arp -s 192.168.0.5 12:9f:41:0d:0e:64

