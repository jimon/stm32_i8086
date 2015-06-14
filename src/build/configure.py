# toolchain configuration
arm_gcc				= "arm-none-eabi-gcc"
arm_as				= "arm-none-eabi-as"
arm_objcopy			= "arm-none-eabi-objcopy"
stm32cubef4_drivers	= "C:/work/stm32/STM32Cube_FW_F4_V1.4.0/Drivers"
stlink_cli			= "C:/Program Files (x86)/STMicroelectronics/STM32 ST-LINK Utility/ST-LINK Utility/ST-LINK_CLI.exe"
build_folder		= "build_temp"

# flash configuration
flash_base			= "0x08000000"
flash_offset		= "0"
flash_signoffset	= "0"
hal					= "STM32F407xx"

# stm32cubef4 features configuration
features = {
	"Adafruit_Shield":						False,
	"ampire480272":							False,
	"ampire640480":							False,
	"Common":								False,
	"cs43l22":								False,
	"ili9325":								False,
	"ili9341":								False,
	"l3gd20":								False,
	"lis302dl":								False,
	"lis3dsh":								False,
	"lsm303dlhc":							False,
	"ov2640":								False,
	"st7735":								False,
	"stmpe1600":							False,
	"stmpe811":								False,
	"ts3510":								False,
	"wm8994":								False,
	"STM324x9I_EVAL":						False,
	"STM324xG_EVAL":						False,
	"STM32F4-Discovery":					True,
	"stm32f4_discovery_audio.c":			False,
	"stm32f4_discovery_accelerometer.c":	False,
	"STM32F401-Discovery":					False,
	"STM32F429I-Discovery":					False,
	"STM32F4xx-Nucleo":						False,
	"STM32F4xx/Source/Templates":			False,
	"CMSIS/Documentation":					False,
	"CMSIS/DSP_Lib":						False,
	"CMSIS/RTOS":							False,
	"CMSIS/Lib":							False,
}

# helper functions to get list of files
import os

def excluded(path):
	for name, val in features.items():
		name = os.path.normpath(name)
		if name in path and not val:
			return True
	return False

def any(files, with_ext):
	for file in files:
		if file.endswith(with_ext):
			return True
	return False

def anyitem(iterable):
	for i in iterable:
		return i
	return None

def now_known(ext):
	return ext not in [".h", ".c", ".cxx", ".cpp", ".in", ".s"]

def temp_file(name, ext = None):
	if ext is None:
		return os.path.normpath(os.path.abspath(os.path.join(build_folder, os.path.basename(name))))
	else:
		return os.path.normpath(os.path.abspath(os.path.join(build_folder, os.path.splitext(os.path.basename(name))[0] + ext)))

def tool_file(name):
	return os.path.normpath(os.path.abspath(os.path.join(os.path.dirname(__file__), name)))

def shellquote(s):
	return "\"" + s.replace("\"", "\\\"") + "\""

def find_files(path, include_dirs, sources):
	include_dirs.add(os.path.normpath(os.path.abspath(path)))
	for root, dir, files in os.walk(path):
		if excluded(root):
			continue
		if any(files, ".h") or any(files, ".hpp"):
			include_dirs.add(os.path.normpath(os.path.abspath(root)))
		for file in files:
			filename_abs = os.path.normpath(os.path.abspath(os.path.join(root, file)))
			ext = os.path.splitext(filename_abs)[1]
			if excluded(filename_abs) or now_known(ext):
				continue
			if ext not in sources:
				sources[ext] = set()
			sources[ext].add(filename_abs)

# figure out list of required files
include_dirs = set()
sources = {}
find_files(stm32cubef4_drivers, include_dirs, sources)
find_files(".", include_dirs, sources)

if ".in" not in sources:
	print("unable to find linker script template (_name_.ld.in), do not know how to link")
	exit(1)

linker_script_in = anyitem(sources[".in"])
linker_script = temp_file(os.path.basename(os.path.splitext(linker_script_in)[0]))

# generate build.ninja file
if not os.path.isdir(build_folder):
	os.mkdir(build_folder)
import ninja_syntax
wr = ninja_syntax.Writer(open(temp_file("build.ninja"), "w"))
wr.comment("general settings")
wr.variable("flash_base", flash_base)
wr.variable("flash_offset", flash_offset)
wr.variable("flash_signoffset", flash_signoffset)
wr.variable("hal", hal)
wr.variable("defines", "-D $hal=1 -DVECT_TAB_OFFSET=$flash_offset+$flash_signoffset")

wr.comment("locations")
wr.variable("bin_cc", arm_gcc)
wr.variable("bin_as", arm_as)
wr.variable("bin_cp", arm_objcopy)
wr.variable("bin_stlink", stlink_cli)
wr.variable("include_paths", "".join(["-I" + f + " " for f in include_dirs]))
wr.variable("linkerscript", linker_script)

wr.comment("compiler flags")
wr.variable("cpuflags", "-mcpu=cortex-m4 -mthumb -mlittle-endian -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb-interwork")
wr.variable("optflags", "-O3 -fdata-sections -ffunction-sections -fsingle-precision-constant")
wr.variable("debugflags", "-g")
wr.variable("cflags", "$cpuflags $debugflags $optflags -std=c99 $defines $include_paths")
wr.variable("ldflags", "$cpuflags $debugflags -O3 -T$linkerscript -Wl,--gc-sections -lm")
wr.variable("asflags", "$cpuflags $debugflags")

wr.comment("rules")
wr.rule("ldscript", "python " + tool_file("tool_strrepl.py") + " $in $out FLASH_BASE $flash_base FLASH_OFFSET $flash_offset FLASH_SIGNOFFSET $flash_signoffset")
wr.rule("cc", "$bin_cc $cflags -o $out -c $in")
wr.rule("ld", "$bin_cc $ldflags -o $out $in")
wr.rule("as", "$bin_as $asflags -o $out -c $in")
wr.rule("cp", "$bin_cp -O binary $in $out")
wr.rule("sign_and_deploy", "python " + tool_file("tool_sign_and_deploy.py") + " $in $out $flash_signoffset " + shellquote(stlink_cli) + " $flash_base $flash_offset")

wr.comment("build")

wr.build(linker_script, "ldscript", linker_script_in)

o_files = set()
for ext in [".c", ".cpp", ".cxx"]:
	if ext in sources:
		for file in sources[ext]:
			temp = temp_file(file, ".o")
			o_files.add(temp)
			wr.build(temp, "cc", file)

for ext in [".s"]:
	if ext in sources:
		for file in sources[ext]:
			temp = temp_file(file, ".o")
			o_files.add(temp)
			wr.build(temp, "as", file)

main_elf		= temp_file("main.elf")
main_bin_ns		= temp_file("main.bin_ns")
main_bin		= temp_file("main.bin")

wr.build(main_elf, "ld", list(o_files), implicit = [linker_script])
wr.build(main_bin_ns, "cp", main_elf)
wr.build(main_bin, "sign_and_deploy", main_bin_ns)

