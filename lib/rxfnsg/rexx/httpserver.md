# Bounded Level G HTTP server source note

`httpserver.crexx` adds the public `.httpserver`, `.httprequest` and
`.httpservice` types to `rxfnsg`. It uses `_rxhttpcore`, the same private binary
framing backend as the Level G client and LLM library.

The controller owns the listening and accepted sockets. It accepts only a
complete, bounded request before submitting a sealed task, and the task receives
only a transferable `.httprequest` record plus its `.taskcontext`. A private
bounded byte endpoint carries the buffered `.httpresponse` back to the
controller, so neither sockets nor live VM objects cross worker boundaries.

A service class implements both interfaces. The explicit `run` bridge is part
of the canonical `.taskwork` ABI; `.httpservice.dispatch` supplies the shared
typed adapter:

```rexx
myservice: class implements .httpservice .taskwork
  *: factory
    return

  handle: method = .httpresponse
    arg request = .httprequest, context = .taskcontext
    if request.verb() = "GET" & request.target() = "/health" then do
      return .httpresponse.text(200, "ok")
    end
    return .httpresponse.text(404, "not found")

  run: method = .channelvalue
    arg encoded = .channelvalue, context = .taskcontext
    service = self as .httpservice
    return service.dispatch(encoded, context)

server = .httpserver.local("127.0.0.1", 8080)
call server.serve(task .myservice())
call server.close()
```

The initial server supports clear-text HTTP/1.0 and HTTP/1.1 with buffered
request and response bodies, canonical `Content-Length`, keep-alive and one
in-flight request per connection. Worker count, admission, accepted
connections, header bytes, header count, body bytes, request-read time and
handler time are all bounded. Request `Transfer-Encoding` is rejected with
501; TLS, HTTP/2, WebSockets, detached serving and streaming handlers are not
part of this surface.

The listener is nonblocking. A private connected loopback socket gives the
controller a portable 1 ms idle wait because receive timeouts are portable here
while accept timeouts are not. This keeps request deadlines advancing without
an idle busy-spin or an optional system-plugin dependency.

Executable coverage is in
`lib/rxfnsg/tests_functional/ts_http_service_contract.crexx` and
`lib/rxfnsg/tests_functional/ts_http_server.crexx`, with separate handler
failure and unusable-response coverage in
`lib/rxfnsg/tests_functional/ts_http_server_failures.crexx`. The matrix includes
both threaded VM runners, optimized and unoptimized bytecode, binary bodies,
pipelining parser boundaries, malformed framing and an incomplete-request 408
deadline.
