options levelb
import rxfnsb
import rxsocket

if getenv("CREXX_TLS_LIVE_SMOKE") <> "1" then do
  say "PASS: rxsocket TLS live smoke skipped"
  exit 0
end

host = getenv("CREXX_TLS_LIVE_HOST")
if host = "" then host = "www.example.com"

portText = getenv("CREXX_TLS_LIVE_PORT")
if datatype(portText, "W") = 1 then port = portText
else port = 443

sock = socketcreate()
if sock <= 0 then do
  say "socketcreate failed"
  exit 1
end

if sockettimeout(sock, 10000) <> 0 then do
  say "sockettimeout failed:" socketerror(sock)
  drop_rc = socketclose(sock)
  exit 1
end

if socketconnecttls(sock, host, port) <> 0 then do
  say "socketconnecttls failed:" socketerror(sock)
  drop_rc = socketclose(sock)
  exit 1
end

if socketclose(sock) <> 0 then do
  say "socketclose failed"
  exit 1
end

say "PASS: rxsocket TLS live smoke"
exit 0
