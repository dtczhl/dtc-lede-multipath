#!/bin/bash

# Huanle Zhang
# www.huanlezhang.com


ps | grep dtc_many_sock_client | grep -v grep | awk '{print $1}' | xargs -r kill -9
ps | grep dtc_many_sock_echo   | grep -v grep | awk '{print $1}' | xargs -r kill -9
