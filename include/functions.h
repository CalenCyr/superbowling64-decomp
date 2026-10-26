#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "structs.h"

void func_8003C088( s32 arg );
void func_8003C354( UnkMsg8003C3D4 *msg );
s32 func_8003C3D4( UnkMsg8003C3D4 *msg );
void func_8003C75C( UnkChannel *chan, s32 index );
void func_8003CCA4( UnkChannel *chan, s32 index );
void func_8003CD20( UnkChannel *chan, s32 index );
void func_8003CE74( UnkChannel *chan, s32 index, f32 pitch );
void func_8003CFF0( UnkChannel *chan );
void func_8003D200( UnkChannel *chan );
f32 func_8003D2B8( UnkChannel *chan );
f32 func_8003D310( UnkChannel *chan );
void func_8003D374( UnkChannel *chan );
void func_8003D438( UnkChannel *chan );
void func_8003D6E0( UnkSoundBank *bank, s32 base );
s32 func_8003D878( s32 range );
void func_8003D90C( UnkChannel *chan );
s32 func_8003DA48( UnkSeqHeader *hdr, s32 track );
void func_8003DC10( s32 *table, s32 base, s32 count );
s32 func_8003DEA0( UnkSeqHeader *hdr );
s32 func_8003DC48( UnkChannel *chan, UnkSeqBank *bank, s32 index, s32 arg3, s32 arg4, s32 priority );
s32 func_8003DCF8( UnkSeqBank *bank, s32 index, s32 arg2, s32 arg3, s32 priority );
void func_8003E10C( s32 id, s32 mask, s32 bits );
s32 func_8003F340( s32 arg );
void *func_8003FCAC( s32 size );
void func_8003FD2C( void *dst, void *src, s32 len );

void func_800424B0( UnkVoice *voice, s32 pan );
void func_800425D0( UnkVoice *voice, s32 volume, s32 time );
void func_80042680( UnkVoice *voice, s32 arg1 );
void func_80042710( UnkVoice *voice );

f32 __sinf( f32 x );

#endif
