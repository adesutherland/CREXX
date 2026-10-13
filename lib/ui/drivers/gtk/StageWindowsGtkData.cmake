# GTK runtime data is kept separate from the adjacent-DLL native manifest.
# Windows GdkPixbuf's cache names modules relative to the installation prefix.
foreach(relative IN ITEMS
        share/glib-2.0/schemas share/icons/Adwaita share/icons/AdwaitaLegacy
        share/icons/hicolor share/mime share/gtk-3.0
        lib/gdk-pixbuf-2.0/2.10.0)
    if(NOT IS_DIRECTORY "${PREFIX}/${relative}")
        message(FATAL_ERROR "GTK runtime data missing: ${PREFIX}/${relative}")
    endif()
    get_filename_component(parent "${relative}" DIRECTORY)
    file(MAKE_DIRECTORY "${OUTPUT}/${parent}")
    file(COPY "${PREFIX}/${relative}" DESTINATION "${OUTPUT}/${parent}")
endforeach()
file(READ "${OUTPUT}/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache" loader_cache)
if(loader_cache MATCHES [=["[A-Za-z]:[/\\]]=])
    message(FATAL_ERROR "GTK loader cache contains an absolute Windows module path")
endif()
file(WRITE "${OUTPUT}/complete.stamp" "GTK data staged from ${PREFIX}\n")
