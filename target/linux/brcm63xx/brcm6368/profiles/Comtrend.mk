define Profile/VR3025U
  NAME:=Comtrend VR-3025u
  PACKAGES:=kmod-b43 wpad-mini
endef
define Profile/VR3025U/Description
	Package set for the VR-3025u
endef
$(eval $(call Profile,VR3025U))
