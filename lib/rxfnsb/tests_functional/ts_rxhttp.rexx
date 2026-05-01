options levelb
import rxfnsb
import rxhttp

failures = 0
crlf = "0d0a"x
client = .httpclient("127.0.0.1", 11434, 1000, 0)

body = "Hello"
request = client.buildRequest("POST", "/api/test", body, "text/plain")
failures = failures + check_int("request method", left(request, 19) = "POST /api/test HTTP", 1)
failures = failures + check_int("request host", pos("Host: 127.0.0.1:", request) > 0, 1)
failures = failures + check_int("request content type", pos("Content-Type: text/plain", request) > 0, 1)
failures = failures + check_int("request content length", pos("Content-Length: 5", request) > 0, 1)
failures = failures + check_int("request identity encoding", pos("Accept-Encoding: identity", request) > 0, 1)
failures = failures + check_int("request close", pos("Connection: close", request) > 0, 1)

unicodeBody = "Hello " || d2c(128512)
unicodeRequest = client.buildRequest("POST", "/api/test", unicodeBody, "text/plain")
unicodeLength = utf8_len(unicodeBody)
failures = failures + check_int("request byte content length", pos("Content-Length: " || unicodeLength, unicodeRequest) > 0, 1)

json = '{"ok":true,"response":"Hello"}'
http = "HTTP/1.1 200 OK" || crlf || ,
       "Content-Type: application/json" || crlf || ,
       "Content-Length: " || length(json) || crlf || ,
       crlf || json || "ignored"
extracted = client.extractBody(http)
failures = failures + check_str("extract content-length body", extracted, json)
failures = failures + check_int("extract content-length status", client.status(), 0)
failures = failures + check_int("extract content-length http status", client.httpStatus(), 200)
failures = failures + check_str("extract content-type header", client.header("content-type"), "application/json")
failures = failures + check_str("last body", client.lastBody(), json)

chunkedHttp = "HTTP/1.1 200 OK" || crlf || ,
              "Transfer-Encoding: chunked" || crlf || ,
              crlf || ,
              "5" || crlf || left(json, 5) || crlf || ,
              d2x(length(json) - 5) || crlf || substr(json, 6) || crlf || ,
              "0" || crlf || crlf
chunkedExtracted = client.extractBody(chunkedHttp)
failures = failures + check_str("extract chunked body", chunkedExtracted, json)
failures = failures + check_int("extract chunked status", client.status(), 0)

unicodeJson = '{"ok":true,"response":"Hello ' || d2c(128512) || '"}'
unicodeChunkedHttp = "HTTP/1.1 200 OK" || crlf || ,
                     "Transfer-Encoding: chunked" || crlf || ,
                     crlf || ,
                     d2x(utf8_len(unicodeJson)) || crlf || unicodeJson || crlf || ,
                     "0" || crlf || crlf
unicodeExtracted = client.extractBody(unicodeChunkedHttp)
failures = failures + check_str("extract unicode chunked body", unicodeExtracted, unicodeJson)
failures = failures + check_int("extract unicode chunked status", client.status(), 0)

badJson = '{"error":"busy"}'
badHttp = "HTTP/1.1 503 Service Unavailable" || crlf || ,
          "Content-Length: " || utf8_len(badJson) || crlf || ,
          crlf || badJson
badBody = client.extractBody(badHttp)
failures = failures + check_int("http error status", client.status(), 503)
failures = failures + check_int("http error code", client.httpStatus(), 503)
failures = failures + check_int("http error body", pos('"error"', badBody) > 0, 1)

truncated = "HTTP/1.1 200 OK" || crlf || ,
            "Content-Length: 99" || crlf || ,
            crlf || "short"
truncatedBody = client.extractBody(truncated)
failures = failures + check_int("truncated status", client.status(), -31)
failures = failures + check_str("truncated body", truncatedBody, "short")

invalid = client.extractBody("no status" || crlf || crlf || "body")
failures = failures + check_int("invalid status", client.status(), -21)

if failures <> 0 then do
  say "rxhttp failures:" failures
  exit 1
end

say "PASS: rxhttp helpers"
exit 0

check_int: procedure = .int
  arg label = .string, actual = .int, expected = .int
  if actual <> expected then do
    say label": expected" expected "got" actual
    return 1
  end
  return 0

check_str: procedure = .int
  arg label = .string, actual = .string, expected = .string
  if actual <> expected then do
    say label": expected ["expected"] got ["actual"]"
    return 1
  end
  return 0

utf8_len: procedure = .int
  arg text = .string
  bytes = 0
  do i = 1 to length(text)
    code = c2d(substr(text, i, 1))
    if code < 128 then bytes = bytes + 1
    else if code < 2048 then bytes = bytes + 2
    else if code < 65536 then bytes = bytes + 3
    else bytes = bytes + 4
  end
  return bytes
