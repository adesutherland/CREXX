!ifndef LLAMA_STAGE
  !error "LLAMA_STAGE must contain verified installer inputs."
!endif
!ifndef LLAMA_OUT
  !error "LLAMA_OUT is required."
!endif
!ifndef LLAMA_BACKEND
  !error "LLAMA_BACKEND is required."
!endif
!ifndef LLAMA_VERSION
  !define LLAMA_VERSION "dev"
!endif

Unicode true
SetCompressor /SOLID lzma
RequestExecutionLevel admin
ManifestDPIAware true
!ifdef CREXX_SIGN_HELPER
  !system '"${CREXX_SIGN_HELPER}" --nsis-plugins "${NSISDIR}/Plugins/x86-unicode" "${CREXX_SIGNED_PLUGIN_DIR}"' = 0
  !addplugindir /x86-unicode "${CREXX_SIGNED_PLUGIN_DIR}"
  !uninstfinalize '"${CREXX_SIGN_HELPER}" "%1"' = 0
!endif
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "Sections.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"

Name "llama.rexx ${LLAMA_BACKEND} ${LLAMA_VERSION}"
OutFile "${LLAMA_OUT}"
; Leave empty until .onInit so a command-line /D= override is distinguishable.
; NSIS removes /D= from $CMDLINE before callbacks run.
InstallDir ""
BrandingText "cREXX optional native inference"
Var RegistrationId
Var InstallerMutex
Var ActivateArgument
!define UNINSTALL_ROOT "Software\Microsoft\Windows\CurrentVersion\Uninstall"
!define MUI_WELCOMEPAGE_TEXT "Add the ${LLAMA_BACKEND} llama.rexx plugin to an existing matching cREXX installation.$\r$\n$\r$\nModels are downloaded separately. Other installed backends and your cREXX core are preserved."
!insertmacro MUI_PAGE_WELCOME
!define MUI_DIRECTORYPAGE_TEXT_TOP "Select the existing cREXX installation. Its version and files must match this plugin."
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE ValidateCore
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_TEXT "The backend is installed. Use crexx-llama status to see the active backend.$\r$\n$\r$\nTo switch, close programs using llama.rexx and run crexx-llama use ${LLAMA_BACKEND} in an administrator terminal."
!insertmacro MUI_PAGE_FINISH
!define MUI_UNCONFIRMPAGE_TEXT_TOP "Remove this ${LLAMA_BACKEND} plugin. Your cREXX core, other installed backends and models are retained. If this is the active backend, no other backend will be activated automatically."
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

!macro InitInstaller
  FileOpen $2 "$TEMP\crexx-llama-${LLAMA_BACKEND}-installer.log" a
  FileSeek $2 0 END
  FileWrite $2 "Initialize: $EXEPATH | Core: $INSTDIR$\r$\n"
  FileClose $2
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "This plugin requires 64-bit Windows." /SD IDOK
    SetErrorLevel 1
    Abort
  ${EndIf}
  SetRegView 64
  System::Call 'kernel32::CreateMutexW(p 0, i 0, w "Global\CREXX-llama-setup") p.r1 ?e'
  Pop $0
  StrCpy $InstallerMutex $1
  ${If} $0 == 183
    FileOpen $2 "$TEMP\crexx-llama-${LLAMA_BACKEND}-installer.log" a
    FileSeek $2 0 END
    FileWrite $2 "Another installer owns the mutex.$\r$\n"
    FileClose $2
    MessageBox MB_ICONSTOP "Another llama.rexx installer is running." /SD IDOK
    SetErrorLevel 1
    Abort
  ${EndIf}
  InitPluginsDir
  SetOutPath "$PLUGINSDIR"
  File /oname=installer.json "${LLAMA_STAGE}\installer.json"
  File /r "${LLAMA_STAGE}\tool"
!macroend

Function .onInit
  !insertmacro InitInstaller
  ; InstallDirRegKey ignores SetRegView. A /D= override has already populated
  ; $INSTDIR and been removed from $CMDLINE by NSIS itself.
  ${If} $INSTDIR == ""
    ReadRegStr $INSTDIR HKLM "Software\CREXX\CREXX" "InstallDir"
    ${If} $INSTDIR == ""
      StrCpy $INSTDIR "$PROGRAMFILES64\CREXX"
    ${EndIf}
  ${EndIf}
