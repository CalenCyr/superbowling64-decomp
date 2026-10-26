#ifndef HEAP_H
#define HEAP_H

/* heap.h -- the block allocator (unit 51F0) */

#include "common.h"

/*
Header in front of each allocation; the blocks form a list in address order from D_80062A20. Names of the
type and its members are invented.
*/
typedef struct HeapBlock_s {
    /* 0x00 */ struct HeapBlock_s *next;
    /* 0x04 */ u32 size;                /* bytes from this header to the end of the block, header included */
} HeapBlock;

extern HeapBlock *D_80062A20;           /* first block; the heap starts here */
extern u8 *D_80062A24;                  /* end of the heap */
extern s32 D_80062A28;
extern s32 D_80066B88;                  /* bytes left after the last block */

void func_80029DF0( HeapBlock *base, u8 *end );
void *func_80029E28( u32 size );
void func_80029F18( void *ptr );
void func_80029F68( void *ptr, u32 size );

#endif
