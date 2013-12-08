define Profile/HG520V
  NAME:=Huawei HG520v
  PACKAGES:=kmod-b43 wpad-mini
endef
define Profile/HG520V/Description
	Package set for the HG520v
endef
$(eval $(call Profile,HG520V))

define Profile/HG553
  NAME:=Huawei HG553
  PACKAGES:=kmod-b43 wpad-mini kmod-usb-core kmod-usb-ohci kmod-usb2
endef
define Profile/HG553/Description
	Package set for the HG553
endef
$(eval $(call Profile,HG553))

define Profile/HG556A
  NAME:=Huawei HG556a
  PACKAGES:=kmod-usb-core kmod-usb-ohci kmod-usb2
endef
define Profile/HG556A/Description
	Package set for the HG556a
endef
$(eval $(call Profile,HG556A))
