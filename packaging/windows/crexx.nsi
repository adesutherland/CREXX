!ifndef CREXX_PAYLOAD_DIR
  !error "Define CREXX_PAYLOAD_DIR with the staged CREXX Windows payload directory."
!endif

!ifndef CREXX_OUTFILE
  !define CREXX_OUTFILE "CREXX-windows-x64-setup.exe"
!endif

!ifndef CREXX_DISPLAY_VERSION
  !define CREXX_DISPLAY_VERSION "dev"
!endif

!ifndef CREXX_FILE_VERSION
  !define CREXX_FILE_VERSION "0.0.0.0"
!endif

Unicode true
SetCompressor /SOLID lzma
ManifestDPIAware true
RequestExecutionLevel admin

!ifdef CREXX_SIGN_HELPER
  ; Sign private copies of the embedded NSIS helper DLLs as well as the
  ; CREXX payload. Never modify the machine's installed NSIS plugin files.
  !system '"${CREXX_SIGN_HELPER}" --nsis-plugins "${NSISDIR}/Plugins/x86-unicode" "${CREXX_SIGNED_PLUGIN_DIR}"' = 0
  !addplugindir /x86-unicode "${CREXX_SIGNED_PLUGIN_DIR}"
!endif

!include "LogicLib.nsh"
!include "MUI2.nsh"
!include "WinMessages.nsh"
!include "x64.nsh"

!define CREXX_PRODUCT_NAME "CREXX"
!define CREXX_PUBLISHER "CREXX"
!define CREXX_REG_KEY "Software\CREXX\CREXX"
!define CREXX_UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\CREXX"
!define CREXX_ENV_KEY "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"

Name "${CREXX_PRODUCT_NAME} ${CREXX_DISPLAY_VERSION}"
OutFile "${CREXX_OUTFILE}"
Caption "${CREXX_PRODUCT_NAME} ${CREXX_DISPLAY_VERSION} Setup"
UninstallCaption "${CREXX_PRODUCT_NAME} ${CREXX_DISPLAY_VERSION} Uninstall"
BrandingText "${CREXX_PRODUCT_NAME} ${CREXX_DISPLAY_VERSION}"
InstallDir "$PROGRAMFILES64\CREXX"
InstallDirRegKey HKLM "${CREXX_REG_KEY}" "InstallDir"

!define MUI_ABORTWARNING

!ifdef CREXX_WIZARD_BITMAP
  !define MUI_WELCOMEFINISHPAGE_BITMAP "${CREXX_WIZARD_BITMAP}"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "${CREXX_WIZARD_BITMAP}"
!endif

!define MUI_WELCOMEPAGE_TITLE "Install ${CREXX_PRODUCT_NAME} ${CREXX_DISPLAY_VERSION}"
!define MUI_WELCOMEPAGE_TEXT "This wizard installs the CREXX command-line toolchain and runtime.$\r$\n$\r$\nIt will set CREXX_HOME and REXX_HOME, add the CREXX bin directory to the machine Path, and register an uninstaller."
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_TITLE "${CREXX_PRODUCT_NAME} is ready"
!define MUI_FINISHPAGE_TEXT "Open a new terminal to use crexx, rxc, rxas, rxlink, and rxvm from Path."
!insertmacro MUI_PAGE_FINISH

!define MUI_WELCOMEPAGE_TITLE "Uninstall ${CREXX_PRODUCT_NAME} ${CREXX_DISPLAY_VERSION}"
!define MUI_WELCOMEPAGE_TEXT "This wizard removes the CREXX installation and reverses the CREXX_HOME, REXX_HOME, and Path changes made by the installer."
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_FINISHPAGE_TITLE "${CREXX_PRODUCT_NAME} has been removed"
!define MUI_FINISHPAGE_TEXT "Open a new terminal for the environment changes to take effect."
!insertmacro MUI_UNPAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

