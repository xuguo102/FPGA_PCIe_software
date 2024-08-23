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
	{ 0x8c90f330, "pci_unregister_driver" },
	{ 0xdbd6ced7, "__pci_register_driver" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xd1c01878, "pci_read_config_dword" },
	{ 0x1b08bd39, "pci_read_config_byte" },
	{ 0xddce31ef, "cdev_add" },
	{ 0x8f1571b8, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0x18191baa, "dma_alloc_attrs" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x88a1d6c0, "pci_irq_vector" },
	{ 0x4ff4d77a, "pci_alloc_irq_vectors_affinity" },
	{ 0xaf924fc2, "pci_iomap" },
	{ 0x3f764d8a, "pci_request_regions" },
	{ 0x15b33556, "pci_set_master" },
	{ 0xdbe0711a, "pci_enable_device" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x40dc3334, "kmem_cache_alloc_trace" },
	{ 0x7e565f1e, "kmalloc_caches" },
	{ 0x37a0cba, "kfree" },
	{ 0x12a9d164, "pci_disable_device" },
	{ 0x9260dd60, "pci_clear_master" },
	{ 0xc46cf527, "pci_release_regions" },
	{ 0x2fa6ce8b, "pci_iounmap" },
	{ 0xdd7695b1, "pci_free_irq_vectors" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x5ddd8349, "dma_free_attrs" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x42d7baf9, "cdev_del" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x1000e51, "schedule" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x800473f, "__cond_resched" },
	{ 0xcf2a6966, "up" },
	{ 0x6bd0e573, "down_interruptible" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "A3EDE04FDDD569243FA0D63");
