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

/* synthesizer and effect setup (19F50.c) */
void func_8003EB50( N_ALGlobals *g, ALSynConfig *c );
void func_8003EBB8( ALSynConfig *c );
ALFx *func_8003EE28( s16 bus, ALSynConfig *c, ALHeap *hp );
void func_8003EE78( ALFx **fx, ALSynConfig *c, ALHeap *hp );
void func_8003EFC0( s32 *param );
void func_8003F2A0( ALSynConfig *c );
s32 func_8003F2B8( void );
s32 func_8003F2FC( void );
s32 func_8003F340( s32 index );


/* ROM sample cache and scheduler client (1A7A0.c) */
ALDMANew func_8003F3A0( s32 count, s32 size );
void func_8003F514( void );
ALDMAproc func_8003F5E4( void *state );
s32 func_8003F5F0( s32 addr, s32 len, void *state );
dmaBuffer_t *func_8003F65C( u32 addr, s32 len );
void func_8003F7F0( OSSched *sc );
void func_8003F7FC( void );
void func_8003F860( void );
void func_8003F8C0( audioTask_t *audio );

void func_8003F9A0( sndConfig_t *config, s32 rate, s32 arg2 );
void Snd_InitHeap( u8 *base, s32 len );
void *Snd_Alloc( s32 size );
s32 Snd_HeapUsed( void );
ALHeap *Snd_GetHeap( void );
void Snd_MemSet( void *dst, s32 value, u32 len );
void Snd_MemMove( void *dst, void *src, u32 len );
u32 Snd_InitFrameSizes( u32 ticks, u32 outputRate, u32 tickRate, u32 extraPercent );
u32 Snd_NextFrameSize( u32 queued );

/* called from the boot units */
s32 Snd_PlayBgm( seqHeader_t *hdr );
s32 Snd_SetVolume( s32 id, s32 value );

#endif
