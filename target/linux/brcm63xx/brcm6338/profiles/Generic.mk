define Profile/6338GW
  NAME:=Generic 6338GW
  PACKAGES:=kmod-b43 wpad-mini
endef
define Profile/6338GW/Description
	Package set for the 6338GW
endef
$(eval $(call Profile,6338GW))


define Profile/6338W
  NAME:=Generic 6338W
  PACKAGES:=kmod-b43 wpad-mini
endef
define Profile/6338W/Description
	Package set for the 6338W
endef
$(eval $(call Profile,6338W))
