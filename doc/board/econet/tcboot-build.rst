.. SPDX-License-Identifier: GPL-2.0+

EcoNet MIPS DDR and TCBoot image build
====================================

The EN751221 and EN7580 combined images are experimental reset-path images.
A successful build does not validate internal-ROM NAND boot compatibility.
The EN7528 reference and XC220 configurations still build chainloaded U-Boot
proper; building their DDR payload does not add a TPL/SPL boot path.

Build inputs
------------

TPL configurations build their DDR payload before Binman. The payload is a
mandatory local blob, not an optional external firmware file. Out-of-tree
builds use the same CROSS_COMPILE prefix and put the payload in the output
directory. The standalone DDR helper also explicitly selects the target
endianness, O32 ABI and soft-float code generation::

    make O=out/en751221 en7512_reference_defconfig
    make O=out/en751221 CROSS_COMPILE=mips-linux-gnu- -j4

    make O=out/en7580 en7580_xgz030_defconfig
    make O=out/en7580 CROSS_COMPILE=mipsel-linux-gnu- -j4

    make O=out/en7528 en7528_reference_defconfig
    make O=out/en7528 CROSS_COMPILE=mipsel-linux-gnu- -j4

EN7528 DDR can separately be built with::

    objtree="$PWD/out/en7528" CROSS_COMPILE=mipsel-linux-gnu- \
        tools/build-econet-ddr.sh en7528

The helper rejects unresolved section relocations. EN7528 and EN7580 have a
MIPS32 entry shim at the start of the payload. It clears BSS and passes the
SDK's NEED_DDR_CALIBRATION argument. GNU assembler .insn annotations preserve
MIPS16 function metadata, allowing the linker to encode interworking calls
and function pointers correctly. Linker assertions reserve the upper 4 KiB
of FE SRAM for the TPL stack.

The EN751221 vendor head.o has a vector prefix before its start symbol. It
must be first in .text; ENTRY(start) alone does not order input sections.

Provenance
----------

* EN751221: cjdelisle/EN751221-Linux26,
  tclinux_phoenix/bootrom/ddr_cal_en7512/output/*.o.
* EN7528: TP-Link XC220-G3_BR_v1_GPL, SDK en7528_7.3.275.1,
  release_bsp/*/bootrom/ddr_cal_en7512/output/*.o, and bootrom/spram_ext/system.o.
* EN7580: TP-Link XGZ030_v1_GPL, SDK en7580,
  release_bsp/UNION_EN7580_SFU_KERNEL_4_4_demo/bootrom/unopen_img/
  ram_init/output/*.o and unopen_img/pkgId.o.

The assembly retains vendor instructions. It is a reconstruction of
relocatable objects, not a native DRAM driver. Section-symbol relocations
must use their actual section base; an empty .reloc expression is not an
acceptable substitute.

TCBoot versus the new images
---------------------------

The Archer VR600 v3 and VX830v SDK bootrom/Makefile builds calibration,
bootram, the LZMA loader and move_data/boot2 stages. Its linker script embeds
these sections, mic inserts manufacturing data, and header fills stage
boundaries and flash-related fields. Its byteswap utility also generates a
separate swapped artifact. These are build/format operations, not an
instruction to byte-swap the new raw U-Boot image.

The existing EN751221 TPL descriptor is not the vendor descriptor ABI:
header.c uses 0x10/0x14 for the LZMA interval and 0x18/0x1c for the bootram
interval, whereas the port places DDR and SPL/U-Boot offsets there. Retaining
the magic at 0x0c does not make this compatible with the internal ROM.

Board validation still required
-------------------------------

* Establish the actual reset path: flash execution versus internal-ROM stage
  loading, including secondary CPU/CPS state.
* Validate NAND geometry, boot ECC/OOB layout and bad-block handling. The
  early reader still assumes 2 KiB pages and does not implement BMT/BBT or
  controller-ECC decoding.
* Preserve manufacturing information and establish partition sizes. The
  combined images are larger than some stock bootloader partitions. The
  EN751221 merge helper only covers its existing 0xff00..0xffff interval.
* Validate DDR configuration for the actual package, board and memory chips,
  and cold-boot initialization of switch/PHY and other peripherals.
* For EN7528, implement a board-specific reset/TPL/SPL path before replacing
  TCBoot; neither existing defconfig currently provides that path.

Do not treat a chainloading result as validation of a cold flash boot.
