/*
 * 1ADA0.c -- the audio thread: each frame it waits for the retrace, queues the last frame's samples on the AI,
 * has n_audio build the next frame's command list, and runs it on the RSP into the next output buffer.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/* ===== State ===== */

static OSThread audio_thread;       /* the audio thread */
static u8 *audio_stack;             /* its stack */
static audioBuffer_t *audio_buffers;  /* NUM_AUDIO_BUFFERS output buffers */
static Acmd *audio_cmdList;         /* the frame's command list */

/* ===== Data ===== */

/* the audio thread's routines, unless another table stands in */
static audioTaskFuncs_t audio_defaultTaskFuncs = { Audio_ClientInit, Audio_ClientWaitFrame, Audio_ClientRunTask };
audioTaskFuncs_t *audio_taskFuncs = &audio_defaultTaskFuncs;
static audioBuffer_t *audio_lastBuffer = NULL;  /* the buffer the last task filled, for the AI next; NULL: none yet */

#define AUDIO_THREAD_ID     3
#define FRAME_QUEUE_PERCENT 15      /* AI queue kept ahead, in percent of a frame (Snd_InitFrameSizes) */

/*
================
Audio_Init

Sets up the synthesizer, the ROM sample cache and the output buffers from config, ticking tickRate times a
second with effect fxType, and starts the audio thread.
================
*/
void Audio_Init( sndConfig_t *config, s32 tickRate, s32 fxType ) {
    ALSynConfig c;
    u32 bufSamples;
    u32 i;

    c.maxVVoices = c.maxPVoices = config->numVoices;
    c.maxUpdates = config->maxUpdates;
    c.dmaproc = Audio_InitDmaCache( config->numDmaBuffers, config->dmaBufferSize );
    c.fxType = fxType;
    c.outputRate = osAiSetFrequency( config->outputRate );
    c.heap = Snd_GetHeap();
    Synth_Init( &audio_globals, &c );

    bufSamples = Snd_InitFrameSizes( config->ticksPerFrame, c.outputRate, tickRate, FRAME_QUEUE_PERCENT );
    audio_cmdList = Snd_Alloc( config->maxCmds * sizeof( Acmd ) );
    audio_buffers = Snd_Alloc( NUM_AUDIO_BUFFERS * sizeof( audioBuffer_t ) );
    for ( i = 0; i < NUM_AUDIO_BUFFERS; i++ ) {
        audio_buffers[i].ptr = Snd_Alloc( bufSamples * 2 * sizeof( s16 ) );
    }

    audio_stack = Snd_Alloc( AUDIO_STACK_SIZE );
    osCreateThread( &audio_thread, AUDIO_THREAD_ID, Audio_Thread, NULL, audio_stack + AUDIO_STACK_SIZE,
                    config->voicePriority );
    osStartThread( &audio_thread );
}

/*
================
Audio_Thread

The audio thread. NOTE: cmdLen is read before the first frame sets it.
================
*/
void Audio_Thread( void *arg ) {
    audioTask_t task;
    s32 cmdLen;
    u32 frame;
    u32 status;
    u32 queued;
    audioBuffer_t *buf;
    Acmd *cmdEnd;

    task.cmdList = audio_cmdList;
    task.ucode = (u64 *)aspMainTextStart;
    task.ucodeData = (u64 *)aspMainDataStart;
    frame = 0;
    audio_taskFuncs->init();
    for ( ;; ) {
        do {
            audio_taskFuncs->waitFrame();
            status = osAiGetStatus();
            queued = osAiGetLength() / 4;
        } while ( status & AI_STATUS_FIFO_FULL );

        Audio_UpdateDmaCache();
        if ( audio_lastBuffer != NULL && cmdLen != 0 ) {
            osAiSetNextBuffer( audio_lastBuffer->ptr, audio_lastBuffer->samples * 2 * sizeof( s16 ) );
        }

        buf = &audio_buffers[frame];
        buf->samples = Snd_NextFrameSize( queued );
        cmdEnd = Synth_MakeFrame( audio_cmdList, &cmdLen, (s16 *)osVirtualToPhysical( buf->ptr ), buf->samples );
        if ( cmdLen != 0 ) {
            task.cmdSize = ( cmdEnd - audio_cmdList ) * sizeof( Acmd );
            audio_taskFuncs->runTask( &task );
            audio_lastBuffer = buf;
        }
        frame = ( frame + 1 ) % NUM_AUDIO_BUFFERS;
    }
}
