# Beginnings

## Greeting the world

Rexx is designed as a small language, but there are already a lot of  things you can do without involving anything outside of it. One of those is to greet the world, as is the usual and obligatory first example of any programming language book:

```rexx <!--hello.crexx-->
options levelb
/* rexx: welcoming the world */
say 'hello world!'
```

<!--splice--crexx hello.crexx-->

There is no need to include the starting comment, but by all means include it if you want to.[^1]

[^1]: Level B cRexx has a few ```options``` like ```comments_hash```, ```comments_dash``` or ```comments_slash``` to tune the comment format but the starting comment is meant for some operating systems to distinguish Rexx from the other, earlier command processors they have, like exec/exec2 for VM, CLIST for TSO on z/OS and the .bat language for DOS and OS/2.

\code{say} is a statement of the Rexx language and as such built-in. It does not matter if it is expressed as ```say```, ```Say```, or ```SAY````; in other words, the statements of the language are case-insensitive.

With the roots of the Rexx language (when it still went with one less 'x' at the end) as a command language, there is one other important thing we can do without any outside support, and that is sending commands to the environment.

```rexx <!--helloaddress.rexx-->
options levelb
/* rexx: welcoming the world */
say 'hello world!'
'date'
```

<!--splice--crexx helloaddress.rexx-->

As ```system``` is the standard environment before we change it (by specifying something else on the \code{address} statement), this is sent to this environment, most of the time the shell under which your program executes. If this has a ```date``` command, the output of it will be returned. The I/O to the environment using the ```address``` statement can also be redirected using *stem variables*. In fact, sending commands to an environment is one of the examples of Object Orientation using Classic Rexx; the same message can be sent to different objects, as long as these are implemented in the environment, and will potentially yield different answers.
