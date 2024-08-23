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
	{ 0x6a8274a3, "kill_fasync" },
	{ 0x7bc1942f, "fasync_helper" },
	{ 0xf7be671b, "device_destroy" },
	{ 0x92ce99, "class_destroy" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x449ad0a7, "memcmp" },
	{ 0x1a194a4f, "dma_free_attrs" },
	{ 0xad026174, "irq_get_irq_data" },
	{ 0x754d539c, "strlen" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x7b695bdf, "pci_iounmap" },
	{ 0xea6f0800, "pci_release_regions" },
	{ 0xbd1b8f20, "pci_clear_master" },
	{ 0x50e6d0f5, "pci_disable_device" },
	{ 0x37a0cba, "kfree" },
	{ 0xb9afdc4e, "pci_free_irq_vectors" },
	{ 0xcee81a5f, "__pci_register_driver" },
	{ 0xb55ef4e9, "pci_unregister_driver" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xb88db70c, "kmalloc_caches" },
	{ 0x4454730e, "kmalloc_trace" },
	{ 0x35e7f1fa, "pci_enable_pcie_error_reporting" },
	{ 0xc5d61c97, "pci_enable_device" },
	{ 0x18bb35f9, "pci_set_master" },
	{ 0x38f3e73f, "pci_request_regions" },
	{ 0x90f7aeb2, "pci_iomap" },
	{ 0xd1a5af3c, "pci_alloc_irq_vectors" },
	{ 0x93c67f58, "pci_irq_vector" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x2bd1e17f, "dma_alloc_attrs" },
	{ 0xa6f7a612, "cdev_init" },
	{ 0xf4407d6b, "cdev_add" },
	{ 0x1399bb1, "class_create" },
	{ 0xd3044a78, "device_create" },
	{ 0xe0ecd44f, "pci_read_config_dword" },
	{ 0x5ab1fd1d, "pci_read_config_byte" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0x6729d3df, "__get_user_4" },
	{ 0x6d334118, "__get_user_8" },
	{ 0x1a595472, "pci_write_config_dword" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x122c3a7e, "_printk" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x2fa5cadd, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "99A1AE03AB8311024F33A76");
