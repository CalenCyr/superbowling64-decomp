/*
 * 1C260.c -- n_audio: a physical voice's ADPCM and raw decoder.
 */

#include "common.h"
#include "libultra.h"
#include "functions.h"
#include "variables.h"

/*
================
Synth_DecodeSamples

Adds the commands that decode outCount samples of the table to DMEM *outp, and advances *outp past the part
of the first ADPCM frame already used. A loop the samples reach is decoded pass by pass from its start (each
pass moved to follow the last); samples past the table's end are cleared.
================
*/
#ifdef NON_MATCHING
// NON_MATCHING: registers only (frame, schedule and the loop's (s16)inp now match): retail gives outp s5, frameSam
// s1 and nOver s3; here outp s3, frameSam s5, nOver s1 (global alloc takes nOver before op; retail op first, as
// when op is reused for the clear offset, which R30 rules out). `s16 inp` keeps the loop's sll/sra and gives
// retail's 0x58 frame; the clear offset as its own local gives retail's a2. Not it: the clear local's declaration
// position (any) or block scope, clear = 0 before an if, !decoded first, clear << 1 at the use, a ?: (duplicates
// the aClearBuffer), frameSam set before overFlow or after its clamp, no frameSam local. Permuter (30 min): only
// a working copy of pv taken after the early return, and it does not fix these registers.
// Earlier: with s32 inp combine folds (s16)inp to 0 in all three calls (get_last_value, inp set once).
Acmd *Synth_DecodeSamples( N_PVoice *pv, s16 *outp, s32 outCount, Acmd *p ) {
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
    s32 frameSam;                       /* the samples in the whole frames */
    s32 decoded = FALSE;
    s16 inp;
    s32 clear;

    if ( outCount == 0 ) {
        return ptr;
    }

    aLoadADPCM( ptr++, pv->bookSize, K0_TO_PHYS( pv->table->waveInfo.adpcmWave.book->book ) );

    inp = 0;
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
        ptr = Synth_DecodeAdpcm( ptr, pv, tsam, nbytes, *outp, inp, pv->first );
        if ( pv->lastsam ) {
            *outp += pv->lastsam << 1;
        } else {
            *outp += 32;
        }

        pv->lastsam = pv->loopStart & 0xF;
        pv->memin = (s32)pv->table->base + N_ADPCM_FRAME_BYTES * ( ( pv->loopStart >> 4 ) + 1 );
        pv->sample = pv->loopStart;

        op = *outp;
        while ( nSam < outCount ) {
            dmemOut = ( op + ( nframes + 1 ) * 32 + 16 ) & ~0x1F;
            op += nSam << 1;
            outCount -= nSam;
            if ( pv->loopCount != ~0 && pv->loopCount != 0 ) {
                pv->loopCount--;
            }

            nSam = MIN( outCount, pv->loopEnd - pv->loopStart );
            tsam = nSam - 16 + pv->lastsam;
            if ( tsam < 0 ) {
                tsam = 0;
            }
            nframes = ( tsam + 15 ) >> 4;
            nbytes = nframes * N_ADPCM_FRAME_BYTES;
            ptr = Synth_DecodeAdpcm( ptr, pv, tsam, nbytes, dmemOut, inp, pv->first | N_ADPCM_LOOP );
            aDMEMMove( ptr++, dmemOut + ( pv->lastsam << 1 ), op, nSam << 1 );
        }

        pv->lastsam = ( outCount + pv->lastsam ) & 0xF;
        pv->sample += outCount;
        pv->memin += nframes * N_ADPCM_FRAME_BYTES;
        return ptr;
    }

    overFlow = pv->memin + nbytes - ( (s32)pv->table->base + pv->table->len );
    if ( overFlow < 0 ) {
        overFlow = 0;
    }
    nOver = ( overFlow / N_ADPCM_FRAME_BYTES ) << 4;
    frameSam = nframes * 16;
    if ( frameSam + lead < nOver ) {
        nOver = frameSam + lead;
    }
    nbytes -= overFlow;

    if ( nOver - ( nOver & 0xF ) < outCount ) {
        decoded = TRUE;
        ptr = Synth_DecodeAdpcm( ptr, pv, frameSam - nOver, nbytes, *outp, inp, pv->first );
        if ( pv->lastsam ) {
            *outp += pv->lastsam << 1;
        } else {
            *outp += 32;
        }
        pv->lastsam = ( outCount + pv->lastsam ) & 0xF;
        pv->sample += outCount;
        pv->memin += nframes * N_ADPCM_FRAME_BYTES;
    } else {
        pv->lastsam = 0;
        pv->memin += nframes * N_ADPCM_FRAME_BYTES;
    }

    if ( nOver != 0 ) {
        pv->lastsam = 0;
        if ( decoded ) {
            clear = ( lead + frameSam - nOver ) << 1;
        } else {
            clear = 0;
        }
        aClearBuffer( ptr++, clear + *outp, nOver << 1 );
    }
    return ptr;
}
#else
INCLUDE_ASM("asm/nonmatchings/1C260", Synth_DecodeSamples);
#endif

/*
================
Synth_DecoderParam

The decoder's parameter handler. N_FILTER_SET_WAVETABLE: plays the wave table param from its start, takes its
codebook size and its loop (an ADPCM table's length is cut to whole frames). N_FILTER_RESET: rewinds to the
table's start with the loop count restored.
================
*/
void Synth_DecoderParam( N_PVoice *pv, s32 paramID, void *param ) {
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
Synth_DecodeAdpcm

Adds the commands that decode tsam samples to DMEM outp: loads nbytes of the table from memin to DMEM inp
(rounded to whole 8-byte words from the aligned address the DMA returns), the loop state if flags asks for
N_ADPCM_LOOP, and the decode itself. Returns the end of the commands.
================
*/
Acmd *Synth_DecodeAdpcm( Acmd *ptr, N_PVoice *pv, s32 tsam, s32 nbytes, s16 outp, s16 inp, s32 flags ) {
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
