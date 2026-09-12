#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
"""Finalize the EN751221 chainloader binary.

The BootROM validates its XMODEM download with an 8-bit checksum, which lets
corruption through, so the last loaded word holds a CRC32 of everything before
it and the chainloader verifies itself in DRAM before doing anything else.

The image is then padded to a multiple of 128 bytes so the BootROM does not
invent padding of its own on top of .bss.
"""
import binascii
import struct
import sys

XMODEM_BLOCK = 128

path = sys.argv[1]
data = bytearray(open(path, 'rb').read())
if len(data) < 8:
    sys.exit('chainloader image too short')

crc = binascii.crc32(bytes(data[:-4])) & 0xffffffff
data[-4:] = struct.pack('>I', crc)          # MIPS big-endian
pad = (-len(data)) % XMODEM_BLOCK
data += b'\x00' * pad
open(path, 'wb').write(bytes(data))
print('  CHAIN   %s: len=0x%x crc32=0x%08x size=%d'
      % (path, len(data) - pad - 4, crc, len(data)))
