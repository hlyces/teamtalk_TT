#!/bin/bash
#srcSRV.sh

#service mysql start
service mariadb start
redis-server /usr/local/etc/redis.conf
/usr/local/php5/sbin/php-fpm
/usr/sbin/nginx

IM_SERVER_DIR=/efaxin/TeamTalk-master/auto_setup/im_server/im-server-1/
cd $IM_SERVER_DIR
IM_SERVER=./restart.sh
$IM_SERVER file_server
$IM_SERVER login_server
$IM_SERVER msg_server
$IM_SERVER route_server
$IM_SERVER msfs_server
$IM_SERVER http_msg_server
$IM_SERVER push_server
$IM_SERVER db_proxy_server
$IM_SERVER push_client

