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
	"96328A-1241N")
		board_model="Comtrend AR-5381u"
		status_led="AR-5381u:green:power"
		brcm63xx_has_reset_button="true"
		;;
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
	"96369R-1231N")
		board_model="Comtrend WAP-5813n"
		status_led="WAP-5813n:green:power"
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
	"HW553")
		board_model="Huawei HG553"
		status_led="HG553:blue:power"
		brcm63xx_has_reset_button="true"
		;;
	"HW556")
		board_model="Huawei HG556a (Unknown)"
		status_led="HG556a:red:power"
		brcm63xx_has_reset_button="true"
		;;
	"HW556_A")
		board_model="Huawei HG556a (Ver A - Ralink)"
		status_led="HG556a:red:power"
		brcm63xx_has_reset_button="true"
		;;
	"HW556_B")
		board_model="Huawei HG556a (Ver B - Atheros)"
		status_led="HG556a:red:power"
		brcm63xx_has_reset_button="true"
		;;
	"HW556_C")
		board_model="Huawei HG556a (Ver C - Atheros)"
		status_led="HG556a:red:power"
		brcm63xx_has_reset_button="true"
		;;
	"HW6358GW_B")
		board_model="Huawei HG520v"
		status_led="HG520v:green:ppp"
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
