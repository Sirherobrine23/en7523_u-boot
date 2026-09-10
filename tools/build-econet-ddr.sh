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
  # Preserve the vendor link order.  In particular start_spram/UART/timer
  # precede the calibration objects in the original EN7512 spram image.
  asm_order="head setup main init time string dramc dramc_dq_dqs_cal dramc_dle_cal dramc_dqs_gw_cal en7512_dramc_init spram"
  ;;
 en751627)
  dir="$srctree/arch/mips/mach-econet/en751627/ddr"
  endian=-EB
  emulation=elf32btsmip
  cc_default=mips-linux-gnu-
  out="$objtree/en751627_ddr.bin"
  prebuilt="$dir/en751627_ddr.bin"
  ;;
 en7528)
  dir="$srctree/arch/mips/mach-econet/en7528/ddr"
  endian=-EL
  emulation=elf32ltsmip
  cc_default=mipsel-linux-gnu-
  out="$objtree/en7528_ddr.bin"
  readable="$dir/readable"
  # The EN7528 V1.8 DDR stage is now fully readable C.  The complete path
  # (system/eFuse setup, MEMPLL, DRAMC init and all training stages) has
  # completed a standalone SRAM training run on EN7528HU/QFP DDR3.
  no_reconstructed=1
  ddr_variant="readable V1.8 full-C"
  ;;
 en7580)
  dir="$srctree/arch/mips/mach-econet/en7580/ddr"
  endian=-EL
  emulation=elf32ltsmip
  cc_default=mipsel-linux-gnu-
  out="$objtree/en7580_ddr.bin"
  ;;
 *)
  echo "usage: $0 {en751221|en751627|en7528|en7580}" >&2
  exit 2
  ;;
esac

if [ -n "${prebuilt:-}" ]; then
 expected_size=$((0xd050))
 actual_size=$(wc -c < "$prebuilt")
 if [ "$actual_size" -ne "$expected_size" ]; then
  echo "error: EN751627 DDR payload has size $actual_size, expected $expected_size" >&2
  exit 1
 fi
 cp "$prebuilt" "$out.tmp"
 mv -f "$out.tmp" "$out"
 printf '%s\n' "$out"
 exit 0
fi

cross="${CROSS_COMPILE:-$cc_default}"
cc="${cross}gcc"
ld="${cross}ld"
objcopy="${cross}objcopy"
build="$objtree/.econet-ddr-$soc"
rm -rf "$build"
mkdir -p "$build"

cflags="$endian -mabi=32 -mips32r2 -msoft-float -mno-abicalls -fno-pic -ffreestanding -fno-builtin -Os -G0"
# An omitted section symbol silently turns a relocation into an absolute one.
if [ -z "${no_reconstructed:-}" ]; then
 for src in "$dir"/reconstructed/*.S; do
  [ -f "$src" ] || continue
  if grep -nE '\.reloc.*,[[:space:]]*$' "$src" >&2; then
   echo "error: DDR source contains unresolved section relocations" >&2
   exit 1
  fi
 done
fi

objs=''

compile_asm()
{
 src="$1"
 [ -f "$src" ] || return 0
 obj="$build/$(basename "${src%.S}").o"
 "$cc" $cflags -x assembler-with-cpp -c "$src" -o "$obj"
 objs="$objs $obj"
}

compile_c()
{
 src="$1"
 [ -f "$src" ] || return 0
 obj="$build/$(basename "${src%.c}").o"
 "$cc" $cflags -fomit-frame-pointer -fno-stack-protector \
  ${readable:+-I"$readable"} -c "$src" -o "$obj"
 objs="$objs $obj"
}

if [ -z "${no_reconstructed:-}" ]; then
 if [ -n "${asm_order:-}" ]; then
  for name in $asm_order; do
   compile_asm "$dir/reconstructed/$name.S"
  done
 else
  for src in "$dir"/reconstructed/*.S; do
   [ -f "$src" ] || continue
   name=$(basename "${src%.S}")
   case " ${skip_reconstructed:-} " in
    *" $name "*) continue ;;
   esac
   compile_asm "$src"
  done
 fi
fi

for src in "$dir"/*.S; do
 compile_asm "$src"
done

if [ -n "${readable:-}" ]; then
 for src in "$readable"/*.c; do
  compile_c "$src"
 done
fi

if [ -f "$dir/glue.c" ]; then
 compile_c "$dir/glue.c"
fi

"$ld" "$endian" -m "$emulation" -T "$dir/ddr.lds" -Map "$build/ddr.map" -o "$build/ddr.elf" $objs
"$objcopy" -O binary "$build/ddr.elf" "$out.tmp"
mv -f "$out.tmp" "$out"

if [ -n "${ddr_variant:-}" ]; then
 printf 'EN7528 DDR: %s (%s bytes)\n' "$ddr_variant" "$(wc -c < "$out")" >&2
fi
printf '%s\n' "$out"
