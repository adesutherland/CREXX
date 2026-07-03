@echo off
setlocal
REM -------------------------------------------------------------
REM Compile a CREXX Script
REM -------------------------------------------------------------

:: Input parameters
set flags=%~1
set inrexx=%~2
set genrexx=%~3
set maclib=%~4

for %%f in ("%~3") do (
    set "member=%%~nf"
)

setlocal enabledelayedexpansion
call rxflags.bat

 REM -------------------------------------------------------------
 REM Pre Compile CREXX Script
 REM -------------------------------------------------------------
  if "!precomp!"=="P" call rxprecomp.bat

 REM -------------------------------------------------------------
 REM Compile CREXX Script
 REM -------------------------------------------------------------
  if "!compile!"=="C" call rxcompile.bat

 REM -------------------------------------------------------------
 REM Assembly and Link the compile CREXX Script
 REM -------------------------------------------------------------
  if "!asm!"=="A" call rxasm.bat

  REM -------------------------------------------------------------
  REM Execute the CREXX script
  REM -------------------------------------------------------------
   if "!run!"=="R" call rxrun.bat