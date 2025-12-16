# Running cRexx

These are instructions for running the binary distribution as downloaded from the https://github.com/adesutherland/CREXX/releases section  
The executables are in the release/bin directory, the libraries are in release/lib.
For the Windows and macOS platforms the executables need to be de-quarantined once before running. The executable directory can be added to the PATH environment variable; the `crexx` driver program will find its library directory automatically.


## Windows

## Linux

## macOS

in the `bin` directory, run 

```
xattr -d com.apple.quarantine .
chmod +x *
```

This will remove the quarantine attribute and make the executables executable. If the quarantine attribute is not removed, macOS will suggest to put the executable in the (garbage) bin, and when it is not marked as executable, it will tell you that you have no access.
