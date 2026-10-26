build/superbowling.elf: \
    build/asm/header.o \
    build/assets/ipl3.o \
    build/asm/1000.o \
    build/asm/1060.o \
    build/asm/data/32150.data.o \
    build/asm/data/3C2A0.bss.o \
    build/assets/3C2A0.o
build/asm/header.o:
build/assets/ipl3.o:
build/asm/1000.o:
build/asm/1060.o:
build/asm/data/32150.data.o:
build/asm/data/3C2A0.bss.o:
build/assets/3C2A0.o:
-include build/asm/header.d build/assets/ipl3.d build/asm/1000.d build/asm/1060.d build/asm/data/32150.data.d build/asm/data/3C2A0.bss.d build/assets/3C2A0.d
