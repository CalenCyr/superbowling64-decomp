/*
 * 1ADA0.c -- the audio thread: each frame it waits for the retrace, queues the last frame's samples on the AI,
 * has n_audio build the next frame's command list, and runs it on the RSP into the next output buffer.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

#define AUDIO_THREAD_ID     3
#define FRAME_QUEUE_PERCENT 15      /* AI queue kept ahead, in percent of a frame (Snd_InitFrameSizes) */

/*
================
func_8003F9A0

Sets up the synthesizer, the ROM sample cache and the output buffers from config, ticking tickRate times a
second with effect fxType, and starts the audio thread.
================
*/
void func_8003F9A0( sndConfig_t *config, s32 tickRate, s32 fxType ) {
    ALSynConfig c;
    u32 bufSamples;
    u32 i;

    c.maxVVoices = c.maxPVoices = config->numVoices;
    c.maxUpdates = config->maxUpdates;
    c.dmaproc = func_8003F3A0( config->numDmaBuffers, config->dmaBufferSize );
    c.fxType = fxType;
    c.outputRate = osAiSetFrequency( config->outputRate );
    c.heap = Snd_GetHeap();
    func_8003EB50( &D_800E64D0, &c );

    bufSamples = Snd_InitFrameSizes( config->ticksPerFrame, c.outputRate, tickRate, FRAME_QUEUE_PERCENT );
    D_80064598 = Snd_Alloc( config->maxCmds * sizeof( Acmd ) );
    D_80064594 = Snd_Alloc( NUM_AUDIO_BUFFERS * sizeof( audioBuffer_t ) );
    for ( i = 0; i < NUM_AUDIO_BUFFERS; i++ ) {
        D_80064594[i].ptr = Snd_Alloc( bufSamples * 2 * sizeof( s16 ) );
    }

    D_80064590 = Snd_Alloc( AUDIO_STACK_SIZE );
    osCreateThread( &D_800643E0, AUDIO_THREAD_ID, func_8003FAD8, NULL, D_80064590 + AUDIO_STACK_SIZE,
                    config->voicePriority );
    osStartThread( &D_800643E0 );
}

/*
================
func_8003FAD8

The audio thread. NOTE: cmdLen is read before the first frame sets it.
================
*/
void func_8003FAD8( void *arg ) {
    audioTask_t task;
    s32 cmdLen;
    u32 frame;
    u32 status;
    u32 queued;
    audioBuffer_t *buf;
    Acmd *cmdEnd;

    task.cmdList = D_80064598;
    task.ucode = (u64 *)aspMainTextStart;
    task.ucodeData = (u64 *)aspMainDataStart;
    frame = 0;
    D_8005CE4C->init();
    for ( ;; ) {
        do {
            D_8005CE4C->waitFrame();
            status = osAiGetStatus();
            queued = osAiGetLength() / 4;
        } while ( status & AI_STATUS_FIFO_FULL );

        func_8003F514();
        if ( D_8005CE50 != NULL && cmdLen != 0 ) {
            osAiSetNextBuffer( D_8005CE50->ptr, D_8005CE50->samples * 2 * sizeof( s16 ) );
        }

        buf = &D_80064594[frame];
        buf->samples = Snd_NextFrameSize( queued );
        cmdEnd = func_80042A00( D_80064598, &cmdLen, (s16 *)osVirtualToPhysical( buf->ptr ), buf->samples );
        if ( cmdLen != 0 ) {
            task.cmdSize = ( cmdEnd - D_80064598 ) * sizeof( Acmd );
            D_8005CE4C->runTask( &task );
            D_8005CE50 = buf;
        }
        frame = ( frame + 1 ) % NUM_AUDIO_BUFFERS;
    }
}
