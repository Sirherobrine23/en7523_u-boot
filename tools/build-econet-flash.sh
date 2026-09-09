#!/bin/sh
# SPDX-License-Identifier: GPL-2.0+
set -eu
soc="$1"
case "$soc" in
 en751627) endian=-EB; emulation=elf32btsmip ;;
 en7528) endian=-EL; emulation=elf32ltsmip ;;
 *) echo "unsupported flash-boot SoC: $soc" >&2; exit 1 ;;
esac
: "${srctree:?}" "${objtree:?}" "${CROSS_COMPILE:?}" "${UBOOT_LOAD_ADDR:?}"
src="$srctree/arch/mips/mach-econet/flash"
build="$objtree/.econet-flash-$soc"
mkdir -p "$build"
cc="${CROSS_COMPILE}gcc"
ld="${CROSS_COMPILE}ld"
objcopy="${CROSS_COMPILE}objcopy"
flags="$endian -mabi=32 -mips32r2 -msoft-float -mno-abicalls -fno-pic -fno-pie -ffreestanding -fno-builtin -fno-stack-protector -Os -G0"
"$cc" $flags -c "$src/loader-start.S" -o "$build/loader-start.o"
"$cc" $flags -DUBOOT_LOAD_ADDR="$UBOOT_LOAD_ADDR" -c "$src/loader.c" -o "$build/loader.o"
"$cc" $flags -DECONET_STANDALONE_BOOT -c "$src/../early_sfc.c" -o "$build/flash.o"
"$ld" $endian -m "$emulation" -T "$src/loader.lds" -Map "$build/loader.map" \
 -o "$build/loader.elf" "$build/loader-start.o" "$build/loader.o" "$build/flash.o"
"$objcopy" -O binary "$build/loader.elf" "$build/loader.bin"
"$cc" $flags -c "$src/$soc/start.S" -o "$build/start.o"
"$cc" $flags -c "$src/$soc/stages.S" -o "$build/stages.o"
# Add raw stage sections to a fresh object; no legacy header before the loader.
"$objcopy" --add-section .loader="$build/loader.bin" \
 --set-section-flags .loader=alloc,load,readonly,data \
 --add-section .spram="$objtree/${soc}_ddr.bin" \
 --set-section-flags .spram=alloc,load,readonly,data \
 "$build/stages.o" "$build/payloads.o"
"$ld" $endian -m "$emulation" -T "$src/flash.lds" -Map "$build/flash.map" \
 -o "$build/flash.elf" "$build/start.o" "$build/payloads.o"
"$objcopy" -O binary "$build/flash.elf" "$build/flash.bin"
"${CROSS_COMPILE}nm" -n "$build/flash.elf" > "$build/flash.nm"
"${PYTHON3:-python3}" "$srctree/tools/econet_flash_image.py" \
 --soc "$soc" --stages "$build/flash.bin" --symbols "$build/flash.nm" \
 --uboot "$objtree/u-boot.img" --load "$UBOOT_LOAD_ADDR" \
 --output "$objtree/tcboot.bin"
