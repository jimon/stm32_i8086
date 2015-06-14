# stm32 binary sign
import hashlib
import struct
import sys
if len(sys.argv) < 7:
	print("wrong invocation, usage : python toolsign.py $in $out $FLASH_SIGNOFFSET $stlink $FLASH_BASE $FLASH_OFFSET")
	exit(1)
prefixlen = int(sys.argv[3], 0)

with open(sys.argv[1], "rb") as infile:
	fw = infile.read()
	with open(sys.argv[2], "wb") as outfile:
		if prefixlen > 0 :
			ha = hashlib.md5(fw).digest()
			le = len(fw)
			prefix = struct.pack("<I16s", le, ha).ljust(prefixlen, b"\0")
			outfile.write(prefix)
		outfile.write(fw)

addr = int(sys.argv[5], 16) + int(sys.argv[6], 16)

import subprocess
res = subprocess.call([sys.argv[4], "-c", "SWD", "-p", sys.argv[2], hex(addr), "-Rst", "-Run"])

if res != 0:
	import os
	os.remove(sys.argv[2]) # remove file so build system will rerun this command next time
	raise Exception("ST Link failed", "see stdout")
