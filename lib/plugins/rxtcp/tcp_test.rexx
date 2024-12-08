/* TCP Test */
options levelb
import rxtcp
import rxfnsb

sockets.1=""
server=TCPSERVER(3090,sockets)
if server<0 then do
   say "Server Create faile with rc="server
   exit
end
say "Server Socket "server

i=0

do forever
   i=i+1
   if i>100 then leave
   client=TCPWAIT(server,100,sockets)
##   say time('l')' RC Wait 'i' 'client
   if client>0 then do           ## -4 is timeout occurred, nothing new
      say "Client Socket "client
      record=TCPRECEIVE(client,100)                     ## timeout in milliseconds
      say "TCPRECEIVE : Length: "length(record) record  ## receive record of client
      say "TCPSEND    : "tcpsend(client,"Hello Client, I am a CREXX server")  ## reply to client
      say "** Active clients **"
      do j=1 to sockets.0
         say "current open sockets "j" "sockets.j
      end
   end
   call wait 100
end
say "TCPCLOSE   : "TCPCLOSE(server)

exit
/*
say time()' before wait'
call wait(300)
say time()' after wait'
socket=tcpopen("127.0.0.1",3033)
say "Socket: "socket
say "TCPSEND    : "tcpsend(socket,"Hello Server, I am a CREXX script")
record=TCPRECEIVE(socket,10)                   ## timeout in milliseconds
say "TCPRECEIVE : Length: "length(record) record

say "TCPSEND    : "tcpsend(socket,"How is it going")
record=TCPRECEIVE(socket,1000)                   ## timeout in milliseconds
say "TCPRECEIVE : Length: "length(record) record

say "TCPSEND    : "tcpsend(socket,"I am happy to meet you")
record=TCPRECEIVE(socket,10)                   ## timeout in milliseconds
say "TCPRECEIVE : Length: "length(record) record

say "TCPSEND    : "tcpsend(socket,"Sorry, I must leave now")
record=TCPRECEIVE(socket,10)                   ## timeout in milliseconds
say "TCPRECEIVE : Length: "length(record) record

say "TCPCLOSE   : "TCPCLOSE(socket)
*/
