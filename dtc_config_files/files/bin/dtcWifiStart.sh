#!/bin/sh

wpa_supplicant -B -iwlan0 -c/etc/config/wpa_supplicant.conf -Dnl80211
