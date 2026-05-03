do forever
   pull msg
   call lineout 'STDOUT', 'Regina> I received <'msg'> from you, how can I help'
   call stream  'STDOUT', 'C', 'FLUSH'
   if pos('GOODBYE',translate(msg))=0 then iterate
   call lineout 'STDOUT', 'Regina> Hello CREXX, was nice talking to you, cheers see you soon'
   call stream  'STDOUT', 'C', 'FLUSH'
   leave
 end
   call lineout 'STDOUT', 'Regina terminates now'
   call stream  'STDOUT', 'C', 'FLUSH'
exit 4711