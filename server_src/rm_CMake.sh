#!/bin/bash

clean() {
	cd base
	rm CMakeCache.txt
	make clean
	cd ../login_server
	rm CMakeCache.txt
	make clean
	cd ../route_server
	rm CMakeCache.txt
	make clean
	cd ../msg_server
	rm CMakeCache.txt
	make clean
	cd ../http_msg_server
	rm CMakeCache.txt
    make clean
	cd ../file_server
	rm CMakeCache.txt
    make clean
    cd ../push_server
	rm CMakeCache.txt
    make clean
	cd ../db_proxy_server
	rm CMakeCache.txt
	make clean
    cd ../push_server
	rm CMakeCache.txt
    make clean
    cd ../push_client
	rm CMakeCache.txt
    make clean
    cd ../msfs_server
	rm CMakeCache.txt
    make clean
	cd ../test
	make clean
	cd ..
}
clean
