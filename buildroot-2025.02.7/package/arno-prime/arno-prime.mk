ARNO_PRIME_VERSION = 1.0
ARNO_PRIME_SITE = $(TOPDIR)/package/arno-prime
ARNO_PRIME_SITE_METHOD = local

$(eval $(kernel-module))
$(eval $(generic-package))