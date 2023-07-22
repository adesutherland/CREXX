### Description

This program generates a corpus of regular expressions and input strings.

### Authors

This program was written by Angelo Borsotti.

### Build and run

You can build the program either manually or with CMake:

- Manually:

  ```
  javac -encoding UTF8  REgen.java
  javac -encoding UTF8  REgenMain.java
  java -Dfile.encoding=UTF8 -Xmx1200m -Xss10m REgenMain <benchmark>
  ```

- With CMake

  ```
  cmake -S . -B <builddir>
  cmake --build <builddir>
  java -jar <builddir>/REgen.jar <benchmark>
  ```

### Customization

REgenMain.java contains an example of benchmark generation. You can tailor it
to fit the generation of your specific benchmark:

- change the class MyEmitter, providing some implementation of the methods
  in it, that allow you to store in whatever way you need the REs and texts.

- set the parameters to the values that represent best the characteristics of
  your use case.

