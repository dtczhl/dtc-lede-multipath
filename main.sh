#/bin/bash

# Huanle Zhang
# www.huanlezhang.com


# !!! change ...
target=1
# 1: Raspberry Pi 2
# 2: Netgear R7800
# 3: APU2

kernel_version=4.9.51
kernel_linux_path=

kernel_debugfs_patch='990-dtc_debugfs.patch'


# color output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

HEAD_COLOR=$YELLOW
TAIL_COLOR=$GREEN 
ERROR_COLOR=$RED

usage () {
    cat <<EOF
Usage: program [-h] [-i] [-k] [-s] [-w] [-p] [-u]
	-h	help information
	-i	initialize LEDE
	-k	install kernel files (linux+wireless)
	-s	save kernel files (linux)
	-w	save kernel files (wireless) 
	-p	install packages 
	-u	uninstall all     
Example: ./main.sh -k to install kernel files
EOF
}

kernel_linux_build_dir () {
	# return kernel_linux_path
	if [ $# -ne 1 ]; then
		echo " Error! build_dir(), num of input != 1"
		echo " Format: build_dir"
		exit
	fi
	case $1 in
	
	1) # Raspberry Pi 2
		kernel_linux_path='build_dir/target-arm_cortex-a7+neon-vfpv4_musl_eabi/linux-brcm2708_bcm2709/linux-'${kernel_version}
		;;
	2) # Netgear R7800
		;;
	3) # APU 2
		;;
	*) 
		echo " unknown target in kernel_build_dir()"
		;;
	esac
}

error_info () {
    echo "  Error! Show help information with -h option"
}

# starts here ----------------------------------
if [ $# -lt 1 ]; then
    error_info 
    exit 
fi

# get paths
kernel_linux_build_dir $target


while getopts ":hikupsw" opt; do
    case $opt in

    h)  # help information
        usage
        exit
        ;;
	i)	# initialize LEDE (download, config)
		echo -e "${HEAD_COLOR} -------- download LEDE to dtcLede folder ${NC}"
		git clone 'https://git.lede-project.org/source.git' dtcLede
		mv -r dtcLede/* ../
		exit;;

    p)  # packages
        echo -e "${HEAD_COLOR} -------- install customized packages ${NC}"
        cp -v -r ./dtc_config_files ../package/feeds/
        cp -v -r ./dtc_packages/* ../package/feeds/
        echo -e "${TAIL_COLOR} -------- install customized packages done -------- ${NC}"
        exit
        ;;
	k)  # kernel files (linux+wireless)
		echo -e "${HEAD_COLOR} -------- install kernel files (linux+wireless) ${NC}"
		(
			cd ..
			make target/linux/{clean,prepare} QUILT=1
			cd $kernel_linux_path
			quilt push -a
			if [ ! -f platform/${kernel_linux_path} ]; then
					quilt new platform/${kernel_debugfs_patch}
			fi 
		)
		(
			cd ..
			cp -v ./dtc-lede-multipath/dtc_kernel/patches/linux/${kernel_debugfs_patch} ${kernel_linux_path}/patches/platform/
		)
		(
			cd ..
			cd $kernel_linux_path 
		)
		(
			cd ..
			make target/linux/update
			make target/linux/clean 
		)
		echo -e "${TAIL_COLOR} -------- install kernel files (linux+wireless) done -------- ${NC}"
        exit
        ;;
	s)	# save kernel files (linux)
		echo -e "${HEAD_COLOR} -------- save kernel files (linux) ${NC}"
	    cp -v ../${kernel_linux_path}/patches/platform/${kernel_debugfs_patch} ./dtc_kernel/patches/linux/
		(
			cd ..
			make target/linux/update
			make target/linux/clean 
		)
		echo -e "${TAIL_COLOR} -------- save kernel files (linux) done -------- ${NC}"
		exit	
		;;
	w)	# save kernel files (wireless)
		echo -e "${HEAD_COLOR} -------- save kernel files (wireless) ${NC}"
		echo -e "${TAIL_COLOR} -------- save kernel files (wireless) done -------- ${NC}"
		exit
		;;
    u)  # remove all, recover to original
		echo -e "${HEAD_COLOR} -------- removing all ${NC}"
		# remove packages
        rm -i -v -rf ../package/feeds/dtc_* 
		# remove kernel files (linux)
		(
			cd ..
			make target/linux/{clean,prepare} QUILT=1
			#kernel_linux_build_dir $target 
			cd $kernel_linux_path 
			if [ -f patches/platform/$kernel_debugfs_patch ]; then
					quilt delete platform/$kernel_debugfs_patch
			fi 
		)
		(
			cd ..
			make target/linux/update
			make target/linux/clean 
		)
		# remove kernel files (wireless)
		# to do ...
        echo -e "${TAIL_COLOR} -------- removing all done -------- ${NC}"
        exit
        ;;
    \?) # error, unknown options
        echo -e "${ERROR_COLOR} -------- unknown options ${NC}"
        error_info
        exit
        ;;
    esac
done

shift $(($OPTIND - 1))
