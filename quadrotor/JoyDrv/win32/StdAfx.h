/*
THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. NEITHER RECIPIENT NOR
ANY CONTRIBUTORS SHALL HAVE ANY LIABILITY FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING WITHOUT
LIMITATION LOST PROFITS), HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OR DISTRIBUTION OF THE PROGRAM
OR THE EXERCISE OF ANY RIGHTS GRANTED HEREUNDER, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES.

The name of the Copyright Holder may not be used to endorse or promote
products derived from this software without specific prior written permission.

Copyright 2000 George Rhoten and others.

*/

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__976098B9_B107_43EA_A285_252F2021D51A__INCLUDED_)
#define AFX_STDAFX_H__976098B9_B107_43EA_A285_252F2021D51A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
//#define WIN32_LEAN_AND_MEAN       // Exclude rarely-used stuff from Windows headers
/*  If defined, the following flags inhibit definition
 *     of the indicated items.
 *
 *  NOGDICAPMASKS     - CC_*, LC_*, PC_*, CP_*, TC_*, RC_
 *  NOVIRTUALKEYCODES - VK_*
 *  NOWINMESSAGES     - WM_*, EM_*, LB_*, CB_*
 *  NOWINSTYLES       - WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
 *  NOSYSMETRICS      - SM_*
 *  NOMENUS           - MF_*
 *  NOICONS           - IDI_*
 *  NOKEYSTATES       - MK_*
 *  NOSYSCOMMANDS     - SC_*
 *  NORASTEROPS       - Binary and Tertiary raster ops
 *  NOSHOWWINDOW      - SW_*
 *  OEMRESOURCE       - OEM Resource values
 *  NOATOM            - Atom Manager routines
 *  NOCLIPBOARD       - Clipboard routines
 *  NOCOLOR           - Screen colors
 *  NOCTLMGR          - Control and Dialog routines
 *  NODRAWTEXT        - DrawText() and DT_*
 *  NOGDI             - All GDI defines and routines
 *  NOKERNEL          - All KERNEL defines and routines
 *  NOUSER            - All USER defines and routines
 *  NONLS             - All NLS defines and routines
 *  NOMB              - MB_* and MessageBox()
 *  NOMEMMGR          - GMEM_*, LMEM_*, GHND, LHND, associated routines
 *  NOMETAFILE        - typedef METAFILEPICT
 *  NOMINMAX          - Macros min(a,b) and max(a,b)
 *  NOMSG             - typedef MSG and associated routines
 *  NOOPENFILE        - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
 *  NOSCROLL          - SB_* and scrolling routines
 *  NOSERVICE         - All Service Controller routines, SERVICE_ equates, etc.
 *  NOSOUND           - Sound driver routines
 *  NOTEXTMETRIC      - typedef TEXTMETRIC and associated routines
 *  NOWH              - SetWindowsHook and WH_*
 *  NOWINOFFSETS      - GWL_*, GCL_*, associated routines
 *  NOCOMM            - COMM driver routines
 *  NOKANJI           - Kanji support stuff.
 *  NOHELP            - Help engine interface.
 *  NOPROFILER        - Profiler interface.
 *  NODEFERWINDOWPOS  - DeferWindowPos routines
 *  NOMCX             - Modem Configuration Extensions
 */

#ifdef WIN32

#define NOGDICAPMASKS     //- CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES //- VK_*
#define NOWINMESSAGES     //- WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES       //- WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS      //- SM_*
#define NOMENUS           //- MF_*
#define NOICONS           //- IDI_*
#define NOKEYSTATES       //- MK_*
#define NOSYSCOMMANDS     //- SC_*
#define NORASTEROPS       //- Binary and Tertiary raster ops
#define NOSHOWWINDOW      //- SW_*
#define OEMRESOURCE       //- OEM Resource values
#define NOATOM            //- Atom Manager routines
#define NOCLIPBOARD       //- Clipboard routines
#define NOCOLOR           //- Screen colors
#define NOCTLMGR          //- Control and Dialog routines
#define NODRAWTEXT        //- DrawText() and DT_*
#define NOGDI             //- All GDI defines and routines
#define NOKERNEL          //- All KERNEL defines and routines
#define NOUSER            //- All USER defines and routines
#define NONLS             //- All NLS defines and routines
#define NOMB              //- MB_* and MessageBox()
#define NOMEMMGR          //- GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE        //- typedef METAFILEPICT
#define NOMINMAX          //- Macros min(a,b) and max(a,b)
#define NOMSG             //- typedef MSG and associated routines
#define NOOPENFILE        //- OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL          //- SB_* and scrolling routines
#define NOSERVICE         //- All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND           //- Sound driver routines
#define NOTEXTMETRIC      //- typedef TEXTMETRIC and associated routines
#define NOWH              //- SetWindowsHook and WH_*
#define NOWINOFFSETS      //- GWL_*, GCL_*, associated routines
#define NOCOMM            //- COMM driver routines
#define NOKANJI           //- Kanji support stuff.
#define NOHELP            //- Help engine interface.
#define NOPROFILER        //- Profiler interface.
#define NODEFERWINDOWPOS  //- DeferWindowPos routines
#define NOMCX             //- Modem Configuration Extensions

#define MMNODRV         //Installable driver support
#define MMNOSOUND       //Sound support
#define MMNOWAVE        //Waveform support
#define MMNOMIDI        //MIDI support
#define MMNOAUX         //Auxiliary audio support
#define MMNOMIXER       //Mixer support
#define MMNOTIMER       //Timer support
//#define MMNOJOY         //Joystick support
#define MMNOMCI         //MCI support
#define MMNOMMIO        //Multimedia file I/O support
#define MMNOMMSYSTEM    //General MMSYSTEM functions

#define NOWINRES

#include <windows.h>

//This is half of the maximum value for any given axis.
#define HALF_AXIS   ((jfloat)32767.5f)

#endif  // WIN32

#include <jni.h>

extern const jint numDevices;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__976098B9_B107_43EA_A285_252F2021D51A__INCLUDED_)
