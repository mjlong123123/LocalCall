# Microsoft Developer Studio Project File - Name="jvoiplib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=jvoiplib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jvoiplib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "jvoiplib.mak" CFG="jvoiplib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "jvoiplib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "jvoiplib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jvoiplib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "./src" /I "./src/componentframework" /I "./src/libcomponents" /I "./src/libcomponents/base" /I "./src/libcomponents/impl" /I "./src/libcomponents/impl/win32" /I "./src/thirdparty/gsm/inc" /I "./src/thirdparty/lpc" /I "..\jrtplib-3.3.0\src" /I "..\jthread-1.1.2\src" /I "..\speex-1.0.5\include\speex" /I "..\libsndfile-1.0.11\Win32" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "SASR" /D "USE_FLOAT_MUL" /D "FAST" /D NeedFunctionPrototypes=1 /D "_AFXDLL" /YX /FD /c
# ADD BASE RSC /l 0x813 /d "NDEBUG"
# ADD RSC /l 0x813 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copy header files
PostBuild_Cmds=copyheaderfiles
# End Special Build Tool

!ELSEIF  "$(CFG)" == "jvoiplib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "./src" /I "./src/componentframework" /I "./src/libcomponents" /I "./src/libcomponents/base" /I "./src/libcomponents/impl" /I "./src/libcomponents/impl/win32" /I "./src/thirdparty/gsm/inc" /I "./src/thirdparty/lpc" /I "..\jrtplib-3.3.0\src" /I "..\jthread-1.1.2\src" /I "..\speex-1.0.5\include\speex" /I "..\libsndfile-1.0.11\Win32" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "SASR" /D "USE_FLOAT_MUL" /D "FAST" /D NeedFunctionPrototypes=1 /D "_AFXDLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x813 /d "_DEBUG"
# ADD RSC /l 0x813 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copy all header files
PostBuild_Cmds=copyheaderfiles
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "jvoiplib - Win32 Release"
# Name "jvoiplib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "componentframework_src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\componentframework\voiceblock.cpp
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\voicecall.cpp
# End Source File
# End Group
# Begin Group "libcomponents_src"

# PROP Default_Filter ""
# Begin Group "base_src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipcompression.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipsessionlocalisation.cpp
# End Source File
# End Group
# Begin Group "impl_src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipdpcmcompression.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipfileinput.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipfileoutput.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipgsmcompression.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoiphrtfdata.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoiphrtflocalisation.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoiplpccompression.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\win32\jvoipmixerstatus.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipnoinput.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipnooutput.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipnormalmixer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoiprtptransmission.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipsampleconverter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipsilencesuppression.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipsimplelocalisation.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipsimpletimer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\win32\jvoipsoundcardinput.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\win32\jvoipsoundcardoutput.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipspeexcompression.cpp
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipulawcompression.cpp
# End Source File
# End Group
# End Group
# Begin Group "thirdparty_src"

# PROP Default_Filter ""
# Begin Group "gsm_src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_add.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_code.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_create.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_debug.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_decode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_decode2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_destroy.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_encode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_explode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_implode.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_long_term.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_lpc.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_option.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_preprocess.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_print.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_rpe.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_short_term.cpp
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\src\gsm_table.cpp
# End Source File
# End Group
# Begin Group "lpc_src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\thirdparty\lpc\lpccodec.cpp
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\src\debugnew.cpp
# End Source File
# Begin Source File

SOURCE=.\src\jvoiperrors.cpp
# End Source File
# Begin Source File

SOURCE=.\src\jvoipsession.cpp
# End Source File
# Begin Source File

SOURCE=.\src\jvoipsessionparams.cpp
# End Source File
# Begin Source File

SOURCE=.\src\jvoipsigwait.cpp
# End Source File
# Begin Source File

SOURCE=.\src\jvoipthread.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "componentframework_hdr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\componentframework\location3d.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\sampleinput.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\sampleoutput.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\samplingtimer.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\transform3d.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\voiceblock.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\voicecall.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\voicecompressor.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\voicedecompressor.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\voicemixer.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\voicetransmitter.h
# End Source File
# Begin Source File

SOURCE=.\src\componentframework\voicetypes.h
# End Source File
# End Group
# Begin Group "libcomponents_hdr"

# PROP Default_Filter ""
# Begin Group "base_hdr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipcomponent.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipcomponentparams.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipcompression.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipcompressionmodule.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoiplocalisation.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipmixer.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipsamplingtimer.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipsessionlocalisation.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoiptransmission.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipvoiceinput.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\base\jvoipvoiceoutput.h
# End Source File
# End Group
# Begin Group "impl_hdr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipdpcmcompression.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipfileinput.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipfileoutput.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipgsmcompression.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoiphrtflocalisation.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoiplpccompression.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\win32\jvoipmixerstatus.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipnoinput.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipnooutput.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipnormalmixer.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoiprtptransmission.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipsampleconverter.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipsilencesuppression.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipsimplelocalisation.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipsimpletimer.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\win32\jvoipsoundcardinput.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\win32\jvoipsoundcardoutput.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipspeexcompression.h
# End Source File
# Begin Source File

SOURCE=.\src\libcomponents\impl\jvoipulawcompression.h
# End Source File
# End Group
# End Group
# Begin Group "thirdparty_hdr"

# PROP Default_Filter ""
# Begin Group "gsm_hdr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\thirdparty\gsm\inc\config.h
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\inc\gsm.h
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\inc\private.h
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\inc\proto.h
# End Source File
# Begin Source File

SOURCE=.\src\thirdparty\gsm\inc\unproto.h
# End Source File
# End Group
# Begin Group "lpc_hdr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\thirdparty\lpc\lpccodec.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\src\debugnew.h
# End Source File
# Begin Source File

SOURCE=.\src\jvoipconfig.h
# End Source File
# Begin Source File

SOURCE=.\src\jvoipconfig_win.h
# End Source File
# Begin Source File

SOURCE=.\src\jvoiperrors.h
# End Source File
# Begin Source File

SOURCE=.\src\jvoipexception.h
# End Source File
# Begin Source File

SOURCE=.\src\jvoipsession.h
# End Source File
# Begin Source File

SOURCE=.\src\jvoipsessionparams.h
# End Source File
# Begin Source File

SOURCE=.\src\jvoipsigwait.h
# End Source File
# Begin Source File

SOURCE=.\src\jvoipthread.h
# End Source File
# Begin Source File

SOURCE=.\src\jvoiptypes.h
# End Source File
# End Group
# End Target
# End Project
