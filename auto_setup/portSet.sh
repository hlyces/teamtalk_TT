#!/bin/bash

#mysql
iptables -I INPUT -p tcp --dport 3306 -j ACCEPT

#nginx
iptables -I INPUT -p tcp --dport 80 -j ACCEPT

#msg_server
iptables -I INPUT -p tcp --dport 8000 -j ACCEPT

#login_server
iptables -I INPUT -p tcp --dport 8080 -j ACCEPT

#msfs
iptables -I INPUT -p tcp --dport 8700 -j ACCEPT

#file_server
iptables -I INPUT -p tcp --dport 8600 -j ACCEPT