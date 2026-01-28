#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_MITIGATION_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xb5987758, "__platform_driver_register" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xe189abd8, "devm_kmalloc" },
	{ 0xbbef630c, "devm_gpiod_get" },
	{ 0x2a71d93b, "__register_chrdev" },
	{ 0xf3a226b, "_dev_info" },
	{ 0x2ab8e3ab, "_dev_err" },
	{ 0x5ac19ccb, "platform_driver_unregister" },
	{ 0x5f754e5a, "memset" },
	{ 0xb7e3aff3, "gpiod_set_value" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x1d4aae90, "gpiod_get_value" },
	{ 0xb43f9365, "ktime_get" },
	{ 0xa6a7a2ad, "div_s64_rem" },
	{ 0x314b20c8, "scnprintf" },
	{ 0x51a910c0, "arm_copy_to_user" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x5d3c04b4, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cinsa,hcsr04");
MODULE_ALIAS("of:N*T*Cinsa,hcsr04C*");
