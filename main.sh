#/bin/bash

# Huanle Zhang
# www.huanlezhang.com


# !!! change ...
target=3
# 1: Raspberry Pi 2
# 2: Netgear R7800
# 3: APU2
# 4: Raspberry Pi 3

kernel_version=4.9.54
wireless_version=4.14-rc2-1

CURRENT_PATH="$( cd "$(dirname "$0")"; pwd -P )"

build_directory_path=
kernel_linux_path=
kernel_wireless_path=

linux_debugfs_patch='990-dtc_debugfs.patch'
wireless_debugfs_patch='990-dtc_debugfs.patch'

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
Note: remember to change target in main.sh 
Usage: program [-h] [-i] [-k] [-l] [-w] [-p] [-u]
	-h	help information
	-i	initialize/download LEDE
	-k	install kernel files (linux+wireless)
	-l	save kernel files (linux)
	-w	save kernel files (wireless) 
	-p	install packages 
	-u	uninstall all     
Example: ./main.sh -k 
			install kernel files
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
		build_directory_path='build_dir/target-x86_64_musl/linux-x86_64'
		kernel_linux_path=${build_directory_path}'/linux-'${kernel_version}
		kernel_wireless_path=${build_directory_path}'/backports-'${wireless_version}
		;;
	4) # Raspbery Pi 3
		kernel_linux_path='build_dir/target-aarch64_cortex-a53_musl/linux-brcm2708_bcm2710/linux-'${kernel_version}
		;;
	*) 
		echo " unknown target in kernel_build_dir()"
		;;
	esac
}

error_info () {
    echo "  Error! Show help information with -h option"
}

check_caller_loc(){
	if [ ! -d dtc-lede-multipath ]; then
		echo -e "${ERROR_COLOR} ****Error ${NC}"
		echo -e "${ERROR_COLOR} Cannot find dtc-lede-multipath folder under current caller location ${NC}"
		exit
	fi
}

# starts here ----------------------------------
if [ $# -lt 1 ]; then
    error_info 
    exit 
fi

# get paths
kernel_linux_build_dir $target

while getopts ":hikuplw" opt; do
    case $opt in

    h)  # help information
        usage
        exit
        ;;
	i)	# initialize LEDE (download, config)
		if [ -d ../../dtcLede ] || [ -d ../dtcLede ]; then
			echo -e "${ERROR_COLOR} -------- seems you have already donwloaded LEDE!!! ${NC}"
			exit
		fi 
		echo -e "${HEAD_COLOR} -------- download and config LEDE ${NC}"
		git clone 'https://git.lede-project.org/source.git' ../dtcLede
		mv ../dtc-lede-multipath ../dtcLede/
		(
			cd ..
			cp dtc-lede-multipath/dtc_setup_lede/kernel-version.mk include/
			rm -rf package/kernel/mac80211
			cp -r dtc-lede-multipath/dtc_setup_lede/mac80211/ package/kernel/mac80211
			./scripts/feeds update -a
			./scripts/feeds install -a
			case $target in
			1) # raspberry pi 2
				cp dtc-lede-multipath/dtc_setup_lede/config_rasp2 .config
				;;
			2) # netgear r7800
				;;
			3) # apu2
				cp dtc-lede-multipath/dtc_setup_lede/config_apu2 .config
				;;
			4) # raspberry pi 3
				cp dtc-lede-multipath/dtc_setup_lede/config_rasp3 .config 
				;;
			*) # error
				echo -e "${ERROR_COLOR} -------- unkown target in i)"
				;;
			esac 
		)
		echo -e "${TAIL_COLOR} -------- download and config LEDE done -------- ${NC}"
		exit;;
    p)  # packages 
		check_caller_loc 
        echo -e "${HEAD_COLOR} -------- install customized packages ${NC}"
        cp -v -r dtc-lede-multipath/dtc_environment package/feeds/
        cp -v -r dtc-lede-multipath/dtc_packages/* package/feeds/
        echo -e "${TAIL_COLOR} -------- install customized packages done -------- ${NC}"
        exit
        ;;
	k)  # kernel files (linux+wireless)
		check_caller_loc 
		echo -e "${HEAD_COLOR} -------- install kernel files (linux+wireless) ${NC}"
		# install linux
		make target/linux/{clean,prepare} QUILT=1
		(
			cd $kernel_linux_path
			quilt push -a
			if [ ! -f patches/platform/${linux_debugfs_patch} ]; then
					quilt new platform/${linux_debugfs_patch}
			fi
		)
		cp -v dtc-lede-multipath/dtc_kernel/patches/linux/${linux_debugfs_patch} ${kernel_linux_path}/patches/platform/
		make target/linux/update
		# install wireless
		cp -v dtc-lede-multipath/dtc_kernel/patches/wireless/${wireless_debugfs_patch} package/kernel/mac80211/patches/
		echo -e "${TAIL_COLOR} -------- install kernel files (linux+wireless) done -------- ${NC}"
        exit
        ;;
	l)	# save kernel files (linux)
		check_caller_loc 
		echo -e "${HEAD_COLOR} -------- save kernel files (linux) ${NC}"
		if [ ! -f ${kernel_linux_path}/patches/platform/${linux_debugfs_patch} ]; then
			make target/linux/{clean,prepare} QUILT=1
		fi 
		cp -v ${kernel_linux_path}/patches/platform/${linux_debugfs_patch} dtc-lede-multipath/dtc_kernel/patches/linux/
		make target/linux/update
		echo -e "${TAIL_COLOR} -------- save kernel files (linux) done -------- ${NC}"
		exit	
		;;
	w)	# save kernel files (wireless)
		check_caller_loc 
		echo -e "${HEAD_COLOR} -------- save kernel files (wireless) ${NC}"
		if [ -f ${kernel_wireless_path}/patches/${wireless_debugfs_patch} ]; then 
			cp -v ${kernel_wireless_path}/patches/${wireless_debugfs_patch} package/kernel/mac80211/patches/
		fi 
		cp -v package/kernel/mac80211/patches/${wireless_debugfs_patch} dtc-lede-multipath/dtc_kernel/patches/wireless/
		echo -e "${TAIL_COLOR} -------- save kernel files (wireless) done -------- ${NC}"
		exit
		;;
    u)  # remove all, recover to original
		check_caller_loc 
		echo -e "${HEAD_COLOR} -------- removing all ${NC}"
		# remove packages
        rm -i -v -rf package/feeds/dtc_* 
		rm -i -vf dl/dtc_*
		# remove kernel files (linux)
		make target/linux/{clean,prepare} QUILT=1
		#kernel_linux_build_dir $target 
		(
			cd $kernel_linux_path 
			if [ -f patches/platform/$linux_debugfs_patch ]; then
				quilt delete platform/$linux_debugfs_patch
			fi 
		)
		make target/linux/update
		make target/linux/clean 
		# remove kernel files (wireless)
		if [ -f package/kernel/mac80211/patches/${wireless_debugfs_patch} ]; then 
			rm package/kernel/mac80211/patches/${wireless_debugfs_patch}
		fi 
		make package/kernel/mac80211/clean 
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
