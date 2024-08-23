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

KSYMTAB_FUNC(amdexerciser_exit, "_gpl", "");

SYMBOL_CRC(amdexerciser_exit, 0x5791d6f0, "_gpl");

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x6ddeb84f, "kill_fasync" },
	{ 0x63b3f905, "fasync_helper" },
	{ 0xe8e5ec15, "device_destroy" },
	{ 0x704fc943, "class_destroy" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x449ad0a7, "memcmp" },
	{ 0xa3b148e8, "dma_free_attrs" },
	{ 0xeb3576b7, "irq_get_irq_data" },
	{ 0x754d539c, "strlen" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xcda921d3, "pci_iounmap" },
	{ 0xb0a80359, "pci_release_regions" },
	{ 0x614e8a22, "pci_clear_master" },
	{ 0xd7e82a12, "pci_disable_device" },
	{ 0x37a0cba, "kfree" },
	{ 0x95d99445, "pci_free_irq_vectors" },
	{ 0x25878ca1, "__pci_register_driver" },
	{ 0x2617cd97, "pci_unregister_driver" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x4c03a563, "random_kmalloc_seed" },
	{ 0xf2999cd4, "kmalloc_caches" },
	{ 0x4524dc3a, "kmalloc_trace" },
	{ 0xc8d6abde, "pci_enable_device" },
	{ 0x2ab6e728, "pci_set_master" },
	{ 0xd6818f74, "pci_request_regions" },
	{ 0xb0f313dc, "pci_iomap" },
	{ 0xce752fe2, "pci_alloc_irq_vectors" },
	{ 0x5b85a250, "pci_irq_vector" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0xee21a974, "dma_alloc_attrs" },
	{ 0xf5f85346, "cdev_init" },
	{ 0x63597d5f, "cdev_add" },
	{ 0x1dee04a0, "class_create" },
	{ 0x714f9a77, "device_create" },
	{ 0xeb708ef5, "pci_read_config_dword" },
	{ 0xe67ee00f, "pci_read_config_byte" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0x6729d3df, "__get_user_4" },
	{ 0x6d334118, "__get_user_8" },
	{ 0xc6c3c970, "pci_write_config_dword" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x122c3a7e, "_printk" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x708cd699, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "7744CDED09743332AB2A999");
