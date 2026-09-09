.. SPDX-License-Identifier: GPL-2.0+

EN751627 and EN7528 flash entry
==============================

``CONFIG_ECONET_FLASH_BOOT`` builds ``tcboot.bin`` for EN751627 big-endian
and EN7528 little-endian. The first 128 KiB are an entry point, DDR training
and a small loader. The uncompressed U-Boot uImage starts at offset 0x20000.
The complete file is padded to 1 MiB. ``u-boot.bin`` remains available for
chainloading, and ``u-boot.img`` is the payload placed at 0x20000.

These targets no longer use the earlier TPL/SPL/Binman cold-boot draft.
The older paths for EN751221 and EN7580 are outside this migration and remain
separate; ``tools/econet_tcboot_image.py`` belongs to those older paths.

Execution
---------

The SDK reset code copies ``move_data`` into GDMP SRAM at 0xbfa40300.
That reader loads ``boot2`` into SRAM. ``boot2`` initializes CPS, caches and
secondary contexts, copies calibration into FE SRAM and calls 0x9fa30000.
Calibration returns through the address in 0xbfb00280. ``boot2`` then copies
the raw loader into DRAM at 0x80000000 and jumps there.

The loader initializes the SFC, reads the legacy header at 0x20000, validates
its CRC, type, load address and size, then reads the uncompressed payload.
It writes through KSEG1 after clearing stale destination cache lines,
checks the data CRC, invalidates the destination I-cache and jumps to the
configured U-Boot entry. It does not decompress, initialize Ethernet, run a
command interpreter or load the old TCBoot main image.

The default U-Boot load/entry address is 0x81000000. The loader and its stack
occupy low RAM below 0x80080000. The image builder requires the payload to fit
in the minimum 32 MiB DRAM window and to stay outside the loader's RAM.

Flash layout
------------

==================== =====================================================
Offset               Contents
==================== =====================================================
0x000000             SDK reset entry and native-endian TCBoot descriptor
before 0x000800      Complete reset entry and move_data
following stages     boot2, raw loader, DDR calibration (linker-selected)
0x00ff00..0x00ffff    256-byte manufacturing area
0x00ffb0             Native word 0x50414745 for NAND page-size detection
0x01fff8             Additional EGAP marker on EN7528 LE
0x01fffc             TCBoot CRC over bytes [0, 0x1fffc)
0x020000             U-Boot legacy header, followed by its raw payload
0x100000             End of the combined image
==================== =====================================================

The linker rejects move_data reaching 0x800 and stages reaching 0xff00.
There is no compressed bootram at 0x10000, no second 256 KiB CRC and no U-Boot
payload at 0x60000. Stage lengths come from the linked symbols, not fixed
DDR padding lengths. The loader is described in the legacy ``lzma`` fields,
but those fields now point to executable code without any image header.

TCBoot CRC uses polynomial 0xedb88320, seed 0xffffffff, and no final XOR.
In Python it is ``zlib.crc32(data) ^ 0xffffffff``. The word is stored in the
SoC's endian order. uImage header/data CRCs use the standard final XOR instead.
The page marker is ``PAGE`` for EN751627 BE and ``EGAP`` for EN7528 LE.

Build
-----

Use a fresh output directory when migrating from the earlier TPL/SPL configs::

    make O=out/en751627 en751627_reference_defconfig
    make O=out/en751627 CROSS_COMPILE=mips-linux-gnu- -j4

    make O=out/en7528 en7528_reference_defconfig
    make O=out/en7528 CROSS_COMPILE=mipsel-linux-gnu- -j4

For the XC220-G3V board, use ``en7528_tplink_xc220_g3v_defconfig``.
``make O=... CROSS_COMPILE=... tcboot.bin`` also builds the required U-Boot
payload and both early images. GNU MIPS binutils/GCC and Python 3 are required;
this path has no Binman or LZMA host-tool dependency.

Each output directory contains ``tcboot.bin``, ``u-boot.bin``, ``u-boot.img``,
the calibration binary, and ``.econet-flash-<soc>/`` with intermediate ELFs,
raw stage images, linker maps and a symbol listing.

Manufacturing area
------------------

The default manufacturing area is zero-filled except for the page marker.
To provide board data, pass an exact 256-byte area to the finalizer. It always
regenerates the mandatory page marker and CRC::

    python3 tools/econet_flash_image.py --soc en7528 \
        --stages out/en7528/.econet-flash-en7528/flash.bin \
        --symbols out/en7528/.econet-flash-en7528/flash.nm \
        --uboot out/en7528/u-boot.img --load 0x81000000 \
        --minfo board-minfo.bin --output out/en7528/tcboot-board.bin

Do not copy an entire 256 KiB stock layout over the assembled image.

Calibration inputs
------------------

EN751627 uses the previously missing 0xd050-byte LOAD segment from the supplied
SDK ELF. Its entry is 0x9fa30280. The packer places a jump in the initial eight
zero bytes, so boot2 can enter at 0x9fa30000 without changing the remaining
calibration code. The original on-disk DDR binary is preserved.

EN7528 keeps the reconstructed calibration sources, adding explicit SRAM
stack setup and cold UART initialization. Imported reset/move_data/boot2
provenance and SHA256 values are recorded in ``arch/mips/mach-econet/flash/README``.

Validation and remaining hardware work
--------------------------------------

Run host image checks with::

    python3 -m unittest discover -s tools -p test_econet_flash_image.py -v

SPI-NOR uses the SFC manual reader only after execution has moved to SRAM or
DRAM. SPI-NAND uses physical page reads and the page shift detected by the
SDK move_data stage (2, 4 or 8 KiB). It does not implement BMT/BBT remapping,
bad-block skipping, or controller-ECC decoding for the U-Boot payload.
The image must occupy matching contiguous physical data offsets on NAND;
a writer which silently skips blocks changes this layout. Payload CRC failure
stops execution, but is not ECC correction. NAND OOB/ECC programming remains
board-specific and is not generated by this packer.

A successful cross-build verifies layout and linkage, not DRAM training or
BootROM acceptance. The exact board/DRAM and flash programming format still
need a cold-boot test. Expected progress after calibration is::

    EcoNet flash loader
    Starting U-Boot
    U-Boot ...

This is the unsigned flash path. Existing partitions must reserve the full
1 MiB image region; secure-header signing is outside this implementation.
