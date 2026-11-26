/*
 * 1C260.c -- n_audio: a physical voice's ADPCM and raw decoder.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

INCLUDE_ASM("asm/nonmatchings/1C260", func_80040E60);

/*
================
func_800412A8

The decoder's parameter handler. N_FILTER_SET_WAVETABLE: plays the wave table param from its start, takes its
codebook size and its loop (an ADPCM table's length is cut to whole frames). N_FILTER_RESET: rewinds to the
table's start with the loop count restored.
================
*/
void func_800412A8( N_PVoice *pv, s32 paramID, void *param ) {
    switch ( paramID ) {
    case N_FILTER_SET_WAVETABLE:
        pv->table = (ALWaveTable *)param;
        pv->memin = (s32)pv->table->base;
        pv->sample = 0;
        switch ( pv->table->type ) {
        case AL_ADPCM_WAVE:
            pv->table->len = ( pv->table->len / N_ADPCM_FRAME_BYTES ) * N_ADPCM_FRAME_BYTES;
            pv->bookSize = 2 * pv->table->waveInfo.adpcmWave.book->order
                         * pv->table->waveInfo.adpcmWave.book->npredictors * ADPCMVSIZE;
            if ( pv->table->waveInfo.adpcmWave.loop != NULL ) {
                pv->loopStart = pv->table->waveInfo.adpcmWave.loop->start;
                pv->loopEnd = pv->table->waveInfo.adpcmWave.loop->end;
                pv->loopCount = pv->table->waveInfo.adpcmWave.loop->count;
                alCopy( pv->table->waveInfo.adpcmWave.loop->state, pv->loopState, sizeof( ADPCM_STATE ) );
            } else {
                pv->loopStart = pv->loopEnd = pv->loopCount = 0;
            }
            break;

        case AL_RAW16_WAVE:
            if ( pv->table->waveInfo.rawWave.loop != NULL ) {
                pv->loopStart = pv->table->waveInfo.rawWave.loop->start;
                pv->loopEnd = pv->table->waveInfo.rawWave.loop->end;
                pv->loopCount = pv->table->waveInfo.rawWave.loop->count;
            } else {
                pv->loopStart = pv->loopEnd = pv->loopCount = 0;
            }
            break;
        }
        break;

    case N_FILTER_RESET:
        pv->lastsam = 0;
        pv->first = TRUE;
        pv->sample = 0;
        if ( pv->table != NULL ) {
            pv->memin = (s32)pv->table->base;
            switch ( pv->table->type ) {
            case AL_ADPCM_WAVE:
                if ( pv->table->waveInfo.adpcmWave.loop != NULL ) {
                    pv->loopCount = pv->table->waveInfo.adpcmWave.loop->count;
                }
                break;

            case AL_RAW16_WAVE:
                if ( pv->table->waveInfo.rawWave.loop != NULL ) {
                    pv->loopCount = pv->table->waveInfo.rawWave.loop->count;
                }
                break;
            }
        }
        break;
    }
}

/*
================
func_80041434

Adds the commands that decode tsam samples to DMEM outp: loads nbytes of the table from memin to DMEM inp
(rounded to whole 8-byte words from the aligned address the DMA returns), the loop state if flags asks for
N_ADPCM_LOOP, and the decode itself. Returns the end of the commands.
================
*/
Acmd *func_80041434( Acmd *ptr, N_PVoice *pv, s32 tsam, s32 nbytes, s16 outp, s16 inp, s32 flags ) {
    s32 addr;
    s32 overFlow;

    if ( nbytes > 0 ) {
        addr = pv->dma( pv->memin, nbytes, pv->dmaState );
        overFlow = addr & 7;
        nbytes += overFlow;
        n_aLoadBuffer( ptr++, nbytes + 8 - ( nbytes & 7 ), inp, addr - overFlow );
    } else {
        overFlow = 0;
    }

    if ( flags & N_ADPCM_LOOP ) {
        aSetLoop( ptr++, K0_TO_PHYS( pv->loopState ) );
    }

    n_aADPCMdec( ptr++, pv->decState, flags, tsam << 1, overFlow, outp );
    pv->first = FALSE;
    return ptr;
}
