#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

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

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x8ae91ffa, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x7e565f1e, "kmalloc_caches" },
	{ 0x684190b4, "pci_write_config_dword" },
	{ 0x8f1571b8, "cdev_init" },
	{ 0xdd7695b1, "pci_free_irq_vectors" },
	{ 0x754d539c, "strlen" },
	{ 0x1b08bd39, "pci_read_config_byte" },
	{ 0x12a9d164, "pci_disable_device" },
	{ 0x17f1b3b8, "device_destroy" },
	{ 0x6729d3df, "__get_user_4" },
	{ 0xc46cf527, "pci_release_regions" },
	{ 0x5ddd8349, "dma_free_attrs" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x15b33556, "pci_set_master" },
	{ 0x4ff4d77a, "pci_alloc_irq_vectors_affinity" },
	{ 0xe834d12e, "pci_enable_pcie_error_reporting" },
	{ 0x2fa6ce8b, "pci_iounmap" },
	{ 0x449ad0a7, "memcmp" },
	{ 0x76336d73, "fasync_helper" },
	{ 0x18191baa, "dma_alloc_attrs" },
	{ 0x20d303f3, "irq_get_irq_data" },
	{ 0x4449593, "device_create" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x9260dd60, "pci_clear_master" },
	{ 0xddce31ef, "cdev_add" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x6d334118, "__get_user_8" },
	{ 0x92997ed8, "_printk" },
	{ 0xd1c01878, "pci_read_config_dword" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x8c90f330, "pci_unregister_driver" },
	{ 0x40dc3334, "kmem_cache_alloc_trace" },
	{ 0x88a1d6c0, "pci_irq_vector" },
	{ 0x37a0cba, "kfree" },
	{ 0x3f764d8a, "pci_request_regions" },
	{ 0xdbd6ced7, "__pci_register_driver" },
	{ 0x37b7090d, "class_destroy" },
	{ 0x410e4384, "kill_fasync" },
	{ 0xaf924fc2, "pci_iomap" },
	{ 0xdbe0711a, "pci_enable_device" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xf3da7aea, "__class_create" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xc1514a3b, "free_irq" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "D0349959754B0EECC8A0A44");
