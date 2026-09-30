# MIT. Dependency filtering for CMake versions before and after path normalization.
if(POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()
# Older CMake returns mixed separators on Windows. Exclude the OS directory,
# while retaining compiler/SDK redistributables from non-system directories.
set(crexx_windows_system_dll_regex [[.*[/\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\][Ss][Yy][Ss][Tt][Ee][Mm]32[/\\].*]])
