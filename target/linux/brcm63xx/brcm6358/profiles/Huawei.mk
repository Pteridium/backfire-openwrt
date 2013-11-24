define Profile/HG520V
  NAME:=Huawei HG520v
  PACKAGES:=kmod-b43 wpad-mini
endef
define Profile/HG520V/Description
	Package set for the HG520v
endef
$(eval $(call Profile,HG520V))
