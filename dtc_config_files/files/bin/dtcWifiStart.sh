#!/bin/sh

# Huanle Zhang
# www.huanlezhang.com

wpa_supplicant -B -iwlan0 -c/etc/config/wpa_supplicant.conf -Dnl80211
