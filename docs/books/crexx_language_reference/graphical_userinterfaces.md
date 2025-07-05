# Graphical User Interfaces and Graphics

## The GTK Plugin

The (multiplatform) GTK Plugin enables a straightforward way to implement portable Rexx programs with agraphical user interface.

\crexx{} does not implement event loop multitasking or callbacks yet. At the moment, the inclusion of the GTK plugin is a build time option[^option]. 

[^option]: \code{-DENABLE_GTK=ON}

