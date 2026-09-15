# MIT. Exercise the actual dependency filter with old/new CMake path forms.
include("${ROOT}/rxpa/RuntimePackagePaths.cmake" NO_POLICY_SCOPE)
foreach(path
        [[C:/Windows/System32/kernel32.dll]]
        [[C:\Windows\system32/kernel32.dll]]
        [[C:\WINDOWS\SYSTEM32\msvcrt.dll]])
    if(NOT path MATCHES "${crexx_windows_system_dll_regex}")
        message(FATAL_ERROR "System DLL would be redistributed: ${path}")
    endif()
endforeach()
foreach(path
        [[D:/msys64/mingw64/bin/libstdc++-6.dll]]
        [[D:\package\bin\providers/msvcp140.dll]]
        [[D:/CUDA/bin/cublas64_12.dll]])
    if(path MATCHES "${crexx_windows_system_dll_regex}")
        message(FATAL_ERROR "Redistributable would be excluded: ${path}")
    endif()
endforeach()
message(STATUS "PASS: Windows system and compiler/SDK dependency paths")
