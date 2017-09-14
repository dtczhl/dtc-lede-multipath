#!/bin/sh

# Huanle Zhang
# www.huanlezhang.com

host_name="simula"
host_ip="192.168.1.10"
host_path="/home/simula/lede-sdk/bin/packages/x86_64/base"

if [ $# -ne 1 ]; then
	echo "    Error! "
	echo "    Format: program filename"
	exit
fi

scp ${host_name}@${host_ip}:${host_path}/$1 .
