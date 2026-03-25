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
xattr -d com.apple.quarantine .
chmod +x *
```

This will remove the quarantine attribute and make the executables executable. If the quarantine attribute is not removed, macOS will suggest to put the executable in the (garbage) bin, and when it is not marked as executable, it will tell you that you have no access.
