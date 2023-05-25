#include <kernel/compiler.h>
#include <kernel/mm.h>

DEFINE_BOOT_PAGE_TABLE(boot_level_0);
DEFINE_BOOT_PAGE_TABLE(boot_level_1);
DEFINE_BOOT_PAGE_TABLE(boot_level_2);
DEFINE_BOOT_PAGE_TABLE(boot_level_3);

DEFINE_BOOT_PAGE_TABLE(boot_id_level_1);
DEFINE_BOOT_PAGE_TABLE(boot_id_level_2);
DEFINE_BOOT_PAGE_TABLE(boot_id_level_3);

DEFINE_BOOT_PAGE_TABLE(hyp_fixmap);

unsigned char __initdata boot_stack[STACK_SIZE]
		__attribute__((__aligned__(STACK_SIZE)));