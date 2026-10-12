# Keep standalone tools' narrow CRT argv and Win32 paths in internal UTF-8.
# This process property is supported on Windows 10 version 1903 and later;
# it does not select an external file/console stream codec.
if(WIN32)
    set(_crexx_utf8_manifest "${CMAKE_SOURCE_DIR}/platform/windows-utf8.manifest")
    if(NOT MSVC)
        enable_language(RC)
        set_source_files_properties("${CMAKE_SOURCE_DIR}/platform/windows-utf8.rc"
            PROPERTIES OBJECT_DEPENDS "${_crexx_utf8_manifest}")
    endif()
    foreach(_crexx_utf8_tool IN ITEMS
            rxc rxc-c rxas rxlink rxdas rxdb rxpp rxseq
            rxbvm rxtvm rxvme rxbvme crexxsaa_tool
            rxbvm_instrumented rxtvm_instrumented)
        if(TARGET "${_crexx_utf8_tool}")
            if(MSVC)
                # CMake merges this with the linker's normal executable manifest.
                target_sources("${_crexx_utf8_tool}" PRIVATE "${_crexx_utf8_manifest}")
            else()
                target_sources("${_crexx_utf8_tool}" PRIVATE
                    "${CMAKE_SOURCE_DIR}/platform/windows-utf8.rc")
                target_include_directories("${_crexx_utf8_tool}" PRIVATE
                    "${CMAKE_SOURCE_DIR}/platform")
            endif()
        endif()
    endforeach()
endif()
