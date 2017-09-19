#/bin/sh

# Huanle Zhang
# www.huanlezhang.com

usage () {
    cat <<EOF
Usage: program [-h] [-k] [-p] [-u]
    -h   help information
    -k   install kernel patches 
    -p   install packages 
    -u   uninstall all     
Example: ./main.sh -u to install 
EOF
}

error_info () {
    echo "  Error! Show help information with -h option"
}

# starts here
if [ $# -lt 1 ]; then
    error_info 
    exit 
fi

while getopts ":hiup" opt; do
    case $opt in

    h)  # help information
        usage
        exit
        ;;
    p)  # packages
        echo "-------- install customized packages"
        cp -v -r ./dtc_config_files ../package/feeds/
        cp -v -r ./dtc_packages/* ../package/feeds/
        echo "-------- install customized packages done --------"
        exit
        ;;
    k)  # kernel patches
        echo "-------- install kernel patches"
        exit
        ;;
    u)  # remove
        rm -i -v -rf ../package/feeds/dtc_* 
        echo "-------- removing done --------"
        exit
        ;;
    \?) # error, unknown options
        echo "-------- unknown options"
        error_info
        exit
        ;;
    esac
done

shift $(($OPTIND - 1))
