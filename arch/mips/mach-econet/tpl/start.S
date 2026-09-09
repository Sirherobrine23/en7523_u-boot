/* SPDX-License-Identifier: GPL-2.0+ */
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <mach/boot.h>

	.set noreorder

ENTRY(_start)
	/* Match the vendor TCBoot reset vector. */
	j	reset
	 nop

	/*
	 * Common EcoNet/Airoha TCBoot descriptor. Raw bytes at +0x0c must be
	 * "6578" for both endian modes.  The vendor 128 KiB EN751221 layout
	 * treats the image at 0x10000 as bootram, while the newer 256 KiB layout
	 * treats the executable loader at 0x10000 as lzma.img.  Binman creates
	 * the fixed placement and tools/econet_tcboot_image.py patches the actual
	 * end address and CRC words after the image is assembled.
	 */
	.org	0x08
	.word	ECONET_TCBOOT_HEADER_LEN
	.ascii	"6578"
#ifdef CONFIG_TARGET_EN751221
	.word	0	/* lzma_flash_start_addr */
	.word	0	/* lzma_flash_end_addr */
	.word	ECONET_SPL_IMAGE_OFFSET	/* bootram_flash_start_addr */
	.word	0	/* bootram_flash_end_addr, finalized after Binman */
#else
	.word	ECONET_SPL_IMAGE_OFFSET	/* lzma_flash_start_addr */
	.word	0	/* lzma_flash_end_addr, finalized after Binman */
	.word	0	/* bootram_flash_start_addr */
	.word	0	/* bootram_flash_end_addr */
#endif
	.word	0
	.word	ECONET_TCBOOT_CHIP_FLASH_INFO
	.word	ECONET_TCBOOT_WORD28
	.word	ECONET_TCBOOT_WORD2C
	.word	ECONET_DDR_BLOB_ADDR
	.word	CONFIG_SPL_TEXT_BASE
	.word	0	/* verify_start_addr: unsigned boot */
	.word	0	/* verify_end_addr: unsigned boot */
	.word	0	/* efuse_clk_width */
	.word	0

#ifdef CONFIG_TARGET_EN751221
	/* The older EN751221 reset image enters at +0x48. */
	.org	0x48
#else
	.word	0	/* move_data_int start */
	.word	0	/* move_data_int end */
	.word	0	/* boot2 start */
	.word	0	/* boot2 end */
	.word	ECONET_DDR_BLOB_OFFSET	/* spram_flash_start_addr */
	.word	ECONET_DDR_BLOB_OFFSET + ECONET_DDR_BLOB_SIZE
	.org	0x60
#endif
reset:
#ifndef CONFIG_MIPS_SKIP_CP0_COUNT_CLEAR
	mtc0	zero, CP0_COUNT
#endif
	/* Disable interrupts and force KSEG0 uncached before DRAM exists. */
	mfc0	t0, CP0_STATUS
	li	t1, ~(ST0_IE | ST0_KSU)
	and	t0, t0, t1
	li	t1, ST0_BEV | ST0_ERL
	or	t0, t0, t1
	mtc0	t0, CP0_STATUS
	mtc0	zero, CP0_CAUSE
	mtc0	zero, CP0_COMPARE

	mfc0	t0, CP0_CONFIG
	li	t1, ~7
	and	t0, t0, t1
	ori	t0, t0, 2
	mtc0	t0, CP0_CONFIG
	ehb

	/* Arbiter and static-memory mappings shared by EN751221 and EN7580. */
	li	t0, ECONET_SYSCTL_BASE
	li	t1, 0x80071f1e
	sw	t1, 0x20(t0)
	li	t1, 0x00071f1f
	sw	t1, 0x24(t0)
	li	t1, 0x80050000
	sw	t1, 0x34(t0)

	li	t0, ECONET_SMC_BASE
	li	t1, 0x102d1040
	sw	t1, 0x00(t0)
	li	t1, 0x200028d0
	sw	t1, 0x14(t0)

	/* Disable the legacy interrupt controller. */
	li	t0, ECONET_INTC_BASE
	sw	zero, 0x00(t0)
	sw	zero, 0x04(t0)

	/* Enable PBUS access to the FE SRAM window. */
	li	t0, ECONET_FE_SRAM_SEL
#ifdef CONFIG_TARGET_EN7528
	/*
	 * Vendor start.o clears 0xbfb00954 before enabling the FE SRAM
	 * selector at 0xbfb00958.  Do this before touching 0x9fa3xxxx.
	 */
	sw	zero, -4(t0)
	sync
#endif
	lw	t1, 0(t0)
	ori	t1, t1, 1
	sw	t1, 0(t0)
	sync

	li	sp, ECONET_TPL_STACK_ADDR
	li	fp, ECONET_TPL_STACK_ADDR

	la	t9, tpl_main
	jalr	t9
	 nop

1:	b	1b
	 nop
END(_start)

LEAF(econet_run_ddr_blob)
	addiu	sp, sp, -48
	sw	ra, 0(sp)
	sw	gp, 4(sp)
	sw	fp, 8(sp)
	sw	s0, 12(sp)
	sw	s1, 16(sp)
	sw	s2, 20(sp)
	sw	s3, 24(sp)
	sw	s4, 28(sp)
	sw	s5, 32(sp)
	sw	s6, 36(sp)
	sw	s7, 40(sp)
	li	t0, ECONET_TPL_SAVED_SP
	sw	sp, 0(t0)

	la	t1, 1f
	li	t0, ECONET_SCREG_WR0
	sw	t1, 0(t0)
	sync

	li	t9, ECONET_DDR_BLOB_ENTRY
	jr	t9
	 nop
1:
	li	t0, ECONET_SCREG_WR0
	sw	zero, 0(t0)
	li	t0, ECONET_TPL_SAVED_SP
	lw	sp, 0(t0)
	lw	ra, 0(sp)
	lw	gp, 4(sp)
	lw	fp, 8(sp)
	lw	s0, 12(sp)
	lw	s1, 16(sp)
	lw	s2, 20(sp)
	lw	s3, 24(sp)
	lw	s4, 28(sp)
	lw	s5, 32(sp)
	lw	s6, 36(sp)
	lw	s7, 40(sp)
	jr	ra
	 addiu	sp, sp, 48
END(econet_run_ddr_blob)