FunctionEnd
Function un.onInit
  !insertmacro InitInstaller
  ; The uninstaller lives under <core>/.llama-backends/<backend>.
  ${GetParent} "$INSTDIR" $INSTDIR
  ${GetParent} "$INSTDIR" $INSTDIR
FunctionEnd

!macro Manage ACTION EXTRA
  nsExec::ExecToStack '"$PLUGINSDIR\tool\bin\crexx-llama.exe" ${ACTION} ${LLAMA_BACKEND} --root "$INSTDIR" --stage "$PLUGINSDIR" ${EXTRA}'
  Pop $0
  Pop $1
  DetailPrint "$1"
  FileOpen $2 "$TEMP\crexx-llama-${LLAMA_BACKEND}-installer.log" a
  FileSeek $2 0 END
  FileWrite $2 "Action: ${ACTION}$\r$\nCore: $INSTDIR$\r$\nExit: $0$\r$\n$1$\r$\n"
  FileClose $2
  ${If} $0 != 0
    MessageBox MB_ICONSTOP "llama.rexx could not complete ${ACTION}:$\r$\n$1$\r$\nDetails: $TEMP\crexx-llama-${LLAMA_BACKEND}-installer.log" /SD IDOK
    SetErrorLevel 1
    Abort
  ${EndIf}
!macroend

Function ValidateCore
  !insertmacro Manage check-core ""
FunctionEnd

Section "Install ${LLAMA_BACKEND} backend (required)" SecInstall
  SectionIn RO
  Call ValidateCore
  SetOutPath "$PLUGINSDIR"
  File /r "${LLAMA_STAGE}\payload"
  Call InstallPlugin
SectionEnd
Section /o "Make ${LLAMA_BACKEND} the active backend" SecActivate
  ; Evaluated by InstallPlugin in the same transaction as installation.
SectionEnd

Function InstallPlugin
  StrCpy $ActivateArgument ""
  ${If} ${SectionIsSelected} ${SecActivate}
    StrCpy $ActivateArgument "--activate"
  ${EndIf}
  !insertmacro Manage install "$ActivateArgument"
  FileOpen $0 "$INSTDIR\.llama-installer\registration-id" r
  FileRead $0 $RegistrationId
  FileClose $0
  WriteUninstaller "$INSTDIR\.llama-backends\${LLAMA_BACKEND}\uninstall.exe"
  WriteRegStr HKLM "${UNINSTALL_ROOT}\CREXX.llama.$RegistrationId.${LLAMA_BACKEND}" "DisplayName" "llama.rexx ${LLAMA_BACKEND} for cREXX"
  WriteRegStr HKLM "${UNINSTALL_ROOT}\CREXX.llama.$RegistrationId.${LLAMA_BACKEND}" "DisplayVersion" "${LLAMA_VERSION}"
  WriteRegStr HKLM "${UNINSTALL_ROOT}\CREXX.llama.$RegistrationId.${LLAMA_BACKEND}" "Publisher" "CREXX"
  WriteRegStr HKLM "${UNINSTALL_ROOT}\CREXX.llama.$RegistrationId.${LLAMA_BACKEND}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${UNINSTALL_ROOT}\CREXX.llama.$RegistrationId.${LLAMA_BACKEND}" "UninstallString" '"$INSTDIR\.llama-backends\${LLAMA_BACKEND}\uninstall.exe"'
  WriteRegStr HKLM "${UNINSTALL_ROOT}\CREXX.llama.$RegistrationId.${LLAMA_BACKEND}" "QuietUninstallString" '"$INSTDIR\.llama-backends\${LLAMA_BACKEND}\uninstall.exe" /S'
  WriteRegDWORD HKLM "${UNINSTALL_ROOT}\CREXX.llama.$RegistrationId.${LLAMA_BACKEND}" "NoModify" 1
  WriteRegDWORD HKLM "${UNINSTALL_ROOT}\CREXX.llama.$RegistrationId.${LLAMA_BACKEND}" "NoRepair" 1
FunctionEnd

Section "Uninstall"
  FileOpen $0 "$INSTDIR\.llama-installer\registration-id" r
  FileRead $0 $RegistrationId
  FileClose $0
  !insertmacro Manage remove ""
  DeleteRegKey HKLM "${UNINSTALL_ROOT}\CREXX.llama.$RegistrationId.${LLAMA_BACKEND}"
SectionEnd
