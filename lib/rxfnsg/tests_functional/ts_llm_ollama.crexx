options levelg
import rxfnsg
import rxfnsb
import rxjson

failures = 0
crlf = "0d0a"x
client = .llm("demo-model", "127.0.0.1", 11434, 1000)
failures = failures + check_str("last json default", client.lastJson(), "")

body = client.buildBody("Say hi")
failures = failures + check_int("body valid", jsonvalid(body), 1)
failures = failures + check_str("body model", jsonget(body, "model"), "demo-model")
failures = failures + check_str("body prompt", jsonget(body, "prompt"), "Say hi")
failures = failures + check_str("body stream", jsonget(body, "stream"), "false")

request = client.buildRequest("Say hi")
failures = failures + check_int("request method", left(request, 23) = "POST /api/generate HTTP", 1)
failures = failures + check_int("request content type", pos("Content-Type: application/json", request) > 0, 1)
failures = failures + check_int("request content length", pos("Content-Length: " || length(body), request) > 0, 1)
failures = failures + check_int("request close", pos("Connection: close", request) > 0, 1)

unicodePrompt = "Say " || d2c(128512)
unicodeBody = client.buildBody(unicodePrompt)
unicodeRequest = client.buildRequest(unicodePrompt)
unicodeLength = utf8_len(unicodeBody)
failures = failures + check_int("request byte content length", pos("Content-Length: " || unicodeLength, unicodeRequest) > 0, 1)

json = '{"model":"demo-model","response":"Hello from fake Ollama","done":true}'
http = "HTTP/1.1 200 OK" || crlf || ,
       "Content-Type: application/json" || crlf || ,
       "Content-Length: " || length(json) || crlf || ,
       crlf || json
extracted = client.extractBody(http)
failures = failures + check_str("extract body", extracted, json)
failures = failures + check_int("extract status", client.status(), 0)
failures = failures + check_str("extract text", client.extractText(extracted), "Hello from fake Ollama")

chunkedHttp = "HTTP/1.1 200 OK" || crlf || ,
              "Content-Type: application/json" || crlf || ,
              "Transfer-Encoding: chunked" || crlf || ,
              crlf || ,
              "5" || crlf || left(json, 5) || crlf || ,
              d2x(length(json) - 5) || crlf || substr(json, 6) || crlf || ,
              "0" || crlf || crlf
chunkedExtracted = client.extractBody(chunkedHttp)
failures = failures + check_str("extract chunked body", chunkedExtracted, json)
failures = failures + check_int("extract chunked status", client.status(), 0)
failures = failures + check_str("extract chunked text", client.extractText(chunkedExtracted), "Hello from fake Ollama")

unicodeText = "Hello " || d2c(128512)
unicodeJson = '{"model":"demo-model","response":' || jsonquote(unicodeText) || ',"done":true}'
unicodeChunkedHttp = "HTTP/1.1 200 OK" || crlf || ,
                     "Content-Type: application/json" || crlf || ,
                     "Transfer-Encoding: chunked" || crlf || ,
                     crlf || ,
                     d2x(utf8_len(unicodeJson)) || crlf || unicodeJson || crlf || ,
                     "0" || crlf || crlf
unicodeExtracted = client.extractBody(unicodeChunkedHttp)
failures = failures + check_str("extract unicode chunked body", unicodeExtracted, unicodeJson)
failures = failures + check_int("extract unicode chunked status", client.status(), 0)
failures = failures + check_str("extract unicode chunked text", client.extractText(unicodeExtracted), unicodeText)

badJson = '{"error":"busy"}'
badHttp = "HTTP/1.1 500 Internal Server Error" || crlf || ,
          "Content-Length: " || utf8_len(badJson) || crlf || ,
          crlf || badJson
badBody = client.extractBody(badHttp)
failures = failures + check_int("http error status", client.status(), 500)
failures = failures + check_int("http error body", pos('"error"', badBody) > 0, 1)

badText = client.extractText("not json")
failures = failures + check_int("json error status", client.status(), -22)

providerError = client.extractText('{"error":"model missing"}')
failures = failures + check_int("provider error status", client.status(), -23)
failures = failures + check_int("provider error message", pos("model missing", client.error()) > 0, 1)

missingResponse = client.extractText('{"done":true}')
failures = failures + check_int("missing response status", client.status(), -24)

if failures <> 0 then do
  say "rxfnsg llm failures:" failures
  exit 1
end

say "PASS: rxfnsg llm ollama helpers"
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
