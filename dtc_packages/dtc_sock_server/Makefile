include $(TOPDIR)/rules.mk

# Huanle Zhang
# www.huanlezhang.com

PKG_NAME:=dtc_sock_server
PKG_RELEASE:=1

# PKG_SOURCE_URL:=file://$(TOPDIR)/package/$(PKG_NAME)/src

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=dtc
	CATEGORY:=dtc
	TITLE:=sock app for measuring bandwidth and latency
	DEFAULT:=y
endef

define Package/$(PKG_NAME)/description
	Lalala
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
