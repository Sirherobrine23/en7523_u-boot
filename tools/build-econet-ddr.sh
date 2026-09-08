#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu

soc="${1:-}"
srctree="${srctree:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}"
objtree="${objtree:-$srctree}"

case "$soc" in
 en751221)
  dir="$srctree/arch/mips/mach-econet/en751221/ddr"
  endian=-EB
  emulation=elf32btsmip
  cc_default=mips-linux-gnu-
  out="$objtree/en751221_ddr.bin"
  ;;
 en7528)
  dir="$srctree/arch/mips/mach-econet/en7528/ddr"
  endian=-EL
  emulation=elf32ltsmip
  cc_default=mipsel-linux-gnu-
  out="$objtree/en7528_ddr.bin"
  ;;
 en7580)
  dir="$srctree/arch/mips/mach-econet/en7580/ddr"
  endian=-EL
  emulation=elf32ltsmip
  cc_default=mipsel-linux-gnu-
  out="$objtree/en7580_ddr.bin"
  ;;
 *)
  echo "usage: $0 {en751221|en7528|en7580}" >&2
  exit 2
  ;;
esac

cross="${CROSS_COMPILE:-$cc_default}"
cc="${cross}gcc"
ld="${cross}ld"
objcopy="${cross}objcopy"
build="$objtree/.econet-ddr-$soc"
rm -rf "$build"
mkdir -p "$build"

cflags="$endian -mabi=32 -mips32r2 -msoft-float -mno-abicalls -fno-pic -ffreestanding -fno-builtin -Os -G0"
# An omitted section symbol silently turns a relocation into an absolute one.
if grep -nE '\.reloc.*,[[:space:]]*$' "$dir"/reconstructed/*.S >&2; then
 echo "error: DDR source contains unresolved section relocations" >&2
 exit 1
fi

objs=''
for src in "$dir"/reconstructed/*.S "$dir"/*.S; do
 [ -f "$src" ] || continue
 obj="$build/$(basename "${src%.S}").o"
 "$cc" $cflags -x assembler-with-cpp -c "$src" -o "$obj"
 objs="$objs $obj"
done

if [ -f "$dir/glue.c" ]; then
 "$cc" $cflags -fno-stack-protector -c "$dir/glue.c" -o "$build/glue.o"
 objs="$objs $build/glue.o"
fi

"$ld" "$endian" -m "$emulation" -T "$dir/ddr.lds" -Map "$build/ddr.map" -o "$build/ddr.elf" $objs
"$objcopy" -O binary "$build/ddr.elf" "$out.tmp"
mv -f "$out.tmp" "$out"

printf '%s\n' "$out"
