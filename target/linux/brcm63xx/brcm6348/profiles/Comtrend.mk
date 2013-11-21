define Profile/CT5621
  NAME:=Comtrend CT-5621
  PACKAGES:=kmod-b43 wpad-mini
endef
define Profile/CT5621/Description
	Package set for the CT-5621
endef
$(eval $(call Profile,CT5621))

define Profile/CT536PLUS
  NAME:=Comtrend CT-536+
  PACKAGES:=kmod-b43 wpad-mini
endef
define Profile/CT536PLUS/Description
	Package set for the CT-536+
endef
$(eval $(call Profile,CT536PLUS))

define Profile/CT5361
  NAME:=Comtrend CT-5361
  PACKAGES:=kmod-b43 wpad-mini
endef
define Profile/CT5361/Description
	Package set for the CT-5361
endef
$(eval $(call Profile,CT5361))
