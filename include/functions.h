#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "structs.h"

void Snd_SetMasterVolume( s32 flags, s32 value );
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
void Snd_RelocateTable( s32 *table, void *data, s32 count );
s32 Snd_StartBgm( void *data );
s32 Snd_StartSeOnChannel( sndChannel_t *chan, seTable_t *table, s32 index, s32 volume, s32 pan, s32 priority );
s32 Snd_StartSe( seTable_t *table, s32 index, s32 volume, s32 pan, s32 priority );
void Snd_SetChannelFlags( s32 id, s32 mask, s32 bits );

/* synthesizer and effect setup (19F50.c) */
void Synth_Init( N_ALGlobals *g, ALSynConfig *c );
void Synth_Create( ALSynConfig *c );
ALFx *Synth_CreateFx( s16 bus, ALSynConfig *c, ALHeap *hp );
void Synth_AllocFx( ALFx **fx, ALSynConfig *c, ALHeap *hp );
void Synth_SetupFx( s32 *param );
void Synth_UseCustomFx( ALSynConfig *c );
s32 Synth_MaxFxSections( void );
s32 Synth_MaxFxLength( void );
s32 Synth_SetFxPreset( s32 index );


/* ROM sample cache and scheduler client (1A7A0.c) */
ALDMANew Audio_InitDmaCache( s32 count, s32 size );
void Audio_UpdateDmaCache( void );
ALDMAproc Audio_DmaSetup( void *state );
s32 Audio_DmaRead( s32 addr, s32 len, void *state );
dmaBuffer_t *Audio_FindDmaBuffer( u32 addr, s32 len );
void Audio_SetScheduler( OSSched *sc );
void Audio_ClientInit( void );
void Audio_ClientWaitFrame( void );
void Audio_ClientRunTask( audioTask_t *audio );


/* audio thread (1ADA0.c) */
void Audio_Init( sndConfig_t *config, s32 tickRate, s32 fxType );
void Audio_Thread( void *arg );
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
