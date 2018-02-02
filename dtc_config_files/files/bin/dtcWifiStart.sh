#!/bin/sh

# Huanle Zhang
# www.huanlezhang.com

wpa_supplicant -B -iwlan0 -c/etc/config/wpa_supplicant1.conf -Dnl80211

wpa_supplicant -B -iwlan1 -c/etc/config/wpa_supplicant2.conf -Dnl80211
