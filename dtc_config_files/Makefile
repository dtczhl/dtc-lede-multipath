include $(TOPDIR)/rules.mk

# Huanle Zhang
# www.huanlezhang.com

PKG_NAME:=dtc_config_files
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=dtc
	CATEGORY:=dtc
	TITLE:=easy tools and configurations
	MAINTAINER:=Huanle Zhang
	URL:=www.huanlezhang.com
	DEFAULT:=y
endef

define Package/$(PKG_NAME)/description
    Customized tools and configurations
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/files
	$(CP) ./files/* $(PKG_BUILD_DIR)/files/
	$(CP) $(PKG_BUILD_DIR)/files/Makefile $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/config/* $(1)/etc/config/
	$(INSTALL_DIR) $(1)/root
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/root/* $(1)/root/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
