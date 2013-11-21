define Profile/DSL2650U
  NAME:=D-Link DSL-2650U
  PACKAGES:=kmod-b43 wpad-mini
endef
define Profile/DSL2650U/Description
	Package set for the DSL-2650U
endef
$(eval $(call Profile,DSL2650U))
