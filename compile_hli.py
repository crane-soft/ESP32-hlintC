import os
import stat
import re
import subprocess
from pathlib import Path

Import("env")
# https://scons.org/doc/2.2.0/HTML/scons-man.html

hli_folder = ".\\hli"
src_folder = ".\\src"

hlevelint_name = "hlevelint.S"
this_file_path = "compile_hli.py"

reg_save_token    = "[REG_SAVE]"
reg_restore_token = "[REG_RESTORE]"


def prepare_compiler():
	platform = env.PioPlatform()
	toolchain_xtensa_dir = platform.get_package_dir("toolchain-xtensa-esp-elf")
	framework_dir = platform.get_package_dir("framework-arduinoespressif32")
	sdk_dir = os.path.join (framework_dir, "tools/sdk/esp32s3")

	global gcc_compiler
	gcc_compiler = os.path.join( toolchain_xtensa_dir, "bin/xtensa-esp32s3-elf-gcc")

	incl_path  = " -I ./include"
	incl_path += " -I " + os.path.join (sdk_dir, "include/soc/esp32s3/include")
	incl_path += " -I " + os.path.join (sdk_dir, "include/esp_common/include")
	incl_path += " -I " + os.path.join (sdk_dir, "qio_opi/include")

	if (env.get("BUILD_TYPE") == "debug") :
		optm_options = " -g -Og"
	else :
		optm_options = " -O1"
		
	global compile_options
	compile_options = incl_path + optm_options + " -mabi=call0 -S -o "


def compile_hli (cfile, ofile):
	compile_cmd = gcc_compiler + compile_options + ofile + " " +cfile
	print ("Compiling: " + cfile)

	retCode = env.Execute (compile_cmd)
	if retCode != 0 :
		env.Exit(1)

def register_usage(file_name, reg_list: list[str]) -> list[str]:
	asm_file = open (file_name)
	asm_text = asm_file.read()
	for regNo in range(2, 16):
		regx = "a"+str(regNo)		# build reg-name from number
		if (not regx in reg_list) :	# is reg-name already used skip searching in file
			pattern = rf"\b{re.escape(regx)}\b"	# create patter with word boundaries
			if re.search(pattern, asm_text):	# if reg-name found in file
				reg_list.append (regx)			# append to reg list
	return reg_list
	
def hli_register_usage(asm_files: list[str])  -> list[str]:
	print ("Computing hlevel interrupt register usage")
	reg_list = []
	for asm_file in asm_files :
		reg_list = register_usage (asm_file, reg_list)
	print ("register usage=", end='')
	print (reg_list)
	return reg_list

def insert_lines_at_token(s: str, itoken: str, insert_lines: list[str]) -> str:
	lines = s.splitlines(keepends=True)  # preserve existing newlines
	out = []
	i = 0
	n = len(lines)

	while i < n:
		if itoken in lines[i] :
			# add insert lines (ensure they end with newline)
			out.append("\t// --- auto generated ---\n")
			for rl in insert_lines:
				out.append(rl if rl.endswith("\n") else rl + "\n")
			out.append("\t// ----------------------\n")
				
		else:
			out.append(lines[i])
		i += 1

	return "".join(out)

def create_asm_lines(reg_list: list[str], asm_cmd) -> list[str]:
	asm_lines = []
	addr_offs = 0
	for regx in reg_list:
		asm_lines.append ("\t" + asm_cmd + "\t" + regx + ",sp," + str(addr_offs))
		addr_offs += 4
	return asm_lines


def update_hlevelint_file(reg_list: list[str]):

	hlevelint_src = os.path.join(hli_folder, hlevelint_name)
	hlevelint_dst = os.path.join(src_folder, hlevelint_name)

	src_file = open(hlevelint_src)
	dst_text  = "// This file was copied from " + hlevelint_src + ".\n"
	dst_text += "// The file is read only. Do not try to edit this file.\n"
	dst_text +=	"// Edit the original one in the hli-folder.\n"
	dst_text += src_file.read()

	# insert save register asm-lines
	asm_lines = create_asm_lines(reg_list,"s32i")
	asm_lines.insert (0, "\taddi\tsp, sp, " + str(-len(reg_list) *4 ))
	dst_text = insert_lines_at_token(dst_text, reg_save_token, asm_lines)

	# insert restore register asm-lines
	asm_lines = create_asm_lines(reg_list,"l32i")
	asm_lines.append ("\taddi\tsp, sp, " + str(len(reg_list) *4 ))

	dst_text = insert_lines_at_token(dst_text, reg_restore_token, asm_lines)

	src_file.close()

	if os.path.exists(hlevelint_dst):
		os.chmod(hlevelint_dst, stat.S_IREAD | stat.S_IWRITE)

	dst_file =  open(hlevelint_dst, "w")
	dst_file.write (dst_text)
	dst_file.close()
	os.chmod(hlevelint_dst, stat.S_IREAD)	

def remove_file (file_path):
	if os.path.exists(file_path):
		os.remove(file_path)		
		print ("Removing: " + file_path)

def clean_hli_targets():
	print ("Cleaning hli targets")
	cFileList = list(Path(hli_folder).glob('*.c*'))
	cFileList = [f.name for f in Path(hli_folder).glob('*.c*')]
	for cFile in cFileList:
		ofile = os.path.splitext(cFile)[0] + ".S"
		ofile = os.path.join (src_folder, ofile)
		remove_file (ofile)
		if os.path.exists(ofile):
			os.remove(ofile)		
			print ("Removing: " + ofile)

	hlevelint_dst = os.path.join(src_folder, hlevelint_name)
	os.chmod(hlevelint_dst, stat.S_IREAD | stat.S_IWRITE)
	remove_file (hlevelint_dst)



def build_hli_folder():

#	print ("Compile_hli script called")
	if env.IsIntegrationDump(): 
		return

	if env.IsCleanTarget():
		clean_hli_targets()
		return

	cFileList = list(Path(hli_folder).glob('*.c*'))
	cFileList = [f.name for f in Path(hli_folder).glob('*.c*')]
#	print (cFileList)
	if (len (cFileList) == 0) :
		print ("No C-source files found in: ",hli_folder)
		return

	oFileList = []
	build_list : str[int] = []
	this_time = os.path.getmtime(this_file_path)

	for fidx in range (0, len(cFileList)):
		cfile = cFileList[fidx]
		ofile = os.path.splitext(cfile)[0] + ".S"
		ofile = os.path.join (src_folder, ofile)
		oFileList.append (ofile)

		cfile = os.path.join (hli_folder, cfile)
		cFileList[fidx] = cfile


		if (not os.path.exists(ofile)) :
			build_list.append (fidx)
		else :
			cfile_time = os.path.getmtime(cfile)			
			ofile_time = os.path.getmtime(ofile)
			if ((cfile_time > ofile_time) or (this_time > ofile_time)):
				build_list.append (fidx)

	if (len(build_list) > 0) :
		prepare_compiler()
		for n in range(0,len(build_list)) :
			fidx = build_list[n]
			compile_hli(cFileList[fidx],oFileList[fidx])

	# TODO check time of hlevelint_name

	reg_list = hli_register_usage (oFileList)
	update_hlevelint_file(reg_list)


build_hli_folder()	# check datetime and compile if source is newer

# --- Skip hli-folder from build process
def skip_file_from_build(env, node):
	if str(node.get_dir().name) == "hli":
		print (node.name + " skipped")		
		return None

	return node

# not used, folder hli seems not part of bild process
#env.AddBuildMiddleware(skip_file_from_build, "*.c")

