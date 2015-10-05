#!/bin/bash
# this is a setup scripts for im server
# author: luoning
# date: 09/02/2014

# setup im server

INSTALL_DIR=.
IM_SERVER=im-server-*

FILE_SERVER=file_server
LOGIN_SERVER=login_server
MSG_SERVER=msg_server
ROUTE_SERVER=route_server
MSFS_SERVER=msfs_server
HTTP_MSG_SERVER=http_msg_server
PUSH_SERVER=push_server
DB_PROXY_SERVER=db_proxy_server
PUSH_CLIENT=push_client



run_im_server() {
	nohup ./monitor.sh $LOGIN_SERVER &
	nohup ./monitor.sh $ROUTE_SERVER &
	nohup ./monitor.sh $MSG_SERVER &
	nohup ./monitor.sh $FILE_SERVER &
	nohup ./monitor.sh $MSFS_SERVER &
	nohup ./monitor.sh $HTTP_MSG_SERVER &
	nohup ./monitor.sh $PUSH_SERVER &
	nohup ./monitor.sh $PUSH_CLIENT &
#	nohup ./monitor.sh $DB_PROXY_SERVER &
}

print_help() {
	echo "Usage: "
	echo "  $0 check --- check environment"
	echo "  $0 install --- check & run scripts to install"
}

case $1 in
	check)
		print_hello $1
		;;
	install)
		print_hello $1

		run_im_server
		;;
	*)
		print_help
		;;
esac


