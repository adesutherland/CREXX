options levelb
import rxfnsb
import process

##cflags def nset  3buf  parse

call ReginaDuplex

pid = processcreate('cmd /C ping 127.0.0.1 -n 4', 1)
say 'Handle:' pid

do while processisrunning(pid)
    peek = processpeekoutput(pid)
    if peek \= '' then
        say 'PEEK> ' peek
    call processwait pid, 500
end

## call processfree pid
say 'done.'


pid = processcreate('cmd /C more', 1)
if pid < 0 then do
    say 'Fehler beim Starten des Prozesses!'
    exit 1
end

say 'Process handle:' pid

## Sende some lines to stdin
rc = processsendinput(pid, 'Hello from REXX!' || "0a"x)
rc = processsendinput(pid, 'This is line 2' || "0a"x)
rc = processsendinput(pid, 'Goodbye!' || "0a"x)

## Close stdin to send EOF to Process
call processcloseinput pid

say '--- Output streaming ---'
do forever
    line = processreadoutput(pid)
    if line = '' then do
        call processwait pid, 100
        if processgetexitcode(pid) \= -1 then leave
        iterate
    end
    say 'OUT> ' line
end
code = processgetexitcode(pid)
call processstackinfo
say 'Exit code:' code
exit

/* Start Process */
proc = processcreate('C:\Program Files\rexx.org\Regina\regina.exe "C:\Temp\pipeget.rexx"', 1)
say 'Process 'proc
say 'Last CC 'processlasterror(proc)
say 'Set Timeout 'processsettimeout(proc,5500)
do while processisrunning(proc)
    line = processreadoutput(proc)
    if line \= "" then say line
    else say 'n/a'
end

tasks=processList()
say 'Current active Processes'
do j=1 to tasks[0]
   say tasks[j] processisrunning(tasks[j])
end

/* Watch Process (polling) */
ww=0
do while processisrunning(proc)
   ww=ww+1
   call wait 10
   say 123 ww
end

/* Capture all available output (stdout/stderr) */
kept = processgetoutput(proc)
say kept
say "Exit Code "processGetExitCode(proc)


tasks=processList()
say 'Current active Processes'
do j=1 to tasks[0]
   say tasks[j] processisrunning(tasks[j])
end

/* Free all processes */
call processfree proc
say processFreeAll()' freed'


ReginaDuplex: procedure
Regina = processcreate('C:\Program Files\rexx.org\Regina\regina.exe "C:\Temp\pipeget.rexx"', 1)
if regina < 0 then do
    say "Process couldn't be started"
    exit 1
end

say 'Process handle:' regina

## Sende some lines to stdin
rc = processsendinput(regina, 'Hello from CREXX!' || "0d0a"x)
say 1 processreadoutput(regina)
rc = processsendinput(regina, 'I need your help' ||  "0d0a"x)
say 2 processreadoutput(regina)
rc = processsendinput(regina, 'Goodbye!' ||  "0d0a"x)
say 3 processreadoutput(regina)
say 4 processreadoutput(regina)
## Close stdin to send EOF to Process
call processcloseinput regina

say '--- Output remaining messages from Regina ---'
do forever
    line = processreadoutput(regina)
    if line = '' then do
        call processwait regina, 100
        if processgetexitcode(regina) \= -1 then leave
        iterate
    end
    say 'Regina> 'line
end
cc = processgetexitcode(regina)
say 'Regina> returned with exit code='cc
return
