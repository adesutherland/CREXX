options levelb
import rxfnsb
import socket

##cflags def nset  3buf  parse
token = socketCreate()
say 'socket  'token
say 'connect 'socketConnect(token, 'localhost', 12345)
say 'error   'socketlasterror(token)
say "Connected? "socketIsConnected(token)
say "Local IP? "socketLocalInfo(token)
say "Peer: "socketPeerInfo(token)
say "TimeOut: "socketSetTimeout(token,100)
say 'send    'socketSend(token, "Hello Server")
say 'error   'socketlasterror(token)
##   line=socketRecv(token,1024)

do forever
   line=socketRecvline(token)
   if line = "" then leave
   say "RECV: '"line"'"
end
call socketShutdown token, 0     ## No more reads allowed (SHUT_RD)
## call socketShutdown token, 1  ## No more sends allowed (SHUT_WR)
## call socketShutdown token, 2 ## Fully close (sends FIN, both directions, SHUT_RDWR)

  call socketClose token