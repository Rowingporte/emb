HCSR04_VERSION = 1.0
HCSR04_SITE = $(TOPDIR)/package/HCSR04
HCSR04_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))