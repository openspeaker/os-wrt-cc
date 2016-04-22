include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=sound-os-dac-one
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/sound-os-dac-one
  SUBMENU:=Sound Support
  TITLE:=Linkit with Openspeaker hifi DAC ONE
  DEPENDS:= +kmod-sound-soc-core +kmod-regmap +kmod-i2c-ralink
  FILES:= \
	$(PKG_BUILD_DIR)/dac-one-gdma.ko \
	$(PKG_BUILD_DIR)/dac-one-i2s-ctrl.ko \
	$(PKG_BUILD_DIR)/dac-one-i2s.ko \
	$(PKG_BUILD_DIR)/dac-one-pcm.ko \
	$(PKG_BUILD_DIR)/dac-one-machine.ko \
	$(PKG_BUILD_DIR)/dac-one-codec-i2c.ko \
	$(PKG_BUILD_DIR)/dac-one-codec.ko
  AUTOLOAD:=$(call AutoLoad,90,dac-one-gdma dac-one-codec dac-one-codec-i2c dac-one-i2s-ctrl dac-one-i2s dac-one-pcm dac-one-machine)
endef

define KernelPackage/sound-os-dac-one/description
 Alsa modules for linkit smart with openspeaker hifi DAC ONE.
endef

EXTRA_CFLAGS :=
EXTRA_CFLAGS += -DCONFIG_MT7628 -DCONFIG_RALINK_MT7628
EXTRA_CFLAGS += -DRALINK_SYSCTL_BASE=0xB0000000
EXTRA_CFLAGS += -DRALINK_INTCL_BASE=0xB0000200
EXTRA_CFLAGS += -DRALINK_PIO_BASE=0xB0000600
EXTRA_CFLAGS += -DRALINK_I2S_BASE=0xB0000A00
EXTRA_CFLAGS += -DRALINK_GDMA_BASE=0xB0002800
EXTRA_CFLAGS += -DCONFIG_GDMA_EVERYBODY
EXTRA_CFLAGS += -DCONFIG_SND_MT76XX_SOC
EXTRA_CFLAGS += -DCONFIG_I2S_WM8960
EXTRA_CFLAGS += -DCONFIG_I2S_MCLK_12P288MHZ
EXTRA_CFLAGS += -DCONFIG_GDMA_EVERYBODY
EXTRA_CFLAGS += -DSURFBOARDINT_DMA=15
EXTRA_CFLAGS += -DRALINK_INTCTL_DMA=128
EXTRA_CFLAGS += -DCONFIG_SND_SOC_WM8960

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,sound-os-dac-one))

