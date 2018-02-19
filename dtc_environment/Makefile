include $(TOPDIR)/rules.mk

# Huanle Zhang
# www.huanlezhang.com
#
# Global Environment Variables:
# 	dtcBin: bin folder
# 	dtcConfig: config folder
# 	dtcRoot: root folder

PKG_NAME:=dtc_environment
PKG_VERSION:=1.0.1
PKG_RELEASE:=1

# environment variables must be same as in ./files/dtcEnvVariables
dtcRoot=/dtcRoot
dtcBin=/usr/dtcBin
dtcConfig=/etc/dtcConfig
dtcDebugDir=$(dtcRoot)/debug
dtcBackupDir=$(dtcRoot)/backup

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

myVar:=

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=DTC_OS
	CATEGORY:=DTC_OS
	TITLE:=Include configs and executables
	MAINTAINER:=Huanle Zhang
	URL:=www.huanlezhang.com
	DEFAULT:=y
	MENU:=1
endef

define Package/$(PKG_NAME)/description
    This is the root for dtc_* packages
	Including some customized files 
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/files
	$(CP) ./files/* $(PKG_BUILD_DIR)/files/
	$(CP) $(PKG_BUILD_DIR)/files/Makefile $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install

	rm -rf $(1)$(dtcRoot) 

	$(INSTALL_DIR) $(1)$(dtcBin)/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/bin/* $(1)$(dtcBin)/

	$(INSTALL_DIR) $(1)$(dtcConfig)/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/config/* $(1)$(dtcConfig)/

	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/init.d/* $(1)/etc/init.d/

	$(INSTALL_DIR) $(1)$(dtcRoot)/
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/files/root/* $(1)$(dtcRoot)/

	$(INSTALL_DIR) $(1)$(dtcDebugDir)
	$(INSTALL_DIR) $(1)$(dtcBackupDir)

	echo "IP=$(CONFIG_DTC_ETH_IP)" > $(1)$(dtcRoot)/dtcConfigs
	echo "HOSTNAME=$(CONFIG_DTC_HOST_NAME)" >> $(1)$(dtcRoot)/dtcConfigs
	
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
