/* BEF0.c -- reference-counted cache of the files of the str segment, by file index */

#include "common.h"
#include "libultra.h"
#include "archive.h"
#include "filecache.h"
#include "heap.h"
#include "hostio.h"


/*
================
func_80030AF0

Empties the cache, without freeing what it holds.
================
*/
void func_80030AF0( void ) {
    s32 i;

    for ( i = FILECACHE_SIZE - 1; i >= 0; i-- ) {
        D_800F80B0[i].refCount = 0;
    }
}

/*
================
func_80030B14

Allocates and loads file index, from the host when host files are on, with one reference.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: exact except its format string, which it shares with func_80030C04: the unit's .rodata stays asm until both are C
inline void func_80030B14( s16 index ) {
    u8 name[0x18];
    u32 size;
    void *data;

    if ( D_8005C0BC && D_8005C0BD ) {
        sprintf( name, "data.%4.4d", index );
        size = func_8002961C( name );
    } else {
        size = D_800A30C0[index].rawSize;
    }
    data = func_80029E28( size );
    D_800F80B0[index].refCount = 1;
    D_800F80B0[index].data = data;
    if ( D_8005C0BC && D_8005C0BD ) {
        func_800296D4( data );
    } else {
        func_80029AF4( index, data );
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/BEF0", func_80030B14);
#endif

/*
================
func_80030C04

Returns file index, loading it and relocating its offsets by type when it is not in the cache.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: register/schedule; in the two func_80030B14 expansions of the part loop, retail forms the address of D_800A30C0[index].rawSize in $a3 (lui/addiu, then addu) where this folds it into the load (lui $at); everything else is exact
void *func_80030C04( s16 index, s16 type ) {
    fileType1_t *file1;
    fileType1Part_t *part;
    fileRefA_t *refA;
    fileRefB_t *refB;
    fileType2_t *file2;
    fileType3_t *file3;
    u32 i;
    s32 j;

    if ( D_800F80B0[index].refCount != 0 ) {
        return D_800F80B0[index].data;
    }
    func_80030B14( index );
    switch ( type ) {
    case FILE_TYPE1:
        file1 = D_800F80B0[index].data;
        file1->unk_04 = FILE_RELOC( file1, file1->unk_04 );
        for ( i = 0; i < file1->numParts; i++ ) {
            file1->parts[i] = FILE_RELOC( file1, file1->parts[i] );
            for ( j = 0; j < file1->parts[i]->numRefs; j++ ) {
                if ( D_800F80B0[file1->parts[i]->refs[j].unk_00].refCount == 0 ) {
                    func_80030B14( file1->parts[i]->refs[j].unk_00 );
                    refA = D_800F80B0[file1->parts[i]->refs[j].unk_00].data;
                    refA->unk_04 = FILE_RELOC( refA, refA->unk_04 );
                    refA->unk_08 = FILE_RELOC( refA, refA->unk_08 );
                    func_8002B380( refA->unk_08 );
                }
                if ( D_800F80B0[file1->parts[i]->refs[j].unk_02].refCount == 0 ) {
                    func_80030B14( file1->parts[i]->refs[j].unk_02 );
                    refB = D_800F80B0[file1->parts[i]->refs[j].unk_02].data;
                    refB->unk_2C = FILE_RELOC( refB, refB->unk_2C );
                    refB->unk_30 = FILE_RELOC( refB, refB->unk_30 );
                    refB->unk_34 = FILE_RELOC( refB, refB->unk_2C );
                    refB->unk_38 = FILE_RELOC( refB, refB->unk_30 );
                }
            }
        }
        break;
    case FILE_TYPE2:
        file2 = D_800F80B0[index].data;
        file2->unk_10 = FILE_RELOC( file2, file2->unk_10 );
        file2->unk_14 = FILE_RELOC( file2, file2->unk_14 );
        break;
    case FILE_TYPE3:
        file3 = D_800F80B0[index].data;
        file3->unk_48 = FILE_RELOC( file3, file3->unk_48 );
        file3->unk_4C = FILE_RELOC( file3, file3->unk_4C );
        file3->unk_50 = FILE_RELOC( file3, file3->unk_50 );
        break;
    }
    return D_800F80B0[index].data;
}
#else
INCLUDE_ASM("asm/nonmatchings/BEF0", func_80030C04);
#endif

/*
================
func_80031104

Drops a reference to the cached file at data; frees it with the last one.
================
*/
void func_80031104( void *data ) {
    s32 i;

    for ( i = 0; i < FILECACHE_SIZE; i++ ) {
        if ( D_800F80B0[i].data == data ) {
            if ( --D_800F80B0[i].refCount == 0 ) {
                func_80029F18( D_800F80B0[i].data );
                return;
            }
        }
    }
}

/*
================
func_80031184

Drops a reference to file index; frees it with the last one.
================
*/
void func_80031184( s16 index ) {
    if ( --D_800F80B0[index].refCount == 0 ) {
        func_80029F18( D_800F80B0[index].data );
    }
}

/*
================
func_800311D0

Frees the cached files below index count.
================
*/
void func_800311D0( s16 count ) {
    s32 i;

    for ( i = 0; i < count; i++ ) {
        if ( D_800F80B0[i].refCount != 0 ) {
            func_80029F18( D_800F80B0[i].data );
            D_800F80B0[i].refCount = 0;
        }
    }
}
