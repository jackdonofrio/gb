# NOTE: currently configured for WSL (hence the CC path including c drive)
CC	= /mnt/c/gbdk/bin/lcc.exe

all:
	$(CC) -o main.gb main.c simplebackground.c simplebackgroundmap.c simplebackgroundmap2.c

compile.bat: Makefile
	@echo "REM Automatically generated from Makefile" > compile.bat
	@make -sn | sed y/\\//\\\\/ | grep -v make >> compile.bat

clean:
	rm -f *.o *.lst *.map *.gb *.ihx *.sym *.cdb *.adb *.asm
