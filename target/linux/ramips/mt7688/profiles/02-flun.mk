#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/FLun
	NAME:=FLun
	PACKAGES:=\
		kmod-usb-core kmod-usb2 kmod-usb-ohci \
		kmod-ledtrig-netdev
endef

define Profile/FLun/Description
	Default package set compatible with most boards.
endef
$(eval $(call Profile,FLun))
