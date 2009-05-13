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

Copyright 2000-2002 George Rhoten and others.

*/

// Main.cpp : Defines the entry point for the DLL application.
//
// original author: George Rhoten
// original date: 5/26/2000

#include "stdafx.h"

static const char *copyright = "Original Author: George Rhoten, (c)2000-2002";

//const jint numDevices = MAX_JOYSTICK;
const jint numDevices = joyGetNumDevs();

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
/*    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
//        puts("\nattach");
        break;
    case DLL_PROCESS_DETACH:
//        puts("\ndetach");
        break;
    }*/
    return TRUE;
}
