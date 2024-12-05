/* TCP Test */
options levelb
import rxtcp
import rxfnsb
say time()' before wait'
call wait(3000)
say time()' after wait'
socket=tcpopen("127.0.0.1",3033)
say "Socket: "socket
say "TCPSEND    : "tcpsend(socket,"Hello Server, I am a CREXX script")
say "TCPRECEIVE : "TCPRECEIVE(socket,10)
say "TCPSEND    : "tcpsend(socket,"How is it going")
say "TCPRECEIVE : "TCPRECEIVE(socket,1000)                   ## timeout in milliseconds

say "TCPSEND    : "tcpsend(socket,"I am happy to meet you")
say "TCPRECEIVE : "TCPRECEIVE(socket,10)                      ## timeout in milliseconds

say "TCPSEND    : "tcpsend(socket,"Sorry, I must leave now")
say "TCPRECEIVE : "TCPRECEIVE(socket,10)                       ## timeout in milliseconds

say "TCPCLOSE   : "TCPCLOSE(socket)

