include $(TOPDIR)/rules.mk

# Huanle Zhang
# www.huanlezhang.com

PKG_NAME:=dtc_show_ip
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_RELEASE).tar.xz
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/dtczhl/dtc-show-ip.git
PKG_SOURCE_VERSION:=e770e083421bac4e200d929a3b6d300cd42ec80e

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=dtc
	CATEGORY:=dtc
	TITLE:=dtc_show_ip 
	DEFAULT:=y
endef

define Package/$(PKG_NAME)/description
	Upload and show your IP address to my website:
	www.huanlezhang.com/interface
endef

define Build/Prepare
	$(Build/Prepare/Default)
	tar -xf $(TOPDIR)/dl/$(PKG_SOURCE) -C $(PKG_BUILD_DIR) --strip 1
	echo 'target: ;' > $(PKG_BUILD_DIR)/Makefile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/inface
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/90-dtcShowIp $(1)/etc/hotplug.d/iface/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dtc-show-ip.sh $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
