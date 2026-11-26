#ifndef HEAP_H
#define HEAP_H

/* heap.h -- the block allocator (unit 51F0) */

#include "common.h"

/*
Header in front of each allocation; the blocks form a list in address order from mem_first. Names of the
type and its members are invented.
*/
typedef struct HeapBlock_s {
    /* 0x00 */ struct HeapBlock_s *next;
    /* 0x04 */ u32 size;                /* bytes from this header to the end of the block, header included */
} HeapBlock;

extern s32 mem_lastFree;                /* bytes left after the last block */

void Mem_Reset( HeapBlock *base, u8 *end );
void *Mem_Alloc( u32 size );
void Mem_Free( void *ptr );
void Mem_Resize( void *ptr, u32 size );

#endif
