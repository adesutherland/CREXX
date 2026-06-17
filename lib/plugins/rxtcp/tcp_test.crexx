/* TCP Test */
options levelb
import rxtcp
import rxfnsb
/* ----------------------------------------------------------------------------------
 * Sample CREXX TCP Client
 * ----------------------------------------------------------------------------------
 */
say tcpflags("Debug")

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

