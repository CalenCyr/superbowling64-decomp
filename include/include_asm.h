#ifndef INCLUDE_ASM_H
#define INCLUDE_ASM_H

/*
 * A unit with no INCLUDE_ASM left may define NO_INCLUDE_ASM before its first include: it then gets no file-scope
 * asm prelude, which changes the assembler's hazard state at the start of .text (without it, KMC as puts a nop
 * before a leading mult/div, as in the original objects).
 */
#if !defined(M2CTX) && !defined(PERMUTER) && !defined(NO_INCLUDE_ASM)

#ifndef INCLUDE_ASM
#define INCLUDE_ASM(FOLDER, NAME) \
    __asm__( \
        ".section .text\n" \
        "    .set noat\n" \
        "    .set noreorder\n" \
        "    .include \"" FOLDER "/" #NAME ".s\"\n" \
        "    .set reorder\n" \
        "    .set at\n" \
    )
#endif
#ifndef INCLUDE_RODATA
#define INCLUDE_RODATA(FOLDER, NAME) \
    __asm__( \
        ".section .rodata\n" \
        "    .include \"" FOLDER "/" #NAME ".s\"\n" \
        ".section .text" \
    )
#endif

#if INCLUDE_ASM_USE_MACRO_INC
__asm__(".include \"include/macro.inc\"\n");
#else
__asm__(".include \"include/labels.inc\"\n");
#endif

#else

#ifndef INCLUDE_ASM
#define INCLUDE_ASM(FOLDER, NAME)
#endif
#ifndef INCLUDE_RODATA
#define INCLUDE_RODATA(FOLDER, NAME)
#endif

#endif /* !defined(M2CTX) && !defined(PERMUTER) && !defined(NO_INCLUDE_ASM) */

#endif /* INCLUDE_ASM_H */
