// SPDX-License-Identifier: GPL-2.0+

#include <fdt_support.h>
#include <init.h>
#include <dm/ofnode.h>
#include <asm/global_data.h>
#include <asm/system.h>
#include <linux/bitfield.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <soc/airoha/pkgids.h>

DECLARE_GLOBAL_DATA_PTR;

#define AN7552_SYS_GLOBAL_PARM		0x1fb00284
#define AN7552_DRAM_SIZE_MASK		GENMASK(29, 20)
#define AN7552_RESET_CONTROL		0x1fb00040

int print_cpuinfo(void)
{
	u32 hir = get_pkg();
	u32 pdidr = get_pdidr();
	u32 pkgid = get_pkgid();
	const char *soc_name = airoha_soc_name_from_regs(hir, pkgid, pdidr);

	if (pkgid != END_PACKAGE_ID) {
		printf("SoC:   Airoha %s\n", soc_name);
	} else {
		printf("SoC:   Airoha AN7552/AN7563\n");
	}

	return 0;
}

int dram_init(void)
{
	phys_size_t size;
	u32 value, units;

	/* The first-stage loader records capacity in units of 16 MiB. */
	value = readl((void __iomem *)AN7552_SYS_GLOBAL_PARM);
	units = FIELD_GET(AN7552_DRAM_SIZE_MASK, value);
	if (units > SZ_1G / SZ_16M)
		return -EINVAL;

	size = (phys_size_t)units * SZ_16M;
	if (!size) {
		ofnode node = ofnode_path("/memory@80000000");
		fdt_size_t dt_size;

		/* Older firmware may leave the field unset; use the board DT. */
		if (!ofnode_valid(node) ||
		    ofnode_get_addr_size(node, "reg", &dt_size) != CFG_SYS_SDRAM_BASE)
			return -EINVAL;
		size = dt_size;
	}

	/* This AArch32 port describes one contiguous bank below 4 GiB. */
	if (size < SZ_32M || size > SZ_1G)
		return -EINVAL;

	gd->ram_base = CFG_SYS_SDRAM_BASE;
	gd->ram_size = size;

	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = gd->ram_base;
	gd->bd->bi_dram[0].size = gd->ram_size;

	return 0;
}

#ifdef CONFIG_OF_SYSTEM_SETUP
int ft_system_setup(void *blob, struct bd_info *bd)
{
	u64 start[] = { gd->ram_base };
	u64 size[] = { gd->ram_size };

	return fdt_fixup_memory_banks(blob, start, size, ARRAY_SIZE(start));
}
#endif

void __noreturn reset_cpu(void)
{
	writel(BIT(31), (void __iomem *)AN7552_RESET_CONTROL);
	while (1)
		wfi();
}
