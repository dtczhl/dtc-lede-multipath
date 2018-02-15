#!/bin/sh


if [ $# -ne 1 ]; then
	echo "*** Error!"
	echo "Format: program program_to_be_killed"
	echo "	E.g., program dtc_abc"
	echo "			Kill dtc_abc process"
fi

ps | grep "$1" | grep -v grep | awk '{print $1}' | xargs -r kill -9
