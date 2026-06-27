// SPDX-License-Identifier: GPL-2.0
/*
 * Author: Mikhail Kshevetskiy <mikhail.kshevetskiy@iopsys.eu>
 */
#include <fdt_support.h>
#include <init.h>
#include <sysreset.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

#define EN7523_DRAM_BASE		0x80000000ULL

#define EN7523_DRAMC_BASE		0x1fb24000UL
#define EN7523_DRAMC_RKCFG		0x034
#define EN7523_DRAMC_RKSIZE_MASK	GENMASK(18, 16)
#define EN7523_DRAMC_RKSIZE_SHIFT	16
#define EN7523_DRAMC_RKMODE_MASK	GENMASK(7, 4)
#define EN7523_DRAMC_RKMODE_SHIFT	4
#define EN7523_DRAMC_THREE_RANKS	BIT(8)

int print_cpuinfo(void)
{
	printf("CPU:   Airoha EN7523/EN7529/EN7562\n");
	return 0;
}

/*
 * The DRAM controller is initialized and trained before U-Boot starts.
 * Read the final rank configuration left by the previous boot stage instead
 * of reprogramming the controller or running DDR training again.
 */
static phys_size_t en7523_dramc_get_size(void)
{
	void __iomem *dramc = (void __iomem *)EN7523_DRAMC_BASE;
	unsigned int boundary;
	unsigned int rank_count;
	unsigned int rank_mode;
	unsigned int rank_size;
	u32 rkcfg;

	rkcfg = readl(dramc + EN7523_DRAMC_RKCFG);

	rank_size = (rkcfg & EN7523_DRAMC_RKSIZE_MASK) >>
		    EN7523_DRAMC_RKSIZE_SHIFT;
	rank_mode = (rkcfg & EN7523_DRAMC_RKMODE_MASK) >>
		    EN7523_DRAMC_RKMODE_SHIFT;

	rank_count = rank_mode ? 2 : 1;
	if (rkcfg & EN7523_DRAMC_THREE_RANKS)
		rank_count = 3;

	/*
	 * With one rank, RKSIZE selects the highest populated address bit.
	 * With multiple ranks, it selects the rank boundary address bit.
	 */
	boundary = 31 - rank_size;

	debug("EN7523 DRAMC: RKCFG=%08x RKSIZE=%u RKMODE=%u ranks=%u\n",
	      rkcfg, rank_size, rank_mode, rank_count);

	if (rank_count == 1)
		return BIT_ULL(boundary + 1);

	return BIT_ULL(boundary) * rank_count;
}

int dram_init(void)
{
	phys_size_t size;

	size = en7523_dramc_get_size();

	/*
	 * Reject impossible or clearly invalid controller values. DRAM starts at
	 * 0x80000000, so no more than 1 GiB can be represented below 2 GiB.
	 */
	if (size < SZ_32M || size > SZ_1G) {
		printf("Invalid EN7523 DRAM size: %lu MiB\n",
		       (size >> 20));
		return -EINVAL;
	}

	gd->ram_base = EN7523_DRAM_BASE;
	gd->ram_size = size;

	debug("EN7523 DRAM: base=%lx size=%lu MiB\n",
	      gd->ram_base, (gd->ram_size >> 20));

	return 0;
}

int dram_init_banksize(void)
{
	int bank;

	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = gd->ram_size;

	for (bank = 1; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		gd->bd->bi_dram[bank].start = 0;
		gd->bd->bi_dram[bank].size = 0;
	}

	return 0;
}

#ifdef CONFIG_OF_SYSTEM_SETUP
int ft_system_setup(void *blob, struct bd_info *bd)
{
	u64 start[1] = { gd->ram_base };
	u64 size[1] = { gd->ram_size };

	(void)bd;

	return fdt_fixup_memory_banks(blob, start, size, 1);
}
#endif

void __noreturn reset_cpu(void)
{
	writel(0x80000000, 0x1FB00040);
	while (1) {
		/* loop forever */
	}
}
