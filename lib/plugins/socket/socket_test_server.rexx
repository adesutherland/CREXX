options levelb
import rxfnsb
import socket

##cflags def nset  3buf  parse

server = socketCreate()
if server then say "Server started "server
else exit
rc = socketBind(server, "0.0.0.0", 12345)
if rc=0 then say "Server bind completed for "server
else exit
rc = socketListen(server, 5)
if rc=0 then say "Server got connect request for "server
else exit
call socketsettimeout server,5000
do forever
   client = socketAccept(server)
   say "Server accepted connection from "client
   say 'client connect status 'socketIsConnected(client)
   do while socketIsConnected(client)
      line = socketRecvLine(client)
      if line = "" then leave
      say 'Line received from 'client': 'line
      call socketSend client, "You said: "line"\r\n"
   end
   call socketClose client
end
call socketClose server