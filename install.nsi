; The name of the installer
Name "msdptool"

!include Library.nsh

;--------------------------------
;Version Information

!define /date TIMESTAMP "%Y.%m.%d"
!define VERSION 0.${TIMESTAMP}

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"

VIProductVersion ${VERSION}
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "MSDPtool"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "Manson SDP power supply tools"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Jiri Pinkava"
;VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "Test Application is a trademark of Fake company"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Jiri Pinkava"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "MSDPtool installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "{__TIMESTAMP__}"

; The file to write
OutFile "msdptool-${VERSION}.exe"

; The default installation directory
InstallDir $PROGRAMFILES\msdptool

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\msdptool" "Install_Dir"

LicenseText "This program is a free and Open Source software."
LicenseData "LGPL.txt"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

; Pages

Page license
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

; The stuff to install
Section "msdptool (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR"
  
  ; Put file there
  File "src\msdptool.exe"
  !insertmacro InstallLib REGDLL $ALREADY_INSTALLED NOREBOOT_NOTPROTECTED "src\msdp2xxx.dll" "$INSTDIR\msdp2xxx.dll" $SYSDIR
  File "src\libmsdp2xxx.a"

  SetOutPath "$INSTDIR\include"
  File "src\include\msdp2xxx_base.h"
  File "src\include\msdp2xxx.h"
  File "src\include\msdp2xxx_low.h"

  SetOutPath "$INSTDIR\examples"
  File "examples\example_01.c"

  SetOutPath "$INSTDIR\doc"
  File /r "doc\html"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\msdptool "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\msdptool" "DisplayName" "MSDPtool"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\msdptool" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\msdptool" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\msdptool" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

;--------------------------------
; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\msdptool"
  DeleteRegKey HKLM SOFTWARE\msdptool

  ; Remove files and uninstaller
  Delete "$INSTDIR\examples\example_01.c"
  Delete "$INSTDIR\include\msdp2xxx_base.h"
  Delete "$INSTDIR\include\msdp2xxx.h"
  Delete "$INSTDIR\include\msdp2xxx_low.h"
  !insertmacro UnInstallLib REGDLL SHARED NOREBOOT_NOTPROTECTED $INSTDIR\msdp2xxx.dll
  Delete "$INSTDIR\libmsdp2xxx.a"
  Delete "$INSTDIR\msdptool.exe"
  Delete "$INSTDIR\uninstall.exe"

  ; Remove directories used
  RMDir "$INSTDIR\doc"
  RMDir "$INSTDIR\examples"
  RMDir "$INSTDIR\include"
  RMDir "$INSTDIR"

SectionEnd
