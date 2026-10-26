#ifndef BOOT_H
#define BOOT_H

/* boot.h -- boot, threads and the cartridge PI handle (units 1060 and 2DF0) */

#include "common.h"
#include "libultra.h"

/*
libultra's raw PI register access, which the SDK headers of 2.0K declare only in the library's internal
piint.h; prototypes as there.
*/
s32 __osEPiRawReadIo( OSPiHandle *pihandle, u32 devAddr, u32 *data );
s32 __osEPiRawWriteIo( OSPiHandle *pihandle, u32 devAddr, u32 data );

extern OSPiHandle *D_800ABD60;      /* cartridge ROM handle */
extern OSIoMesg D_80066B70;         /* the request of each ROM DMA */
extern OSMesgQueue D_800DF578;      /* where ROM DMAs report completion */

#endif
