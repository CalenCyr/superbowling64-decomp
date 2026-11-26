/*
 * 1C260.c -- n_audio: a physical voice's ADPCM and raw decoder.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
func_80040E60

Adds the commands that decode outCount samples of the table to DMEM *outp, and advances *outp past the part
of the first ADPCM frame already used. A loop the samples reach is decoded pass by pass from its start (each
pass moved to follow the last); samples past the table's end are cleared.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: retail's frame is 16 bytes larger (0x20-0x2F unused) and keeps lead, nOver and decoded in
// three more saved registers (s6, s3, fp); the logic and the command words match
Acmd *func_80040E60( N_PVoice *pv, s16 *outp, s32 outCount, Acmd *p ) {
    Acmd *ptr = p;
    s32 looped;
    s32 nSam;
    s32 lead;
    s32 tsam;
    s32 nframes;
    s32 nbytes;
    s32 op;
    s32 dmemOut;
    s32 overFlow;
    s32 nOver;
    s32 decoded = FALSE;
    s32 inp = 0;

    if ( outCount == 0 ) {
        return ptr;
    }

    aLoadADPCM( ptr++, pv->bookSize, K0_TO_PHYS( pv->table->waveInfo.adpcmWave.book->book ) );

    looped = pv->loopEnd < outCount + pv->sample && pv->loopCount != 0;
    if ( looped ) {
        nSam = pv->loopEnd - pv->sample;
    } else {
        nSam = outCount;
    }

    if ( pv->lastsam ) {
        lead = 16 - pv->lastsam;
    } else {
        lead = 0;
    }

    tsam = nSam - lead;
    if ( tsam < 0 ) {
        tsam = 0;
    }
    nframes = ( tsam + 15 ) >> 4;
    nbytes = nframes * N_ADPCM_FRAME_BYTES;

    if ( looped ) {
        ptr = func_80041434( ptr, pv, tsam, nbytes, *outp, inp, pv->first );
        if ( pv->lastsam ) {
            *outp += pv->lastsam << 1;
        } else {
            *outp += 32;
        }

        pv->lastsam = pv->loopStart & 0xF;
        pv->sample = pv->loopStart;
        pv->memin = (s32)pv->table->base + N_ADPCM_FRAME_BYTES * ( ( pv->loopStart >> 4 ) + 1 );

        op = *outp;
        while ( nSam < outCount ) {
            dmemOut = ( op + ( nframes + 1 ) * 32 + 16 ) & ~0x1F;
            op += nSam << 1;
            outCount -= nSam;
            if ( pv->loopCount != ~0 && pv->loopCount != 0 ) {
                pv->loopCount--;
            }

            nSam = pv->loopEnd - pv->loopStart;
            if ( outCount < nSam ) {
                nSam = outCount;
            }
            tsam = nSam - ( 16 - pv->lastsam );
            if ( tsam < 0 ) {
                tsam = 0;
            }
            nframes = ( tsam + 15 ) >> 4;
            nbytes = nframes * N_ADPCM_FRAME_BYTES;
            ptr = func_80041434( ptr, pv, tsam, nbytes, dmemOut, inp, pv->first | N_ADPCM_LOOP );
            aDMEMMove( ptr++, dmemOut + ( pv->lastsam << 1 ), op, nSam << 1 );
        }

        pv->sample += outCount;
        pv->lastsam = ( outCount + pv->lastsam ) & 0xF;
        pv->memin += nframes * N_ADPCM_FRAME_BYTES;
    } else {
        overFlow = pv->memin + nbytes - ( (s32)pv->table->base + pv->table->len );
        if ( overFlow < 0 ) {
            overFlow = 0;
        }
        nOver = ( overFlow / N_ADPCM_FRAME_BYTES ) << 4;
        if ( nframes * 16 + lead < nOver ) {
            nOver = nframes * 16 + lead;
        }
        nbytes -= overFlow;

        if ( nOver - ( nOver & 0xF ) < outCount ) {
            decoded = TRUE;
            ptr = func_80041434( ptr, pv, nframes * 16 - nOver, nbytes, *outp, inp, pv->first );
            if ( pv->lastsam ) {
                *outp += pv->lastsam << 1;
            } else {
                *outp += 32;
            }
            pv->sample += outCount;
            pv->lastsam = ( outCount + pv->lastsam ) & 0xF;
            pv->memin += nframes * N_ADPCM_FRAME_BYTES;
        } else {
            pv->lastsam = 0;
            pv->memin += nframes * N_ADPCM_FRAME_BYTES;
        }

        if ( nOver != 0 ) {
            pv->lastsam = 0;
            if ( decoded ) {
                op = ( lead + nframes * 16 - nOver ) << 1;
            } else {
                op = 0;
            }
            aClearBuffer( ptr++, op + *outp, nOver << 1 );
        }
    }
    return ptr;
}
#else
INCLUDE_ASM("asm/nonmatchings/1C260", func_80040E60);
#endif

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
