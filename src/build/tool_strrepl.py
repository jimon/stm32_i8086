# string replace in files
import sys
if len(sys.argv) < 3:
	print("wrong invocation, usage : python tool_strrepl.py $in $out $replace_from_1 $replace_to_1 $replace_from_2 $replace_to_2 ...")
	exit(1)

with open(sys.argv[1], "r") as infile:
	fw = infile.read()
	with open(sys.argv[2], "w") as outfile:
		for i in range(0, int((len(sys.argv) - 3) / 2)):
			fw = fw.replace(sys.argv[i * 2 + 3], sys.argv[i * 2 + 4])
		outfile.write(fw)

