/* 51F0.c -- block allocator over one heap region */

#include "common.h"
#include "heap.h"
#include "libc_boot.h"

#define HEAP_ALIGN( size )  ( ( ( size ) + 7 ) & ~7 )

/* The block that starts where block ends. */
#define BLOCK_END( block )  ( (HeapBlock *)( (u8 *)( block ) + ( block )->size ) )

/*
================
func_80029DF0

Starts the heap over: at base, ending at end, when base is given; otherwise over the current region.
================
*/
void func_80029DF0( HeapBlock *base, u8 *end ) {
    if ( base != NULL ) {
        D_80062A20 = base;
        D_80062A24 = end;
    }
    D_80062A28 = 0;
    D_80062A20->next = NULL;
    D_80062A20->size = sizeof( HeapBlock );
}

/*
================
func_80029E28

Allocates size bytes, rounded up to 8, in the first gap between blocks that holds them, or after the last
block; returns them cleared.
================
*/
void *func_80029E28( u32 size ) {
    HeapBlock *block;
    HeapBlock *newBlock;

    block = D_80062A20;
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
        D_80066B88 = D_80062A24 - ( (u8 *)BLOCK_END( block ) + ( size + sizeof( HeapBlock ) ) );
    }
    BLOCK_END( block )->size = size + sizeof( HeapBlock );
    func_80056150( BLOCK_END( block ) + 1, 0, size );
    return BLOCK_END( block ) + 1;
}

/*
================
func_80029F18

Frees the block that holds ptr.
================
*/
void func_80029F18( void *ptr ) {
    HeapBlock *prev;
    HeapBlock *block;

    for ( prev = block = D_80062A20; block != NULL; prev = block, block = block->next ) {
        if ( (void *)block < ptr && ptr < (void *)BLOCK_END( block ) ) {
            prev->next = block->next;
            return;
        }
    }
}

/*
================
func_80029F68

Sets the size of the allocation at ptr to size bytes, rounded up to 8.
================
*/
void func_80029F68( void *ptr, u32 size ) {
    HeapBlock *block;

    block = D_80062A20;
    size = HEAP_ALIGN( size ) + sizeof( HeapBlock );
    do {
        if ( (void *)( block + 1 ) == ptr ) {
            block->size = size;
            return;
        }
        block = block->next;
    } while ( block != NULL );
}
