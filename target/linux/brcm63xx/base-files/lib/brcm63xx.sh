#!/bin/sh
# Copyright (C) 2013 OpenWrt.org

board_name=""
board_model=""
status_led=""
brcm63xx_has_reset_button=""
ifname="eth0"

brcm63xx_detect() {
	board_name=$(awk 'BEGIN{FS="[ \t:/]+"} /system type/ {print $4}' /proc/cpuinfo)

	case "$board_name" in
	"96338GW")
		board_model="Generic 96338GW"
		status_led="96338GW:green:power"
		;;
	"96338W")
		board_model="Generic 96338W"
		status_led="96338W:green:power"
		;;
	"96345GW2")
		board_model="Generic 96345GW2"
		;;
	"96348GW-11")
		board_model="Generic 96348GW-11"
		status_led="96348GW-11:green:power"
		brcm63xx_has_reset_button="true"
		;;
	"96358VW2")
		board_model="D-Link DSL-2650U"
		status_led="DSL-2650U:green:power"
		;;
	"96368M-1341N")
		board_model="Comtrend VR-3025un"
		status_led="VR-3025un:green:power"
		brcm63xx_has_reset_button="true"
		;;
	"96368M-1541N")
		board_model="Comtrend VR-3025u"
		status_led="VR-3025u:green:power"
		brcm63xx_has_reset_button="true"
		;;
	"CT-5621")
		board_model="Comtrend 5621"
		status_led="CT-5621:green:power"
		brcm63xx_has_reset_button="true"
		;;
	"CT-536+")
		board_model="Comtrend 536+"
		status_led="CT-536+:green:power"
		brcm63xx_has_reset_button="true"
		;;
	"CT-5361")
		board_model="Comtrend 5361"
		status_led="CT-5361:green:power"
		brcm63xx_has_reset_button="true"
		;;
	"HW6358GW_B")
		board_model="Huawei HG520v"
		status_led="HG520v:green:net"
		brcm63xx_has_reset_button="true"
		;;
	*)
		board_model="Unknown"
		;;
	esac

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"
	[ -e "/tmp/sysinfo/board_name" ] || echo "$board_name" > /tmp/sysinfo/board_name
	[ -e "/tmp/sysinfo/model" ] || echo "$board_model" > /tmp/sysinfo/model
}

brcm63xx_detect
