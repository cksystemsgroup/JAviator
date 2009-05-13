# Microsoft Developer Studio Project File - Name="joystick" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=joystick - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "joystick.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "joystick.mak" CFG="joystick - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "joystick - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "joystick - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "joystick - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JOYSTICK_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /O2 /I "C:\jdk1.3\include" /I "C:\jdk1.3\include\win32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JOYSTICK_EXPORTS" /D "_ATL_MIN_CRT" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /machine:I386
# ADD LINK32 winmm.lib /nologo /dll /machine:I386 /out:"..\lib\jjstick.dll"
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "joystick - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JOYSTICK_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "C:\jdk1.3\include" /I "C:\jdk1.3\include\win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JOYSTICK_EXPORTS" /D "_ATL_MIN_CRT" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib /nologo /dll /debug /machine:I386 /out:"..\lib\jjstick.dll" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "joystick - Win32 Release"
# Name "joystick - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\com_centralnexus_input_ExtendedJoystick.cpp
# End Source File
# Begin Source File

SOURCE=.\com_centralnexus_input_Joystick.cpp
# End Source File
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\com_centralnexus_input_ExtendedJoystick.h
# End Source File
# Begin Source File

SOURCE=.\com_centralnexus_input_Joystick.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\joystick.rc
# End Source File
# End Group
# Begin Group "Java Object Files"

# PROP Default_Filter "*.class"
# Begin Source File

SOURCE=..\com\centralnexus\input\ExtendedJoystick.class

!IF  "$(CFG)" == "joystick - Win32 Release"

# PROP Intermediate_Dir "."
# PROP Ignore_Default_Tool 1
USERDEP__EXTEN="..\com\centralnexus\input\ExtendedJoystick.java"    
# Begin Custom Build
InputPath=..\com\centralnexus\input\ExtendedJoystick.class
InputName=ExtendedJoystick

"com_centralnexus_input_$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javah -classpath .. -force com.centralnexus.input.$(InputName)

# End Custom Build

!ELSEIF  "$(CFG)" == "joystick - Win32 Debug"

# PROP Intermediate_Dir "."
# PROP Ignore_Default_Tool 1
USERDEP__EXTEN="..\com\centralnexus\input\ExtendedJoystick.java"    
# Begin Custom Build
InputPath=..\com\centralnexus\input\ExtendedJoystick.class
InputName=ExtendedJoystick

"com_centralnexus_input_$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javah -classpath .. -force com.centralnexus.input.$(InputName)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\com\centralnexus\input\Joystick.class

!IF  "$(CFG)" == "joystick - Win32 Release"

# PROP Intermediate_Dir "."
# PROP Ignore_Default_Tool 1
USERDEP__JOYST="..\com\centralnexus\input\Joystick.java"    
# Begin Custom Build
InputPath=..\com\centralnexus\input\Joystick.class
InputName=Joystick

"com_centralnexus_input_$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javah -classpath .. -force com.centralnexus.input.$(InputName)

# End Custom Build

!ELSEIF  "$(CFG)" == "joystick - Win32 Debug"

# PROP Intermediate_Dir "."
# PROP Ignore_Default_Tool 1
USERDEP__JOYST="..\com\centralnexus\input\Joystick.java"    
# Begin Custom Build
InputPath=..\com\centralnexus\input\Joystick.class
InputName=Joystick

"com_centralnexus_input_$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javah -classpath .. -force com.centralnexus.input.$(InputName)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Java Source Files"

# PROP Default_Filter "*.java"
# Begin Source File

SOURCE=..\com\centralnexus\input\ExtendedJoystick.java

!IF  "$(CFG)" == "joystick - Win32 Release"

# PROP Intermediate_Dir "."
# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=..\com\centralnexus\input\ExtendedJoystick.java
InputName=ExtendedJoystick

"..\com\centralnexus\input\$(InputName).class" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javac -O -classpath .. $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "joystick - Win32 Debug"

# PROP Intermediate_Dir "."
# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=..\com\centralnexus\input\ExtendedJoystick.java
InputName=ExtendedJoystick

"..\com\centralnexus\input\$(InputName).class" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javac -classpath .. $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\com\centralnexus\input\Joystick.java

!IF  "$(CFG)" == "joystick - Win32 Release"

# PROP Intermediate_Dir "."
# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=..\com\centralnexus\input\Joystick.java
InputName=Joystick

"..\com\centralnexus\input\$(InputName).class" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javac -O -classpath .. $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "joystick - Win32 Debug"

# PROP Intermediate_Dir "."
# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=..\com\centralnexus\input\Joystick.java
InputName=Joystick

"..\com\centralnexus\input\$(InputName).class" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javac -classpath .. $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\com\centralnexus\input\JoystickListener.java

!IF  "$(CFG)" == "joystick - Win32 Release"

# Begin Custom Build
InputPath=..\com\centralnexus\input\JoystickListener.java
InputName=JoystickListener

"..\com\centralnexus\input\$(InputName).class" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javac -O -classpath .. $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "joystick - Win32 Debug"

# Begin Custom Build
InputPath=..\com\centralnexus\input\JoystickListener.java
InputName=JoystickListener

"..\com\centralnexus\input\$(InputName).class" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javac -classpath .. $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\com\centralnexus\input\JoystickNotifier.java

!IF  "$(CFG)" == "joystick - Win32 Release"

# Begin Custom Build
InputPath=..\com\centralnexus\input\JoystickNotifier.java
InputName=JoystickNotifier

"..\com\centralnexus\input\$(InputName).class" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javac -O -classpath .. $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "joystick - Win32 Debug"

# Begin Custom Build
InputPath=..\com\centralnexus\input\JoystickNotifier.java
InputName=JoystickNotifier

"..\com\centralnexus\input\$(InputName).class" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
    c:\jdk1.3\bin\javac -classpath .. $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
