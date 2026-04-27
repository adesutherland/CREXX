# Running cRexx

These are instructions for running the binary distribution as downloaded from the https://github.com/adesutherland/CREXX/releases section  

The executables and libraries are in the release/bin directory.
For the Windows and macOS platforms the executables need to be de-quarantined once before running. The executable directory can be added to the PATH environment variable; the `crexx` driver program will find its library directory automatically.

## Windows

When the zip file has been download, select the property that marks it as unblocked. For this, right-click on the downloaded file in the Downloads location, and choose 'Properties' on the context window.
The bottom part of the dialogue consists of the message: Security: This file came from another computer and might be blocked to help protect this computer. Select (checkmark): Unblock.

The proceed to 'extract all' (unzip) this file to its destination on the filesystem. Add the `bin` directory to the (user or system) PATH environment variable.

## Linux

Unzip the archive to any desired destination and add its `bin` directory to the PATH environment variable.

## macOS

in the `bin` directory, run 

```
xattr -d com.apple.quarantine *
chmod +x *
```

This will remove the quarantine attribute and make the executables executable (that last action is just-in-case: the executables will probably also run without this command). If the quarantine attribute is not removed, macOS will tell you it cannot check the executables for malware and will suggest to put them in the (garbage) bin, and when it is not marked as executable, it will tell you that you have no access.

## Verifing the installation

To verify that the executable directory succesfully was added to the PATH environment variable:

- add a file hello.rexx to your home directory or another writeable directory of your choosing, use an editor that produces plain text.

Make sure that it contains:

```rexx
options levelb

say 'hello world'
```

- Then run `crexx hello`

It should say 'hello world' when it works. For added information, use the --verbose1 to --verbose4 options.

## Compile to a native executable

To make a native executable out of `hello.rexx`, like `hello.exe` on Windows, or just a plain ./hello under Linux or macOS, a C compiler (gnu or clang) should be installed. 

### On Linux

Install `build-essential` with `sudo apt build-essential` (for Debian or Ubuntu) or the proper incantation for your distribution.

### On Windows

Install the msys2 distribution of the gnu c compiler.

### On macOS

When your macOS installation does not contain a c compiler, type in `gcc` in a shell window, and the os will suggest to install it for you. Click OK and wait a few moments until this process has finished.

Then issue the command `crexx hello --native` to compile the hello.rexx into an standalone, native executable.


