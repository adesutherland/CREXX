:: call configuration file for pre-compile, compile, assembly und run
if NOT "%conf%"=="L" call rxconfig.bat
pushd "%pluglib%"
set cmd=%rxc%/rxc -i %build%/lib/rxfnsb;%pluglib% -o %member% %sourcelib%/%member%
echo ----------------------------------------------------
echo [%time%] Compile: %cmd%
echo ----------------------------------------------------
%cmd%
echo [%time%] Compile completed with Return %ERRORLEVEL%
echo ====================================================
popd
