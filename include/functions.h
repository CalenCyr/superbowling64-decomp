#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "structs.h"

void Snd_SetMasterVolume( s32 flags, s16 value );
void Snd_LoadWaveBank( waveBank_t *bank, s32 romBase );
void Snd_Resume( s32 id );
void Snd_LoadSeTable( void *data );
void Snd_InitMessages( s32 count );
ALMicroTime Snd_PlayerHandler( void *clientData );
void Snd_HandleMessage( sndMsg_t *msg );
s32 Snd_PostMessage( sndMsg_t *msg );
void Snd_PlayNextNote( sndChannel_t *chan, s32 index );
void Snd_StartVoice( sndChannel_t *chan, s32 index );
void Snd_UpdateVolumePan( sndChannel_t *chan, s32 index );
void Snd_UpdatePitch( sndChannel_t *chan, s32 index, f32 pitch );
void Snd_StartEnvelope( sndChannel_t *chan );
void Snd_UpdateEnvelope( sndChannel_t *chan );
void Snd_StartAutoPan( sndChannel_t *chan );
void Snd_UpdateAutoPan( sndChannel_t *chan );
f32 Snd_UpdateTrill( sndChannel_t *chan );
f32 Snd_UpdateVibrato( sndChannel_t *chan );
void Snd_StepVolumeStream( sndChannel_t *chan );
void Snd_StepBendStream( sndChannel_t *chan );
f32 Snd_Exp2( f32 x );
void Snd_RelocateWaveBank( void *data, s32 romBase );
s32 Snd_Random( s32 range );
void Snd_ResetChannel( sndChannel_t *chan );
s32 Snd_FindBgmChannel( seqHeader_t *hdr, s32 track );
void Snd_RelocateTable( s32 *table, s32 base, s32 count );
s32 Snd_StartBgm( seqHeader_t *hdr );
s32 Snd_StartSeOnChannel( sndChannel_t *chan, seTable_t *table, s32 index, s32 volume, s32 pan, s32 priority );
s32 Snd_StartSe( seTable_t *table, s32 index, s32 volume, s32 pan, s32 priority );
void Snd_SetChannelFlags( s32 id, s32 mask, s32 bits );
void func_8003F7F0( s32 arg );
void func_8003F9A0( sndConfig_t *config, s32 rate, s32 arg2 );
s32 func_8003F340( s32 arg );
void Snd_InitHeap( s32 arg0, s32 arg1 );
void *Snd_Alloc( s32 size );
void Snd_MemMove( void *dst, void *src, s32 len );
s32 Snd_HeapUsed( void );

#endif
