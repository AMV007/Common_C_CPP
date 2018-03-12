/****************************************************************************
 *  @doc INTERNAL
 *  @module dlportio.h |
 *
 *  DriverLINX Port I/O Driver Interface
 *  <cp> Copyright 1996 Scientific Software Tools, Inc.<nl>
 *  All Rights Reserved.<nl>
 *  DriverLINX is a registered trademark of Scientific Software Tools, Inc.
 *
 *  Win32 Prototypes for DriverLINX Port I/O
 *
 *  Please report bugs to:
 *  Scientific Software Tools, Inc.
 *  19 East Central Avenue
 *  Paoli, PA 19301
 *  USA
 *  E-mail: support@sstnet.com
 *  Web: www.sstnet.com
 *
 *  @comm   
 *  Author: RoyF<nl>
 *  Date:   09/26/96 14:08:58
 *
 *  @group Revision History
 *  @comm
 *  $Revision: 1.3 $
 *  <nl>
 *  $Log: dlportio.h,v $
 *  Revision 1.3  2004/11/30 17:46:48  lancos
 *  Commit PonyProg2000 2.06e
 *
 *  Revision 1.2  2001/02/02 17:08:55  lanconel
 *  no change
 *
 *  Revision 1.1  2000/07/24 08:46:30  lanconel
 *  Apportate le modifiche fino alla versione 1.17e. Ora PonyProg200 e` aggiornato, manca solo da aggiornare il setup
 *
 *  Revision 1.2  2000/06/29 15:06:57  lancos
 *  Driver for direct port I/O header
 *
 *  Revision 1.1  2000/06/23 17:19:25  lancos
 *  Modified for DLPortIO driver
 *
 * 
 * 1     9/27/96 2:03p Royf
 * Initial revision.
 *
 ****************************************************************************/

#ifdef	_WINDOWS

#ifndef DLPORTIO_H
#define DLPORTIO_H

#pragma comment( lib, "Dlportio.lib" )

#ifdef __cplusplus
extern "C" {
#endif

#define DLPORT_API(type) __declspec(dllimport) type APIENTRY

DLPORT_API(BYTE) DlPortReadPortUchar(DWORD Port);
DLPORT_API(WORD) DlPortReadPortUshort(DWORD Port);
DLPORT_API(DWORD)DlPortReadPortUlong(DWORD Port);
DLPORT_API(VOID) DlPortReadPortBufferUchar(DWORD Port, BYTE * Buffer, DWORD  Count);
DLPORT_API(VOID) DlPortReadPortBufferUshort(DWORD Port, WORD * Buffer, DWORD Count);
DLPORT_API(VOID) DlPortReadPortBufferDWORD(DWORD Port, DWORD * Buffer, DWORD Count);
DLPORT_API(VOID) DlPortWritePortUchar(DWORD Port, BYTE Value);
DLPORT_API(VOID) DlPortWritePortUshort(DWORD Port, WORD Value);
DLPORT_API(VOID) DlPortWritePortUlong(DWORD Port, DWORD Value);
DLPORT_API(VOID) DlPortWritePortBufferUchar(DWORD Port, BYTE * Buffer, DWORD  Count);
DLPORT_API(VOID) DlPortWritePortBufferUshort(DWORD Port, WORD * Buffer, DWORD Count);
DLPORT_API(VOID) DlPortWritePortBufferDWORD(DWORD Port, DWORD * Buffer, DWORD Count);

#ifdef __cplusplus
}
#endif

#endif // DLPORTIO_H

#endif
