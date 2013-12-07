define Profile/VR3025U
  NAME:=Comtrend VR-3025u
  PACKAGES:=kmod-usb-core kmod-usb-ohci kmod-usb2
endef
define Profile/VR3025U/Description
	Package set for the VR-3025u
endef
$(eval $(call Profile,VR3025U))

define Profile/VR3025UN
  NAME:=Comtrend VR-3025un
  PACKAGES:=kmod-usb-core kmod-usb-ohci kmod-usb2
endef
define Profile/VR3025UN/Description
	Package set for the VR-3025un
endef
$(eval $(call Profile,VR3025UN))

define Profile/WAP5813N
  NAME:=Comtrend WAP-5813n
endef
define Profile/WAP5813N/Description
	Package set for the WAP-5813n
endef
$(eval $(call Profile,WAP5813N))
