PART_NAME=linux
platform_check_image() {
	[ "$ARGC" -gt 1 ] && return 1
	case "$(get_magic_word "$1")" in
		3600|3700|3800)
			return 0
			;;
		*)
			echo "Invalid image type. Please use only .bin files"
			return 1
			;;
	esac
}
