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
STRIP_CFLAG   = -nostdlib -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs
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

SRC := $(filter-out $(SOURCE_FOLDER)/bouncy/%, $(filter-out $(SOURCE_FOLDER)/shell/%, $(filter-out $(SOURCE_FOLDER)/clock/%, $(shell find $(SOURCE_FOLDER) -name '*.c'))))
DIR := $(filter-out $(SOURCE_FOLDER), $(filter-out $(SOURCE_FOLDER)/bouncy/%, $(filter-out $(SOURCE_FOLDER)/shell, $(filter-out $(SOURCE_FOLDER)/clock, $(patsubst $(SOURCE_FOLDER)/%, $(OUTPUT_FOLDER)/%, $(shell find $(SOURCE_FOLDER) -type d))))))
OBJ := $(patsubst $(SOURCE_FOLDER)/%.c, $(OUTPUT_FOLDER)/%.o, $(SRC))

dir: 
	@for dir in $(DIR); do \
		if [ ! -d $$dir ]; then mkdir -p $$dir; fi \
	done

kernel: $(OBJ)
	@echo Linking object files and generate elf32...
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/kernel_loader.s -o $(OUTPUT_FOLDER)/kernel_loader.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/cpu/intsetup.s -o $(OUTPUT_FOLDER)/intsetup.o
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/process/context-switch.s -o $(OUTPUT_FOLDER)/context-switch.o
	@$(LIN) $(LFLAGS) $(OBJ) $(OUTPUT_FOLDER)/intsetup.o $(OUTPUT_FOLDER)/kernel_loader.o $(OUTPUT_FOLDER)/context-switch.o -o $(OUTPUT_FOLDER)/kernel
	
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


# TODO: Make autocompile for these
# USER PROGRAMS
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
	@echo Compiling shell...
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/shell/shell-entry.s -o shell-entry.o

	@echo Linking object shell object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/shell/shell-linker.ld -melf_i386 \
		shell-entry.o $(OBJ_U) -o $(OUTPUT_FOLDER)/sh
	
	@echo Linking object shell object files and generate ELF32 for debugging...
	@$(LIN) -T $(SOURCE_FOLDER)/shell/shell-linker.ld -melf_i386 --oformat=elf32-i386\
		shell-entry.o $(OBJ_U) -o $(OUTPUT_FOLDER)/sh_elf
	@size --target=binary bin/sh
	
	@rm -rf ${DIR_U}
	@rm -f *.o


# #------------------------------
#clock
$(OUTPUT_FOLDER)/clock/%.o: $(SOURCE_FOLDER)/clock/%.c
	@$(CC) $(CFLAGS) -fno-pie $< -o $@

SRC_C := $(shell find $(SOURCE_FOLDER)/clock -name '*.c')
DIR_C := $(filter-out src/clock, $(patsubst $(SOURCE_FOLDER)/clock/%, $(OUTPUT_FOLDER)/clock/%, $(shell find $(SOURCE_FOLDER)/clock -type d)))
OBJ_C := $(patsubst $(SOURCE_FOLDER)/clock/%.c, $(OUTPUT_FOLDER)/clock/%.o, $(SRC_C))

dir-c: 
	@for dir in $(DIR_C); do \
		if [ ! -d $$dir ]; then mkdir -p $$dir; fi \
	done

# ngecompile clock
clock: dir-c $(OBJ_C)
	@echo Compiling clock...
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/clock/shell-entry.s -o shell-entry.o

	@echo Linking object shell object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/clock/shell-linker.ld -melf_i386 \
		shell-entry.o $(OBJ_C) -o $(OUTPUT_FOLDER)/sysclock
	
	@echo Linking object clock object files and generate ELF32 for debugging...
	@$(LIN) -T $(SOURCE_FOLDER)/clock/shell-linker.ld -melf_i386 --oformat=elf32-i386\
		shell-entry.o $(OBJ_C) -o $(OUTPUT_FOLDER)/sysclock_elf
	@size --target=binary bin/sysclock
	
	@rm -rf ${DIR_C}
	@rm -f *.o

# #------------------------------
#bouncy
$(OUTPUT_FOLDER)/bouncy/%.o: $(SOURCE_FOLDER)/bouncy/%.c
	@$(CC) $(CFLAGS) -fno-pie $< -o $@

SRC_B := $(shell find $(SOURCE_FOLDER)/bouncy -name '*.c')
DIR_B := $(filter-out src/bouncy, $(patsubst $(SOURCE_FOLDER)/bouncy/%, $(OUTPUT_FOLDER)/bouncy/%, $(shell find $(SOURCE_FOLDER)/bouncy -type d)))
OBJ_B := $(patsubst $(SOURCE_FOLDER)/bouncy/%.c, $(OUTPUT_FOLDER)/bouncy/%.o, $(SRC_B))

dir-b: 
	@for dir in $(DIR_B); do \
		if [ ! -d $$dir ]; then mkdir -p $$dir; fi \
	done

# ngecompile bouncy
bouncy: dir-b $(OBJ_B)
	@echo Compiling bouncy...
	@$(ASM) $(AFLAGS) $(SOURCE_FOLDER)/bouncy/shell-entry.s -o shell-entry.o

	@echo Linking object shell object files and generate flat binary...
	@$(LIN) -T $(SOURCE_FOLDER)/bouncy/shell-linker.ld -melf_i386 \
		shell-entry.o $(OBJ_B) -o $(OUTPUT_FOLDER)/bounce
	
	@echo Linking object bouncy object files and generate ELF32 for debugging...
	@$(LIN) -T $(SOURCE_FOLDER)/bouncy/shell-linker.ld -melf_i386 --oformat=elf32-i386\
		shell-entry.o $(OBJ_B) -o $(OUTPUT_FOLDER)/bounce_elf
	@size --target=binary bin/bounce
	
	@rm -rf ${DIR_B}
	@rm -f *.o

# masukin program yang udah dicompile ke harddisk
insert: shell clock bouncy
#	segmen unix to dos bisa diskip
# 	@echo Turning possibly dos files into unix files... && cd other && dos2unix stdfont && dos2unix stdbg && dos2unix stdbg2 && dos2unix stdanim && dos2unix text

	@echo Inserting shell into system directory... && cd bin && ./inserter sh 66 drive.img
	@echo Inserting clock into system directory... && cd bin && ./inserter sysclock 66 drive.img prg
	@echo Inserting bouncy into system directory... && cd bin && ./inserter bounce 66 drive.img prg
# 	@echo Inserting font into system directory... && cp other/stdfont bin/stdfont && cd bin && ./inserter stdfont 66 drive.img fnt
# 	@echo Inserting background image into system directory... && cp other/stdbg bin/stdbg && cd bin && ./inserter stdbg 66 drive.img imp
# 	@echo Inserting second background image into system directory... && cp other/stdbg2 bin/stdbg2 && cd bin && ./inserter stdbg2 66 drive.img imp
# 	@echo Inserting animation into system directory... && cp other/stdanim bin/stdanim && cd bin && ./inserter stdanim 66 drive.img anm
# 	@echo Inserting test text into root directory... && cp other/text bin/text && cd bin && ./inserter text 65 drive.img uwu 


#build everything
complete: disk iso inserter insert