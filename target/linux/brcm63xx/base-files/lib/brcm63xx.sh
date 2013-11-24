#!/bin/sh

board_name=""
board_model=""

brcm63xx_detect() {
	board_name=$(awk 'BEGIN{FS="[ \t:/]+"} /system type/ {print $4}' /proc/cpuinfo)

	case "$board_name" in
	96348GW-11)
		board_model="Comtrend CT-5621/CT-536+/CT-5361"
		;;
	96358VW2)
		board_model="D-Link DSL-2650U"
		;;
	HW6358GW_B)
		board_model="Huawei HG520v"
		;;
	*)
		board_model="Unknown"
		;;
	esac

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"
	echo "$board_name" > /tmp/sysinfo/board_name
	echo "$board_model" > /tmp/sysinfo/model
}

brcm63xx_detect
