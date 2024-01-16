# Compiler & linker
ASM           = nasm
LIN           = ld
CC            = gcc
QEMU_IMG      = qemu-img


# Directory
SOURCE_FOLDER = src
OUTPUT_FOLDER = bin
ISO_NAME      = os2023

# Flags
WARNING_CFLAG = -Wall -Wextra -Werror
DEBUG_CFLAG   = -ffreestanding -fshort-wchar -g
STRIP_CFLAG   = -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs
CFLAGS        = $(DEBUG_CFLAG) $(WARNING_CFLAG) $(STRIP_CFLAG) -m32 -c -I$(SOURCE_FOLDER)
AFLAGS        = -f elf32 -g -F dwarf
LFLAGS        = -T $(SOURCE_FOLDER)/linker.ld -melf_i386

#kernel
run: all
	@qemu-system-i386 -s -S -cdrom $(OUTPUT_FOLDER)/$(ISO_NAME).iso
all: build
build: complete
clean:
	rm -rf *.o *.iso $(OUTPUT_FOLDER)/kernel

$(OUTPUT_FOLDER)/%.o: $(SOURCE_FOLDER)/%.c
	@$(CC) $(CFLAGS) $< -o $@

SRC := $(filter-out $(SOURCE_FOLDER)/shell/%, $(filter-out $(SOURCE_FOLDER)/chuuloop/%, $(shell find $(SOURCE_FOLDER) -name '*.c')))
DIR := $(filter-out $(SOURCE_FOLDER), $(filter-out $(SOURCE_FOLDER)/shell, $(filter-out $(SOURCE_FOLDER)/chuuloop, $(patsubst $(SOURCE_FOLDER)/%, $(OUTPUT_FOLDER)/%, $(shell find $(SOURCE_FOLDER) -type d)))))
OBJ := $(patsubst $(SOURCE_FOLDER)/%.c, $(OUTPUT_FOLDER)/%.o, $(SRC))

dir: 
	@for dir in $(DIR); do \
		if [ ! -d $$dir ]; then mkdir -p $$dir; fi \
	done

kernel: $(OBJ)
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/kernel_loader.s -o $(OUTPUT_FOLDER)/kernel_loader.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/cpu/intsetup.s -o $(OUTPUT_FOLDER)/intsetup.o
	@$(LIN) $(LFLAGS) $(OBJ) $(OUTPUT_FOLDER)/intsetup.o $(OUTPUT_FOLDER)/kernel_loader.o -o $(OUTPUT_FOLDER)/kernel
	
	@echo Linking object files and generate elf32...
	@rm -rf ${DIR}
	@rm -f *.o

# ngejalanin si kernel doang
iso: dir kernel
	@mkdir -p $(OUTPUT_FOLDER)/iso/boot/grub
	@cp $(OUTPUT_FOLDER)/kernel     $(OUTPUT_FOLDER)/iso/boot/
	@cp other/grub1                 $(OUTPUT_FOLDER)/iso/boot/grub/
	@cp $(SOURCE_FOLDER)/menu.lst   $(OUTPUT_FOLDER)/iso/boot/grub/
	@cd $(OUTPUT_FOLDER)/iso && genisoimage -R				\
			-b boot/grub/grub1						        \
			-no-emul-boot              						\
			-boot-load-size 4          						\
			-A os                      						\
			-input-charset utf8        						\
			-quiet                     						\
			-boot-info-table           						\
			-o OS2023.iso              						\
			.
	@cp $(OUTPUT_FOLDER)/iso/OS2023.iso ./bin
	@rm -r $(OUTPUT_FOLDER)/iso/
	
# ngereset harddisk jadi kosong
disk:
	@cd $(OUTPUT_FOLDER) && $(QEMU_IMG) create -f raw drive.img 64m

# ngecompile yang buat nginsert file
inserter:
	@$(CC) -Wno-builtin-declaration-mismatch \
		$(SOURCE_FOLDER)/lib/stdmem.c other/fat32nocmos.c \
		other/external-inserter.c \
		-o $(OUTPUT_FOLDER)/inserter


# #------------------------------
#shell
$(OUTPUT_FOLDER)/shell/%.o: $(SOURCE_FOLDER)/shell/%.c
	@$(CC) $(CFLAGS) -fno-pie $< -o $@

SRC_U := $(shell find $(SOURCE_FOLDER)/shell -name '*.c')
DIR_U := $(filter-out src/shell, $(patsubst $(SOURCE_FOLDER)/shell/%, $(OUTPUT_FOLDER)/shell/%, $(shell find $(SOURCE_FOLDER)/shell -type d)))
OBJ_U := $(patsubst $(SOURCE_FOLDER)/shell/%.c, $(OUTPUT_FOLDER)/shell/%.o, $(SRC_U))

dir-u: 
	@for dir in $(DIR_U); do \
		if [ ! -d $$dir ]; then mkdir -p $$dir; fi \
	done

# ngecompile shell
shell: dir-u $(OBJ_U)
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/shell/shell-entry.s -o shell-entry.o

	@$(LIN) -T $(SOURCE_FOLDER)/shell/shell-linker.ld -melf_i386 \
		shell-entry.o $(OBJ_U) -o $(OUTPUT_FOLDER)/sh
	@echo Linking object shell object files and generate flat binary...
	
	@$(LIN) -T $(SOURCE_FOLDER)/shell/shell-linker.ld -melf_i386 --oformat=elf32-i386\
		shell-entry.o $(OBJ_U) -o $(OUTPUT_FOLDER)/sh_elf
	@echo Linking object shell object files and generate ELF32 for debugging...
	@size --target=binary bin/sh
	
	@rm -rf ${DIR_U}
	@rm -f *.o


# masukin shell yang udah dicompile ke harddisk
insert-shell: shell
#	segmen unix to dos bisa diskip
# 	@echo Turning possibly dos files into unix files... && cd other && dos2unix stdfont && dos2unix stdbg && dos2unix stdbg2 && dos2unix stdanim && dos2unix text

	@echo Inserting shell into system directory... && cd bin && ./inserter sh 66 drive.img
# 	@echo Inserting font into system directory... && cp other/stdfont bin/stdfont && cd bin && ./inserter stdfont 66 drive.img fnt
# 	@echo Inserting background image into system directory... && cp other/stdbg bin/stdbg && cd bin && ./inserter stdbg 66 drive.img imp
# 	@echo Inserting second background image into system directory... && cp other/stdbg2 bin/stdbg2 && cd bin && ./inserter stdbg2 66 drive.img imp
# 	@echo Inserting animation into system directory... && cp other/stdanim bin/stdanim && cd bin && ./inserter stdanim 66 drive.img anm
# 	@echo Inserting test text into root directory... && cp other/text bin/text && cd bin && ./inserter text 65 drive.img uwu 


#build everything
complete: disk iso inserter insert-shell