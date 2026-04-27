options levelg

namespace rxfnsg expose llm ollama
import rxfnsb
import rxjson
import rxhttp

llm: interface
  *: factory
    arg model = "gemma4:latest", host = "127.0.0.1", port = 11434, timeout = 120000

  generate: method = .string
    arg prompt = .string

  generateJson: method = .string
    arg prompt = .string

  buildBody: method = .string
    arg prompt = .string

  buildRequest: method = .string
    arg prompt = .string

  extractBody: method = .string
    arg response = .string

  extractText: method = .string
    arg responseJson = .string

  status: method = .int

  error: method = .string

  lastJson: method = .string

  lastHttp: method = .string

ollama: class implements .llm
  _model = .string
  _host = .string
  _port = .int
  _timeout = .int
  _status = .int
  _error = .string
  _last_json = .string
  _last_http = .string
  _http = .httpclient

  *: match
    arg model = "gemma4:latest", host = "127.0.0.1", port = 11434, timeout = 120000
    if port = 11434 then return 100
    return 10

  *: factory
    arg model = "gemma4:latest", host = "127.0.0.1", port = 11434, timeout = 120000
    _model = model
    _host = host
    _port = port
    _timeout = timeout
    _status = 0
    _error = ""
    _last_json = ""
    _last_http = ""
    _http = .httpclient(_host, _port, _timeout)
    return

  generate: method = .string
    arg prompt = .string
    responseJson = generateJson(prompt)
    if _status <> 0 then return ""
    return extractText(responseJson)

  generateJson: method = .string
    arg prompt = .string
    _last_json = ""
    _last_json = _http.post("/api/generate", buildBody(prompt), "application/json")
    _last_http = _http.lastHttp()
    if _http.status() <> 0 then do
      call _set_error _http.status(), _http.error()
      return _last_json
    end
    call _set_error 0, ""
    return _last_json

  buildBody: method = .string
    arg prompt = .string
    keys = .string[]
    values = .string[]

    keys[1] = "model"
    values[1] = jsonquote(_model)
    keys[2] = "prompt"
    values[2] = jsonquote(prompt)
    keys[3] = "stream"
    values[3] = "false"

    return jsonobject(keys, values)

  buildRequest: method = .string
    arg prompt = .string
    return _http.buildRequest("POST", "/api/generate", buildBody(prompt), "application/json")

  extractBody: method = .string
    arg response = .string
    body = _http.extractBody(response)
    _last_http = _http.lastHttp()
    _last_json = body
    if _http.status() <> 0 then call _set_error _http.status(), _http.error()
    else call _set_error 0, ""
    return body

  extractText: method = .string
    arg responseJson = .string
    if jsonvalid(responseJson) = 0 then do
      call _set_error -22, "Ollama response is not valid JSON"
      return ""
    end

    if jsontype(responseJson, "error") = "string" then do
      call _set_error -23, "Ollama error: " || jsonget(responseJson, "error")
      return ""
    end

    if jsontype(responseJson, "response") \= "string" then do
      call _set_error -24, "Ollama JSON has no string response field"
      return ""
    end

    text = jsonget(responseJson, "response")
    call _set_error 0, ""
    return text

  status: method = .int
    return _status

  error: method = .string
    return _error

  lastJson: method = .string
    return _last_json

  lastHttp: method = .string
    return _last_http

  _set_error: method
    arg code = .int, message = .string
    _status = code
    _error = message
    return
