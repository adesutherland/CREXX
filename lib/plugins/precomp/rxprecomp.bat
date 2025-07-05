:: call configuration file for pre-compile, compile, assembly und run
if NOT "%conf%"=="L" call rxconfig.bat
pushd "%pluglib%"
set cmd=%rxvm%/rxvm %rxpre% rx_%plugin% %lib% -a -i "%sourcelib%/%inrexx%" -o "%sourcelib%/%genrexx%" -m "%sourcelib%/%maclib%
echo ----------------------------------------------------
echo [%time%] PreCompile: %cmd%
echo ----------------------------------------------------
%cmd%
echo [%time%] PreCompile completed with Return %ERRORLEVEL%
echo ====================================================
popd