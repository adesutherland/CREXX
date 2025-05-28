:: call configuration file for pre-compile, compile, assembly und run
if NOT "%conf%"=="L" call rxconfig.bat
pushd "%pluglib%"
set cmd=%rxvm%/rxvm %member%  rx_%plugin% %lib% -a
echo ----------------------------------------------------
echo [%time%] VVMRUN: %cmd%
echo ----------------------------------------------------
%cmd%
echo [%time%] VMRUN completed with Return %ERRORLEVEL%
echo ====================================================
popd