VIProductVersion "${CREXX_FILE_VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${CREXX_PRODUCT_NAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "${CREXX_PUBLISHER}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${CREXX_PRODUCT_NAME} Windows x64 Installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${CREXX_DISPLAY_VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright CREXX contributors"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${CREXX_DISPLAY_VERSION}"

!ifdef CREXX_SIGN_HELPER
  !uninstfinalize '"${CREXX_SIGN_HELPER}" "%1"' = 0
!endif

Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "${CREXX_PRODUCT_NAME} Windows x64 requires 64-bit Windows."
    Abort
  ${EndIf}
  SetRegView 64
FunctionEnd

Function un.onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "${CREXX_PRODUCT_NAME} Windows x64 requires 64-bit Windows."
    Abort
  ${EndIf}
  SetRegView 64
FunctionEnd

Function BroadcastEnvironmentChange
  SendMessage ${HWND_BROADCAST} ${WM_SETTINGCHANGE} 0 "STR:Environment" /TIMEOUT=5000
FunctionEnd

Function un.BroadcastEnvironmentChange
  SendMessage ${HWND_BROADCAST} ${WM_SETTINGCHANGE} 0 "STR:Environment" /TIMEOUT=5000
FunctionEnd

; Never bring machine PATH into an NSIS string: ReadRegStr returns empty for
; values exceeding NSIS_MAX_STRLEN. Windows PowerShell's registry API preserves
; the full, unexpanded value and its registry type. The installation directory
; is passed as process environment data, never interpolated as PowerShell code.
!define CREXX_PATH_SCRIPT `$$ErrorActionPreference='Stop';try{$$k=[Microsoft.Win32.Registry]::LocalMachine.OpenSubKey('SYSTEM\CurrentControlSet\Control\Session Manager\Environment',$$true);$$t=$$k.GetValueKind('Path');$$p=[string]$$k.GetValue('Path','',1);$$b=$$env:CREXX_INSTALL_BIN;if($$env:CREXX_PATH_ACTION-eq'add'){if(-not($$p.Split(';')-contains$$b)){if($$p){$$p+=';'+$$b}else{$$p=$$b}}}else{$$p=(';'+$$p+';').Replace(';'+$$b+';',';').Substring(1);if($$p.EndsWith(';')){$$p=$$p.Substring(0,$$p.Length-1)}};$$k.SetValue('Path',$$p,$$t);$$k.Close()}catch{Write-Output $$_;exit 1}`

!macro UpdateInstallPath ACTION
  System::Call 'kernel32::SetEnvironmentVariable(t "CREXX_INSTALL_BIN", t "$INSTDIR\bin")'
  System::Call 'kernel32::SetEnvironmentVariable(t "CREXX_PATH_ACTION", t "${ACTION}")'
  nsExec::ExecToStack `"$WINDIR\Sysnative\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -NonInteractive -Command "${CREXX_PATH_SCRIPT}"`
  Pop $0
  Pop $1
  System::Call 'kernel32::SetEnvironmentVariable(t "CREXX_INSTALL_BIN", p 0)'
  System::Call 'kernel32::SetEnvironmentVariable(t "CREXX_PATH_ACTION", p 0)'
  ${If} $0 != 0
    DetailPrint "Unable to ${ACTION} CREXX's PATH entry (exit $0): $1"
    MessageBox MB_ICONSTOP|MB_OK "Unable to update the machine PATH. Check that Windows PowerShell is available." /SD IDOK
    SetErrorLevel 1
    Abort
  ${EndIf}
!macroend

Function AddInstallBinToPath
  !insertmacro UpdateInstallPath add
FunctionEnd

Function un.RemoveInstallBinFromPath
  !insertmacro UpdateInstallPath remove
FunctionEnd

Section "CREXX" SecCREXX
  SetRegView 64
  SetOverwrite on
  SetOutPath "$INSTDIR"
  File /r "${CREXX_PAYLOAD_DIR}\*"

  WriteUninstaller "$INSTDIR\Uninstall.exe"

  WriteRegStr HKLM "${CREXX_REG_KEY}" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "${CREXX_REG_KEY}" "Version" "${CREXX_DISPLAY_VERSION}"

  WriteRegStr HKLM "${CREXX_UNINSTALL_KEY}" "DisplayName" "${CREXX_PRODUCT_NAME} ${CREXX_DISPLAY_VERSION}"
  WriteRegStr HKLM "${CREXX_UNINSTALL_KEY}" "DisplayVersion" "${CREXX_DISPLAY_VERSION}"
  WriteRegStr HKLM "${CREXX_UNINSTALL_KEY}" "Publisher" "${CREXX_PUBLISHER}"
  WriteRegStr HKLM "${CREXX_UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${CREXX_UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKLM "${CREXX_UNINSTALL_KEY}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  WriteRegDWORD HKLM "${CREXX_UNINSTALL_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${CREXX_UNINSTALL_KEY}" "NoRepair" 1
!ifdef CREXX_ESTIMATED_SIZE_KB
  WriteRegDWORD HKLM "${CREXX_UNINSTALL_KEY}" "EstimatedSize" ${CREXX_ESTIMATED_SIZE_KB}
!endif

  WriteRegExpandStr HKLM "${CREXX_ENV_KEY}" "CREXX_HOME" "$INSTDIR"
  WriteRegExpandStr HKLM "${CREXX_ENV_KEY}" "REXX_HOME" "$INSTDIR"
  Call AddInstallBinToPath
  Call BroadcastEnvironmentChange
SectionEnd

Section "Uninstall"
  SetRegView 64

  ; Optional backend installers have independent entries, scoped to this core.
  ; The core removal owns the entire installation tree, including their stores.
  IfFileExists "$INSTDIR\.llama-installer\registration-id" 0 no_llama_registration
    FileOpen $0 "$INSTDIR\.llama-installer\registration-id" r
    FileRead $0 $1
    FileClose $0
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CREXX.llama.$1.vulkan"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\CREXX.llama.$1.cuda"
  no_llama_registration:

  Call un.RemoveInstallBinFromPath

  ReadRegStr $0 HKLM "${CREXX_ENV_KEY}" "CREXX_HOME"
  ${If} $0 == "$INSTDIR"
    DeleteRegValue HKLM "${CREXX_ENV_KEY}" "CREXX_HOME"
  ${EndIf}

  ReadRegStr $0 HKLM "${CREXX_ENV_KEY}" "REXX_HOME"
  ${If} $0 == "$INSTDIR"
    DeleteRegValue HKLM "${CREXX_ENV_KEY}" "REXX_HOME"
  ${EndIf}

  DeleteRegKey HKLM "${CREXX_UNINSTALL_KEY}"
  DeleteRegKey HKLM "${CREXX_REG_KEY}"

  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR"

  Call un.BroadcastEnvironmentChange
SectionEnd
