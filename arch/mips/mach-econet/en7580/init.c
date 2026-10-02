// SPDX-License-Identifier: GPL-2.0+

#include <fdt_support.h>
#include <init.h>
#include <sysreset.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <mach/en7580.h>
#include <soc/airoha/pkgids.h>

DECLARE_GLOBAL_DATA_PTR;

#define EN7580_RESET_CONTROL		0xbfb00040UL
#define EN7580_LOWMEM_MAX_MB		448U
#define NP_SCU_BASE			((void __iomem *)CKSEG1ADDR(0x1fb00000))

int dram_init(void)
{
	u32 value, size_mb;

	value = readl((void __iomem *)EN7580_SYS_GLOBAL_PARM);
	size_mb = (value & EN7580_DRAM_SIZE_MASK) >> EN7580_DRAM_SIZE_SHIFT;

	debug("EN7580 DRAM: global-param=%08x size=%u MiB\n", value, size_mb);
	if (size_mb < 32 || size_mb > 2048) {
		printf("Invalid EN7580 calibrated DRAM size: %u MiB\n", size_mb);
		return -EINVAL;
	}

	/*
	 * EN7580 has a hole in the direct-mapped KSEG0/KSEG1 window above
	 * 448 MiB. The 0x1c000000..0x1fffffff physical range contains SoC
	 * MMIO, so relocating U-Boot near the top of a 512 MiB linear bank
	 * would place executable code over peripheral registers. Vendor code
	 * exposes memory above 448 MiB through a separate highmem/TLB mapping.
	 * Keep U-Boot proper in directly addressable low memory for now.
	 */
	gd->ram_size = size_mb * SZ_1M;
	if (size_mb > EN7580_LOWMEM_MAX_MB)
		gd->ram_size = EN7580_LOWMEM_MAX_MB * SZ_1M;

	return 0;
}

int dram_init_banksize(void)
{
	u32 value, size_mb;

	value = readl((void __iomem *)EN7580_SYS_GLOBAL_PARM);
	size_mb = (value & EN7580_DRAM_SIZE_MASK) >> EN7580_DRAM_SIZE_SHIFT;
	if (size_mb < 32 || size_mb > 2048) {
		printf("Invalid EN7580 calibrated DRAM size: %u MiB\n", size_mb);
		return -EINVAL;
	}

	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = size_mb * SZ_1M;

	gd->bd->bi_dram[1].start = 0;
	gd->bd->bi_dram[1].size = 0;

	if (size_mb > EN7580_LOWMEM_MAX_MB) {
		gd->bd->bi_dram[0].start = gd->ram_base;
		gd->bd->bi_dram[0].size = EN7580_LOWMEM_MAX_MB * SZ_1M;

		gd->bd->bi_dram[1].start = 0x9c000000;
		gd->bd->bi_dram[1].size = (size_mb - EN7580_LOWMEM_MAX_MB) * SZ_1M;
	}

	return 0;
}

#ifdef CONFIG_OF_SYSTEM_SETUP
int ft_system_setup(void *blob, struct bd_info *bd)
{
	u64 start[1] = { bd->bi_dram[0].start };
	u64 size[1] = { bd->bi_dram[0].size };

	return fdt_fixup_memory_banks(blob, start, size, 1);
}
#endif

void _machine_restart(void)
{
	writel(0x80000000, (void __iomem *)EN7580_RESET_CONTROL);
	for (;;)
		;
}
