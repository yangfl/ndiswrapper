#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/kprobes.h>
#include <linux/version.h>

typedef unsigned long (*kallsyms_lookup_name_t)(const char*);
typedef void* (*__vmalloc_node_range_t)(unsigned long, unsigned long, unsigned long, unsigned long, gfp_t, pgprot_t, unsigned long, int ,const void*);

static __vmalloc_node_range_t __vmalloc_node_range_ptr = NULL;

int nvmalloc_init(void)
{
	static struct kprobe kp = {
		.symbol_name = "kallsyms_lookup_name"
	};

	kallsyms_lookup_name_t kallsyms_lookup_name;

	register_kprobe(&kp);
	kallsyms_lookup_name = (kallsyms_lookup_name_t)kp.addr;
	unregister_kprobe(&kp);

	__vmalloc_node_range_ptr = (__vmalloc_node_range_t)kallsyms_lookup_name("__vmalloc_node_range");
	if (__vmalloc_node_range_ptr)
		return 0;

	__vmalloc_node_range_ptr = (__vmalloc_node_range_t)kallsyms_lookup_name("__vmalloc_node_range_noprof");
	if (__vmalloc_node_range_ptr)
		return 0;

	return -ENOSYS;
}

void *nvmalloc(unsigned long size, gfp_t gfp_mask, pgprot_t prot)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,3,0)
	return __vmalloc(size, gfp_mask, prot);
#else
	return __vmalloc_node_range_ptr(size, 1, VMALLOC_START, VMALLOC_END, gfp_mask, prot, 0, NUMA_NO_NODE, __builtin_return_address(0));
#endif
}
