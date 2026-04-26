options levelb
import rxsocket

host = "127.0.0.1"
server = socketcreate()
if server <= 0 then do
  say "socketcreate server failed"
  exit 1
end

port = 32164
last_port = 32204
bound = 0
do while port <= last_port
  if socketbind(server, host, port) = 0 then do
    bound = 1
    leave
  end
  drop_rc = socketclose(server)
  server = socketcreate()
  if server <= 0 then do
    say "socketcreate retry failed"
    exit 1
  end
  port = port + 1
end

if bound = 0 then do
  say "socketbind failed:" socketerror(server)
  exit 1
end

if socketlocal(server) = "" then do
  say "socketlocal server failed"
  exit 1
end

if sockettimeout(server, 5000) <> 0 then do
  say "sockettimeout server failed:" socketerror(server)
  exit 1
end

if socketlisten(server, 5) <> 0 then do
  say "socketlisten failed:" socketerror(server)
  exit 1
end

client = socketcreate()
if client <= 0 then do
  say "socketcreate client failed"
  exit 1
end

if sockettimeout(client, 5000) <> 0 then do
  say "sockettimeout client failed:" socketerror(client)
  exit 1
end

if socketconnect(client, host, port) <> 0 then do
  say "socketconnect failed:" socketerror(client)
  exit 1
end

if socketblocking(client, 1) <> 0 then do
  say "socketblocking failed:" socketerror(client)
  exit 1
end

if socketnodelay(client, 1) <> 0 then do
  say "socketnodelay failed:" socketerror(client)
  exit 1
end

if socketkeepalive(client, 1) <> 0 then do
  say "socketkeepalive failed:" socketerror(client)
  exit 1
end

accepted = socketaccept(server)
if accepted <= 0 then do
  say "socketaccept failed:" socketerror(server)
  exit 1
end

if socketpeer(client) = "" then do
  say "socketpeer client failed"
  exit 1
end

sent = socketsend(client, "ping")
if sent <> 4 then do
  say "socketsend client failed:" sent socketerror(client)
  exit 1
end

pending = socketpending(accepted)
if pending < 0 then do
  say "socketpending failed:" socketerror(accepted)
  exit 1
end

received = socketrecv(accepted, 4)
if received <> "ping" then do
  say "socketrecv server expected [ping] got ["received"]:" socketerror(accepted)
  exit 1
end

sent = socketsend(accepted, "pong")
if sent <> 4 then do
  say "socketsend accepted failed:" sent socketerror(accepted)
  exit 1
end

received = socketrecv(client, 4)
if received <> "pong" then do
  say "socketrecv client expected [pong] got ["received"]:" socketerror(client)
  exit 1
end

if socketstatus(client) <> 0 then do
  say "socketstatus client failed:" socketerror(client)
  exit 1
end

if socketshutdown(client, 2) <> 0 then do
  say "socketshutdown failed:" socketerror(client)
  exit 1
end

if socketclose(client) <> 0 then do
  say "socketclose client failed"
  exit 1
end

if socketclose(accepted) <> 0 then do
  say "socketclose accepted failed"
  exit 1
end

if socketclose(server) <> 0 then do
  say "socketclose server failed"
  exit 1
end

say "PASS: rxsocket loopback"
exit 0
