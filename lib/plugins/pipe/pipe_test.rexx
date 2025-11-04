options levelb
import rxfnsb
import pipe

##cflags def nset  3buf  parse
array=.string[]
token = pipecreate("w")                          ##  Duplex mode
call piperun token, "sort", "w"                  ##  Start process
say "+++ RUN            "pipeRUN(token,'C:\Program Files\rexx.org\Regina\regina.exe "C:\Temp\pipeput.rexx"','w')
call pipesend token, "banana\napple\npear\n"      ##  Send data

call pipeclose token                         ##  Signal no more input

exit
call pipeget token, array, 0                      ##  Capture ALL output lines

do i=1 to array[0]
   say array[i]
end

rc = pipeexitcode(token)                          ##  (Optional) get exit code

call pipeclose token                              ##  Always clean up!

/*
array=.string[]

token=pipeCreate()
token2=pipeCreate('w')
say "*** Pipe Token     "token token2
## say "SEND DIR command "pipeRUN(token,'cmd /c dir /b')

say "+++ RUN            "pipeRUN(token,'C:\Program Files\rexx.org\Regina\regina.exe "C:\Temp\pipeget.rexx"')
say "*** Status         "pipeStatus(token)

say "Read array (lines) "Pipeget(token,array)
say "*** Status         "pipeStatus(token)

say "*** ExitCode       "PipeExitCode(token)
say "*** Status         "pipeStatus(token)
say array[0]

say "--- Captured Results"
do i=1 to array[0]
   say i array[i]
end
array2=.string[]
say pipesend(token2,'Hello')
say "*** Status         "pipeStatus(token2)
say "Read array (lines) "Pipeget(token2,array2)
say "*** Status         "pipeStatus(token2)
say array2[0]

say "--- Captured Results"
do i=1 to array2[0]
   say i array2[i]
end

call PipeClose(token)
call PipeClose(token2)
say 'This is the end'
return
*/