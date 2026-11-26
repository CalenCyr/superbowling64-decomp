/* 51F0.c -- block allocator over one heap region */

#include "common.h"
#include "heap.h"
#include "libc.h"

#define HEAP_ALIGN( size )  ( ( ( size ) + 7 ) & ~7 )

/* The block that starts where block ends. */
#define BLOCK_END( block )  ( (HeapBlock *)( (u8 *)( block ) + ( block )->size ) )

/*
================
Mem_Reset

Starts the heap over: at base, ending at end, when base is given; otherwise over the current region.
================
*/
void Mem_Reset( HeapBlock *base, u8 *end ) {
    if ( base != NULL ) {
        mem_first = base;
        mem_end = end;
    }
    D_80062A28 = 0;
    mem_first->next = NULL;
    mem_first->size = sizeof( HeapBlock );
}

/*
================
Mem_Alloc

Allocates size bytes, rounded up to 8, in the first gap between blocks that holds them, or after the last
block; returns them cleared.
================
*/
void *Mem_Alloc( u32 size ) {
    HeapBlock *block;
    HeapBlock *newBlock;

    block = mem_first;
    size = HEAP_ALIGN( size );
    while ( block->next != NULL ) {
        newBlock = BLOCK_END( block );
        if ( (u8 *)block->next - (u8 *)newBlock >= size + sizeof( HeapBlock ) ) {
            newBlock->next = block->next;
            block->next = BLOCK_END( block );
            break;
        }
        block = block->next;
    }
    if ( block->next == NULL ) {
        block->next = BLOCK_END( block );
        BLOCK_END( block )->next = NULL;
        mem_lastFree = mem_end - ( (u8 *)BLOCK_END( block ) + ( size + sizeof( HeapBlock ) ) );
    }
    BLOCK_END( block )->size = size + sizeof( HeapBlock );
    memset( BLOCK_END( block ) + 1, 0, size );
    return BLOCK_END( block ) + 1;
}

/*
================
Mem_Free

Frees the block that holds ptr.
================
*/
void Mem_Free( void *ptr ) {
    HeapBlock *prev;
    HeapBlock *block;

    for ( prev = block = mem_first; block != NULL; prev = block, block = block->next ) {
        if ( (void *)block < ptr && ptr < (void *)BLOCK_END( block ) ) {
            prev->next = block->next;
            return;
        }
    }
}

/*
================
Mem_Resize

Sets the size of the allocation at ptr to size bytes, rounded up to 8.
================
*/
void Mem_Resize( void *ptr, u32 size ) {
    HeapBlock *block;

    block = mem_first;
    size = HEAP_ALIGN( size ) + sizeof( HeapBlock );
    do {
        if ( (void *)( block + 1 ) == ptr ) {
            block->size = size;
            return;
        }
        block = block->next;
    } while ( block != NULL );
}
