.. SPDX-License-Identifier: GPL-2.0+

EcoNet MIPS cold-boot image
===========================

The EN751221, EN751627, EN7528-reference and EN7580 targets build an unsigned
TPL/SPL cold-boot replacement image.  The replacement region is capped at
1 MiB.  Its beginning follows the normal (non-secure) TCBoot flash layout used
by the EcoNet/Airoha MIPS internal ROM, while U-Boot proper is kept outside the
legacy TCBoot span and is loaded directly by SPL.

This does not implement the ``ECNT`` secure header. A part with secure boot
fused still requires a signature accepted by its provisioned key.

TCBoot layout
-------------

The vendor ``tHeader.h`` header is emitted at offset zero. Bytes at +0x0c are
the literal ASCII string ``6578`` in both endian modes.  The final image helper
patches the stage-end fields and complemented CRC32 words after Binman has
assembled the image.

EN751221 follows the older 128 KiB layout observed on the Nokia G-240G-E::

    0x000000  TPL / TCBoot descriptor
    0x008000  DDR-training payload
    0x00ff00  256-byte manufacturing block (preserve from stock if required)
    0x010000  SPL legacy image (bootram-equivalent stage)
    0x01fffc  complemented CRC32 over 0x00000..0x1fffb
    0x020000  end of legacy TCBoot span
    0x050000  U-Boot proper
    0x100000  end of replacement image

EN751627 follows the normal 256 KiB big-endian layout::

    0x000000  TPL / TCBoot descriptor
    0x002000  DDR-training payload (spram-equivalent stage)
    0x00ffb0  EGAP constant-data flash magic
    0x010000  SPL legacy image (lzma-loader-equivalent stage)
    0x01fffc  complemented CRC32 #1
    0x03fdfc  complemented CRC32 #2
    0x03fe00  512-byte manufacturing/RFU area
    0x03feb0  EGAP copy in manufacturing information
    0x040000  end of legacy TCBoot span
    0x060000  U-Boot proper
    0x100000  end of replacement image

EN7528 and EN7580 use the same 256 KiB layout, with the little-endian internal
ROM NAND ECC marker at 0x1fff8::

    0x000000  TPL / TCBoot descriptor
    0x004000  DDR-training payload
    0x00ffb0  EGAP constant-data flash magic
    0x010000  SPL legacy image
    0x01fff8  EGAP little-endian NAND ECC flash magic
    0x01fffc  complemented CRC32 #1
    0x03fdfc  complemented CRC32 #2
    0x03fe00  512-byte manufacturing/RFU area
    0x03feb0  EGAP copy in manufacturing information
    0x040000  end of legacy TCBoot span
    0x060000  U-Boot proper
    0x100000  end of replacement image

The comments in the vendor ``header.c`` describe the 256 KiB manufacturing
block as starting at 0x3fdfc and place another CRC at 0x3fffc.  The actual
``mic.c`` mapping and stock EN7528/EN7580 dumps instead show the second CRC at
0x3fdfc and manufacturing information from 0x3fe00.  The finalizer follows the
actual generated images.

Header semantics
----------------

For EN751221 the SPL at 0x10000 is described using
``bootram_flash_start_addr``/``bootram_flash_end_addr``, matching the old
128 KiB arrangement.  The known Nokia image leaves ``tcboot_len`` zero, so the
replacement retains that value.

For EN751627, EN7528 and EN7580 the SPL at 0x10000 is described using
``lzma_flash_start_addr``/``lzma_flash_end_addr``.  The DDR payload is described
by ``spram_flash_start_addr``/``spram_flash_end_addr`` and
``spram_exe_addr``.  The bootram interval is zero because U-Boot proper is not
stored inside the 256 KiB TCBoot image; SPL loads it directly from the larger
replacement region.

The verify interval is zero for unsigned boot.  EN751221 retains the older
+0x48 reset entry, while EN751627, EN7528 and EN7580 enter at +0x60.

CRC generation
--------------

``tools/econet_tcboot_image.py`` runs automatically after Binman.  It reads the
actual SPL legacy-image payload size, fills the TCBoot end field, and writes the
vendor-style complemented CRC32 in native endian::

    crc1 = ~crc32(image[0x000000:0x01fffc])

For the 256 KiB layout the second CRC includes the finalized first CRC::

    crc2 = ~crc32(image[0x000000:0x03fdfc])

This matches the CRC words observed in stock EN751221 and EN7528 bootloaders.

Build inputs
------------

TPL configurations build their DDR payload before Binman. Out-of-tree builds
use the same ``CROSS_COMPILE`` prefix and put the payload in the output
directory::

    make O=out/en751221 en7512_reference_defconfig
    make O=out/en751221 CROSS_COMPILE=mips-linux-gnu- -j4

    make O=out/en751627 en751627_reference_defconfig
    make O=out/en751627 CROSS_COMPILE=mips-linux-gnu- -j4

    make O=out/en7528 en7528_reference_defconfig
    make O=out/en7528 CROSS_COMPILE=mipsel-linux-gnu- -j4

    make O=out/en7580 en7580_xgz030_defconfig
    make O=out/en7580 CROSS_COMPILE=mipsel-linux-gnu- -j4

The EN751627 payload is the 0xd050-byte LOAD image extracted from the
GPL-released big-endian ``boot.out``. It loads at 0x9fa30000 and enters at
0x9fa30280. EN751221, EN7528 and EN7580 use the reconstructed DDR sources in
their respective ``ddr`` directories.

Manufacturing information
-------------------------

The reference images leave board-specific manufacturing bytes erased except
for the flash magic required by the newer boot ROMs.  Preserve stock data when
the board depends on it.

For EN751221 the existing helper copies the 0xff00..0xffff manufacturing block
and recomputes the first CRC::

    tools/en7512_image.py \
        --image u-boot-en7512.bin \
        --stock stock-tcboot.bin \
        --output u-boot-en7512-board.bin

The generic finalizer can also copy the appropriate manufacturing region for
any supported SoC with ``--stock`` and ``--output``.

Board validation still required
-------------------------------

* The early SFC reader assumes 2 KiB SPI-NAND pages and does not implement
  BMT/BBT bad-block relocation. Verify the actual NAND geometry before flash.
* Existing vendor bootloader partitions can be 256 KiB. The full replacement
  image is 1 MiB, so reserve a 1 MiB region before writing it.
* Validate the DDR payload against the exact package, board and DRAM devices.
* Secure-boot parts are not supported by these unsigned images.

Cold-boot testing should be recoverable (external programmer or another known
recovery path); successful chainloading does not prove the internal-ROM path.
