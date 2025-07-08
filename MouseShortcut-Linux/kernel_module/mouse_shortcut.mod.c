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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const char ____versions[]
__used __section("__versions") =
	"\x10\x00\x00\x00\x5a\x25\xd5\xe2"
	"strcmp\0\0"
	"\x14\x00\x00\x00\x24\xcf\xcc\x37"
	"init_net\0\0\0\0"
	"\x20\x00\x00\x00\xf3\x5c\xec\x35"
	"__netlink_kernel_create\0"
	"\x20\x00\x00\x00\x87\xbd\xc3\x0c"
	"input_register_handler\0\0"
	"\x20\x00\x00\x00\x75\x84\xbd\x23"
	"netlink_kernel_release\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x24\x00\x00\x00\x23\x68\xca\xd8"
	"input_unregister_handler\0\0\0\0"
	"\x14\x00\x00\x00\x6e\x4a\x6e\x65"
	"snprintf\0\0\0\0"
	"\x14\x00\x00\x00\x70\x27\xa6\x37"
	"__alloc_skb\0"
	"\x14\x00\x00\x00\x6f\xdb\x1a\x17"
	"__nlmsg_put\0"
	"\x10\x00\x00\x00\x94\xb6\x16\xa9"
	"strnlen\0"
	"\x18\x00\x00\x00\x22\x70\x4d\x29"
	"netlink_unicast\0"
	"\x1c\x00\x00\x00\x65\x62\xf5\x2c"
	"__dynamic_pr_debug\0\0"
	"\x1c\x00\x00\x00\xcd\x8d\x21\x94"
	"kfree_skb_reason\0\0\0\0"
	"\x18\x00\x00\x00\x8c\x89\xd4\xcb"
	"fortify_panic\0\0\0"
	"\x1c\x00\x00\x00\x63\xa5\x03\x4c"
	"random_kmalloc_seed\0"
	"\x18\x00\x00\x00\x52\x57\xa3\x91"
	"kmalloc_caches\0\0"
	"\x18\x00\x00\x00\x19\x4e\x00\x34"
	"kmalloc_trace\0\0\0"
	"\x20\x00\x00\x00\x0f\xf0\x78\x88"
	"input_register_handle\0\0\0"
	"\x1c\x00\x00\x00\xb1\xc8\x57\xc4"
	"input_open_device\0\0\0"
	"\x18\x00\x00\x00\x4c\xf1\x5a\xb3"
	"param_ops_int\0\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x1c\x00\x00\x00\x1b\xdb\x7a\x89"
	"input_close_device\0\0"
	"\x20\x00\x00\x00\xe5\x8e\xb0\x5c"
	"input_unregister_handle\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x18\x00\x00\x00\x3a\x0a\xd8\xfc"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "7A3C61AC20A656C5C47233C");
