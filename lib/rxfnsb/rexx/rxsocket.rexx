options levelb
namespace rxsocket expose socketcreate socketclose socketconnect socketbind socketlisten socketaccept socketshutdown socketsend socketsendb socketrecv socketrecvb socketpending sockettimeout socketblocking socketnodelay socketkeepalive socketpeer socketlocal socketstatus socketerror

socketcreate: procedure = .int
  sock = .int
  assembler socknew sock
  return sock

socketclose: procedure = .int
  arg sock = .int
  rc = .int
  assembler sockclose rc,sock
  return rc

socketconnect: procedure = .int
  arg sock = .int, host = .string, port = .int
  rc = .int
  assembler sockconnect sock,host,port
  assembler sockstatus rc,sock
  return rc

socketbind: procedure = .int
  arg sock = .int, host = .string, port = .int
  rc = .int
  assembler sockbind sock,host,port
  assembler sockstatus rc,sock
  return rc

socketlisten: procedure = .int
  arg sock = .int, backlog = .int
  rc = .int
  assembler socklisten rc,sock,backlog
  return rc

socketaccept: procedure = .int
  arg sock = .int
  client = .int
  assembler sockaccept client,sock
  return client

socketshutdown: procedure = .int
  arg sock = .int, how = .int
  rc = .int
  assembler sockshutdown rc,sock,how
  return rc

socketsend: procedure = .int
  arg sock = .int, data = .string
  sent = .int
  assembler socksend sent,sock,data
  return sent

socketsendb: procedure = .int
  arg sock = .int, data = .binary
  sent = .int
  assembler socksendb sent,sock,data
  return sent

socketrecv: procedure = .string
  arg sock = .int, maxbytes = .int
  data = .string
  assembler sockrecv data,sock,maxbytes
  return data

socketrecvb: procedure = .binary
  arg sock = .int, maxbytes = .int
  data = .binary
  assembler sockrecvb data,sock,maxbytes
  return data

socketpending: procedure = .int
  arg sock = .int
  bytes = .int
  assembler sockpending bytes,sock
  return bytes

sockettimeout: procedure = .int
  arg sock = .int, milliseconds = .int
  rc = .int
  assembler socktimeout rc,sock,milliseconds
  return rc

socketblocking: procedure = .int
  arg sock = .int, enabled = .int
  rc = .int
  assembler sockblocking rc,sock,enabled
  return rc

socketnodelay: procedure = .int
  arg sock = .int, enabled = .int
  rc = .int
  assembler socknodelay rc,sock,enabled
  return rc

socketkeepalive: procedure = .int
  arg sock = .int, enabled = .int
  rc = .int
  assembler sockkeepalive rc,sock,enabled
  return rc

socketpeer: procedure = .string
  arg sock = .int
  endpoint = .string
  assembler sockpeer endpoint,sock
  return endpoint

socketlocal: procedure = .string
  arg sock = .int
  endpoint = .string
  assembler socklocal endpoint,sock
  return endpoint

socketstatus: procedure = .int
  arg sock = .int
  status = .int
  assembler sockstatus status,sock
  return status

socketerror: procedure = .string
  arg sock = .int
  message = .string
  assembler sockerror message,sock
  return message
