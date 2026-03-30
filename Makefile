include $(TOPDIR)/rules.mk

PKG_NAME:=tuya-esp
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/tuya-esp
	CATEGORY:=Base system
	TITLE:=tuya-esp
	DEPENDS:=+libtuyasdk +libubox +libubus +libblobmsg-json +libuci
endef

define Package/tuya-esp/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DIR) $(1)/etc/init.d/
	
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tuya-esp $(1)/usr/bin/
	$(INSTALL_BIN) ./files/tuya-esp.init $(1)/etc/init.d/tuya-esp
	$(INSTALL_CONF) ./files/tuya-esp.config $(1)/etc/config/tuya-esp
 endef

$(eval $(call BuildPackage,tuya-esp))
