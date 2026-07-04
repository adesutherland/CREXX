:: call configuration file for pre-compile, compile, assembly und run
if NOT "%conf%"=="L" call rxconfig.bat
pushd "%pluglib%"
set cmd=%rxas%/rxas -l %pluglib% -o %member% %member%
echo ----------------------------------------------------
echo [%time%] ASM and Link: %cmd%
echo ----------------------------------------------------
%cmd%
echo [%time%] ASM and Link completed with Return %ERRORLEVEL%
echo ====================================================
popd
