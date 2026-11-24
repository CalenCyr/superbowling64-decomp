/* BEF0.c -- reference-counted cache of the files of the str segment, by file index */

#include "common.h"
#include "libultra.h"
#include "archive.h"
#include "filecache.h"
#include "heap.h"
#include "hostio.h"


/*
================
Cache_Clear

Empties the cache, without freeing what it holds.
================
*/
void Cache_Clear( void ) {
    s32 i;

    for ( i = FILECACHE_SIZE - 1; i >= 0; i-- ) {
        cache_files[i].refCount = 0;
    }
}

/*
================
Cache_LoadFile

Allocates and loads file index, from the host when host files are on, with one reference.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: exact except its format string, which it shares with Cache_GetFile: the unit's .rodata stays asm until both are C
inline void Cache_LoadFile( s16 index ) {
    u8 name[0x18];
    u32 size;
    void *data;

    if ( cache_hostMode && cache_hostFiles ) {
        sprintf( name, "data.%4.4d", index );
        size = Host_GetFileSize( name );
    } else {
        size = str_files[index].rawSize;
    }
    data = Mem_Alloc( size );
    cache_files[index].refCount = 1;
    cache_files[index].data = data;
    if ( cache_hostMode && cache_hostFiles ) {
        Host_ReadFile( data );
    } else {
        Str_ReadFile( index, data );
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/BEF0", Cache_LoadFile);
#endif

/*
================
Cache_GetFile

Returns file index, loading it and relocating its offsets by type when it is not in the cache.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: register/schedule; in the two Cache_LoadFile expansions of the part loop, retail forms the address of str_files[index].rawSize in $a3 (lui/addiu, then addu) where this folds it into the load (lui $at); everything else is exact. Tried: the
// two reference indices read into s16 locals first (worse)
void *Cache_GetFile( s16 index, s16 type ) {
    fileType1_t *file1;
    fileType1Part_t *part;
    fileRefA_t *refA;
    fileRefB_t *refB;
    fileType2_t *file2;
    fileType3_t *file3;
    u32 i;
    s32 j;

    if ( cache_files[index].refCount != 0 ) {
        return cache_files[index].data;
    }
    Cache_LoadFile( index );
    switch ( type ) {
    case FILE_TYPE1:
        file1 = cache_files[index].data;
        file1->unk_04 = FILE_RELOC( file1, file1->unk_04 );
        for ( i = 0; i < file1->numParts; i++ ) {
            file1->parts[i] = FILE_RELOC( file1, file1->parts[i] );
            for ( j = 0; j < file1->parts[i]->numRefs; j++ ) {
                if ( cache_files[file1->parts[i]->refs[j].mesh].refCount == 0 ) {
                    Cache_LoadFile( file1->parts[i]->refs[j].mesh );
                    refA = cache_files[file1->parts[i]->refs[j].mesh].data;
                    refA->verts = FILE_RELOC( refA, refA->verts );
                    refA->cmds = FILE_RELOC( refA, refA->cmds );
                    Obj_ConvertModel( refA->cmds );
                }
                if ( cache_files[file1->parts[i]->refs[j].material].refCount == 0 ) {
                    Cache_LoadFile( file1->parts[i]->refs[j].material );
                    refB = cache_files[file1->parts[i]->refs[j].material].data;
                    refB->palette = FILE_RELOC( refB, refB->palette );
                    refB->texels = FILE_RELOC( refB, refB->texels );
                    refB->palette2 = FILE_RELOC( refB, refB->palette );
                    refB->texels2 = FILE_RELOC( refB, refB->texels );
                }
            }
        }
        break;
    case FILE_TYPE2:
        file2 = cache_files[index].data;
        file2->unk_10 = FILE_RELOC( file2, file2->unk_10 );
        file2->unk_14 = FILE_RELOC( file2, file2->unk_14 );
        break;
    case FILE_TYPE3:
        file3 = cache_files[index].data;
        file3->frames = FILE_RELOC( file3, file3->frames );
        file3->light = FILE_RELOC( file3, file3->light );
        file3->unk_50 = FILE_RELOC( file3, file3->unk_50 );
        break;
    }
    return cache_files[index].data;
}
#else
INCLUDE_ASM("asm/nonmatchings/BEF0", Cache_GetFile);
#endif

/*
================
Cache_ReleaseData

Drops a reference to the cached file at data; frees it with the last one.
================
*/
void Cache_ReleaseData( void *data ) {
    s32 i;

    for ( i = 0; i < FILECACHE_SIZE; i++ ) {
        if ( cache_files[i].data == data ) {
            if ( --cache_files[i].refCount == 0 ) {
                Mem_Free( cache_files[i].data );
                return;
            }
        }
    }
}

/*
================
Cache_Release

Drops a reference to file index; frees it with the last one.
================
*/
void Cache_Release( s16 index ) {
    if ( --cache_files[index].refCount == 0 ) {
        Mem_Free( cache_files[index].data );
    }
}

/*
================
Cache_FreeBelow

Frees the cached files below index count.
================
*/
void Cache_FreeBelow( s16 count ) {
    s32 i;

    for ( i = 0; i < count; i++ ) {
        if ( cache_files[i].refCount != 0 ) {
            Mem_Free( cache_files[i].data );
            cache_files[i].refCount = 0;
        }
    }
}
