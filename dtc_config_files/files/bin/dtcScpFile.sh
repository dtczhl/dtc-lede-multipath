#!/bin/sh

# Huanle Zhang
# www.huanlezhang.com

host_name="dtc"
host_ip="192.168.3.10"
host_path="/home/dtc/lede-x86-sdk/bin/packages/x86_64/base"

if [ $# -ne 1 ]; then
	echo "    Error! "
	echo "    Format: program filename"
	exit
fi

scp ${host_name}@${host_ip}:${host_path}/$1 .
