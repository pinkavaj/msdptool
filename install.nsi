; The name of the installer
Name "mscptool"


; The file to write
OutFile "mscptool_inst.exe"

; The default installation directory
InstallDir $PROGRAMFILES\mscptool

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\mscptool" "Install_Dir"

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
Section "mscptool (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$INSTDIR"
  
  ; Put file there
  File "src\msdptool.exe"
  File "src\msdp2xxx.dll"

  SetOutPath "$INSTDIR\include"
  File "src\include\msdp2xxx_base.h"
  File "src\include\msdp2xxx.h"
  File "src\include\msdp2xxx_low.h"

  SetOutPath "$INSTDIR\examples"
  File "examples\example_01.c"

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\mscptool "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\mscptool" "DisplayName" "MSCPtool"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\mscptool" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\mscptool" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\mscptool" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

;--------------------------------
; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\mscptool"
  DeleteRegKey HKLM SOFTWARE\mscptool

  ; Remove files and uninstaller
  Delete "$INSTDIR\examples\example_01.c"
  Delete "$INSTDIR\include\msdp2xxx_base.h"
  Delete "$INSTDIR\include\msdp2xxx.h"
  Delete "$INSTDIR\include\msdp2xxx_low.h"
  Delete "$INSTDIR\msdp2xxx.dll"
  Delete "$INSTDIR\msdptool.exe"
  Delete "$INSTDIR\uninstall.exe"

  ; Remove directories used
  RMDir "$INSTDIR\include"
  RMDir "$INSTDIR"

SectionEnd
