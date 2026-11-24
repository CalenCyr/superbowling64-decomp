#ifndef ARCHIVE_H
#define ARCHIVE_H

/* archive.h -- files of the str ROM segment, stored raw or LZSS-compressed (unit 4DC0) */

#include "common.h"

/* LZSS (the classic 4 KB window variant): window size, longest match, and the shortest match coded */
#define LZSS_N          4096
#define LZSS_F          18
#define LZSS_THRESHOLD  2

/* The str segment's file table (the "IDX     SYS" file), STR_FILE_WORDS words per file index. Names are invented.
   A flat word table, not an array of structs: retail forms each word's address as str_files + word * 4 plus
   index * 12 (a struct member access folds the member offset into the load instead). */
#define STR_FILE_WORDS      3
#define STR_FILE_ROMOFFSET  0       /* from the start of the str segment */
#define STR_FILE_SIZE       1       /* bytes stored in ROM */
#define STR_FILE_RAWSIZE    2       /* bytes once decompressed; equal to the stored size when stored raw */

/* word (one of STR_FILE_ROMOFFSET, STR_FILE_SIZE, STR_FILE_RAWSIZE) of file index's entry in str_files */
#define STR_FILE( index, word )     str_files[( index ) * STR_FILE_WORDS + ( word )]

extern u8 str_ROM_START[];          /* linker: ROM address of the str segment */

extern u32 str_files[];             /* the str segment's file table, STR_FILE_WORDS words per file */
extern u8 lz_window[LZSS_N];        /* LZSS window */
extern u8 lz_readBuffer[];          /* where compressed files are read before decompression */

void LZ_Decompress( u8 *dst, u8 *src, u16 len );
u32 Str_ReadFile( s32 index, u8 *dst );

#endif
