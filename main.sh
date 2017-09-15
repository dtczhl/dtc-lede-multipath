#/bin/sh

# Huanle Zhang
# www.huanlezhang.com

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

patch_kernel(){
    curDir=$(pwd)
    buildDir=build_dir/target-x86_64_musl/linux-x86_64/linux-4.9.44
    
    echo " ----------- patching kernel "
    cd ..
    make target/linux/clean 
    make target/linux/prepare QUILT=1 
    cd $buildDir 
    quilt push -a
    quilt new platform/999-dtc.patch
    quilt refresh
    
    cd $curDir 
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
        cp -v -r ./dtc_packages/* ../package/feeds/
        patch_kernel 
        echo "-------- installing done --------"
        exit 0
        ;;
    u)  # remove
        rm -i -v -rf ../package/feeds/dtc_* 
        echo "-------- removing done --------"
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
