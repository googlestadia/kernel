#include <linux/module.h>

static int __init dummy_amdkfd_init(void)
{
	return 0;
}

static void __exit dummy_amdkfd_exit(void)
{
}

module_init(dummy_amdkfd_init);
module_exit(dummy_amdkfd_exit);

MODULE_AUTHOR("Stadia Graphics Infra team");
MODULE_DESCRIPTION("DUMMY AMD KFD");
MODULE_LICENSE("GPL and additional rights");
MODULE_VERSION("5.0.19.20.6");
