#!/bin/sh

# Author: Huanle Zhang
# Personal Website: www.huanlezhang.com

usage () {
    cat <<EOF
Usage: program [-h] [-i] [-u]
    -h   help information
    -i   install 
    -u   uninstall    
Example: ./main.sh -u
EOF
}

error_info () {
    echo "  Error! Show help information with -h option"
}

# starts here
if [ $# -lt 1 ]; then
    error_info 
    exit 1
fi

while getopts ":hiu" opt; do
    case $opt in

    h)  # help information
        usage
        exit 0
        ;;
    i)  # install
        cp -v -r ./dtc_config_files ../package/feeds/
        echo "-------- done --------"
        exit 0
        ;;
    u)  # remove
        rm -i -v -rf ../package/feeds/dtc_config_files 
        echo "-------- done --------"
        exit 0
        ;;
    \?) # error, unknown options
        echo "-------- unknown options"
        error_info
        exit 1
        ;;
    esac
done

shift $(($OPTIND - 1))
