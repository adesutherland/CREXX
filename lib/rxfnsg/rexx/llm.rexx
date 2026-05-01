options levelg

namespace rxfnsg expose llm ollama openai anthropic gemini
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

openai: class
  _model = .string
  _api_key = .string
  _timeout = .int
  _status = .int
  _error = .string
  _last_json = .string
  _last_http = .string
  _http = .httpclient

  *: factory
    arg model = "gpt-4.1", apiKey = "", timeout = 120000
    _model = _clean_env_value(model)
    _api_key = _clean_env_value(apiKey)
    if _api_key = "" then _api_key = _clean_env_value(getenv("OPENAI_API_KEY"))
    _timeout = timeout
    _status = 0
    _error = ""
    _last_json = ""
    _last_http = ""
    _http = .httpclient("api.openai.com", 443, _timeout, 1)
    return

  generate: method = .string
    arg prompt = .string
    responseJson = generateJson(prompt)
    if _status <> 0 then return ""
    return extractText(responseJson)

  generateJson: method = .string
    arg prompt = .string
    if _api_key = "" then do
      _last_json = ""
      _last_http = ""
      call _set_error -40, "OpenAI API key is not set"
      return ""
    end

    headers = .string[]
    headers[1] = "Authorization: Bearer " || _api_key
    _last_json = _http.postWithHeaders("/v1/responses", buildBody(prompt), "application/json", headers)
    _last_http = _http.lastHttp()
    if _http.status() <> 0 then do
      message = _provider_error_message("OpenAI", _last_json)
      if message = "" then message = _http.error()
      call _set_error _http.status(), message
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
    keys[2] = "input"
    values[2] = jsonquote(prompt)
    return jsonobject(keys, values)

  buildRequest: method = .string
    arg prompt = .string
    headers = .string[]
    headers[1] = "Authorization: Bearer " || _api_key
    return _http.buildRequestWithHeaders("POST", "/v1/responses", buildBody(prompt), "application/json", headers)

  extractBody: method = .string
    arg response = .string
    body = _http.extractBody(response)
    _last_http = _http.lastHttp()
    _last_json = body
    if _http.status() <> 0 then do
      message = _provider_error_message("OpenAI", body)
      if message = "" then message = _http.error()
      call _set_error _http.status(), message
    end
    else call _set_error 0, ""
    return body

  extractText: method = .string
    arg responseJson = .string
    if jsonvalid(responseJson) = 0 then do
      call _set_error -42, "OpenAI response is not valid JSON"
      return ""
    end

    message = _provider_error_message("OpenAI", responseJson)
    if message <> "" then do
      call _set_error -43, message
      return ""
    end

    if jsontype(responseJson, "output_text") = "string" then do
      text = jsonget(responseJson, "output_text")
      call _set_error 0, ""
      return text
    end

    if jsontype(responseJson, "output.1.content.1.text") = "string" then do
      text = jsonget(responseJson, "output.1.content.1.text")
      call _set_error 0, ""
      return text
    end

    call _set_error -44, "OpenAI JSON has no generated text field"
    return ""

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

anthropic: class
  _model = .string
  _api_key = .string
  _timeout = .int
  _status = .int
  _error = .string
  _last_json = .string
  _last_http = .string
  _http = .httpclient

  *: factory
    arg model = "claude-sonnet-4-5", apiKey = "", timeout = 120000
    _model = _clean_env_value(model)
    _api_key = _clean_env_value(apiKey)
    if _api_key = "" then _api_key = _clean_env_value(getenv("ANTHROPIC_API_KEY"))
    _timeout = timeout
    _status = 0
    _error = ""
    _last_json = ""
    _last_http = ""
    _http = .httpclient("api.anthropic.com", 443, _timeout, 1)
    return

  generate: method = .string
    arg prompt = .string
    responseJson = generateJson(prompt)
    if _status <> 0 then return ""
    return extractText(responseJson)

  generateJson: method = .string
    arg prompt = .string
    if _api_key = "" then do
      _last_json = ""
      _last_http = ""
      call _set_error -50, "Anthropic API key is not set"
      return ""
    end

    headers = .string[]
    headers[1] = "x-api-key: " || _api_key
    headers[2] = "anthropic-version: 2023-06-01"
    _last_json = _http.postWithHeaders("/v1/messages", buildBody(prompt), "application/json", headers)
    _last_http = _http.lastHttp()
    if _http.status() <> 0 then do
      message = _provider_error_message("Anthropic", _last_json)
      if message = "" then message = _http.error()
      call _set_error _http.status(), message
      return _last_json
    end
    call _set_error 0, ""
    return _last_json

  buildBody: method = .string
    arg prompt = .string
    msg_keys = .string[]
    msg_values = .string[]
    messages = .string[]
    keys = .string[]
    values = .string[]

    msg_keys[1] = "role"
    msg_values[1] = jsonquote("user")
    msg_keys[2] = "content"
    msg_values[2] = jsonquote(prompt)
    messages[1] = jsonobject(msg_keys, msg_values)

    keys[1] = "model"
    values[1] = jsonquote(_model)
    keys[2] = "max_tokens"
    values[2] = "1024"
    keys[3] = "messages"
    values[3] = jsonarray(messages)
    return jsonobject(keys, values)

  buildRequest: method = .string
    arg prompt = .string
    headers = .string[]
    headers[1] = "x-api-key: " || _api_key
    headers[2] = "anthropic-version: 2023-06-01"
    return _http.buildRequestWithHeaders("POST", "/v1/messages", buildBody(prompt), "application/json", headers)

  extractBody: method = .string
    arg response = .string
    body = _http.extractBody(response)
    _last_http = _http.lastHttp()
    _last_json = body
    if _http.status() <> 0 then do
      message = _provider_error_message("Anthropic", body)
      if message = "" then message = _http.error()
      call _set_error _http.status(), message
    end
    else call _set_error 0, ""
    return body

  extractText: method = .string
    arg responseJson = .string
    if jsonvalid(responseJson) = 0 then do
      call _set_error -52, "Anthropic response is not valid JSON"
      return ""
    end

    message = _provider_error_message("Anthropic", responseJson)
    if message <> "" then do
      call _set_error -53, message
      return ""
    end

    if jsontype(responseJson, "content.1.text") = "string" then do
      text = jsonget(responseJson, "content.1.text")
      call _set_error 0, ""
      return text
    end

    call _set_error -54, "Anthropic JSON has no generated text field"
    return ""

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

gemini: class
  _model = .string
  _api_key = .string
  _timeout = .int
  _status = .int
  _error = .string
  _last_json = .string
  _last_http = .string
  _http = .httpclient

  *: factory
    arg model = "gemini-2.5-flash", apiKey = "", timeout = 120000
    _model = _clean_env_value(model)
    _api_key = _clean_env_value(apiKey)
    if _api_key = "" then _api_key = _clean_env_value(getenv("GOOGLE_API_KEY"))
    if _api_key = "" then _api_key = _clean_env_value(getenv("GEMINI_API_KEY"))
    _timeout = timeout
    _status = 0
    _error = ""
    _last_json = ""
    _last_http = ""
    _http = .httpclient("generativelanguage.googleapis.com", 443, _timeout, 1)
    return

  generate: method = .string
    arg prompt = .string
    responseJson = generateJson(prompt)
    if _status <> 0 then return ""
    return extractText(responseJson)

  generateJson: method = .string
    arg prompt = .string
    if _api_key = "" then do
      _last_json = ""
      _last_http = ""
      call _set_error -60, "Google/Gemini API key is not set"
      return ""
    end

    headers = .string[]
    headers[1] = "x-goog-api-key: " || _api_key
    _last_json = _http.postWithHeaders(_path(), buildBody(prompt), "application/json", headers)
    _last_http = _http.lastHttp()
    if _http.status() <> 0 then do
      message = _provider_error_message("Gemini", _last_json)
      if message = "" then message = _http.error()
      call _set_error _http.status(), message
      return _last_json
    end
    call _set_error 0, ""
    return _last_json

  buildBody: method = .string
    arg prompt = .string
    part_keys = .string[]
    part_values = .string[]
    parts = .string[]
    content_keys = .string[]
    content_values = .string[]
    contents = .string[]
    keys = .string[]
    values = .string[]

    part_keys[1] = "text"
    part_values[1] = jsonquote(prompt)
    parts[1] = jsonobject(part_keys, part_values)

    content_keys[1] = "parts"
    content_values[1] = jsonarray(parts)
    contents[1] = jsonobject(content_keys, content_values)

    keys[1] = "contents"
    values[1] = jsonarray(contents)
    return jsonobject(keys, values)

  buildRequest: method = .string
    arg prompt = .string
    headers = .string[]
    headers[1] = "x-goog-api-key: " || _api_key
    return _http.buildRequestWithHeaders("POST", _path(), buildBody(prompt), "application/json", headers)

  extractBody: method = .string
    arg response = .string
    body = _http.extractBody(response)
    _last_http = _http.lastHttp()
    _last_json = body
    if _http.status() <> 0 then do
      message = _provider_error_message("Gemini", body)
      if message = "" then message = _http.error()
      call _set_error _http.status(), message
    end
    else call _set_error 0, ""
    return body

  extractText: method = .string
    arg responseJson = .string
    if jsonvalid(responseJson) = 0 then do
      call _set_error -62, "Gemini response is not valid JSON"
      return ""
    end

    message = _provider_error_message("Gemini", responseJson)
    if message <> "" then do
      call _set_error -63, message
      return ""
    end

    if jsontype(responseJson, "candidates.1.content.parts.1.text") = "string" then do
      text = jsonget(responseJson, "candidates.1.content.parts.1.text")
      call _set_error 0, ""
      return text
    end

    call _set_error -64, "Gemini JSON has no generated text field"
    return ""

  status: method = .int
    return _status

  error: method = .string
    return _error

  lastJson: method = .string
    return _last_json

  lastHttp: method = .string
    return _last_http

  _path: method = .string
    if left(_model, 7) = "models/" then return "/v1beta/" || _model || ":generateContent"
    return "/v1beta/models/" || _model || ":generateContent"

  _set_error: method
    arg code = .int, message = .string
    _status = code
    _error = message
    return

_provider_error_message: procedure = .string
  arg provider = .string, responseJson = .string
  if responseJson = "" then return ""
  if jsonvalid(responseJson) = 0 then return ""
  if jsontype(responseJson, "error.message") = "string" then return provider || " error: " || jsonget(responseJson, "error.message")
  if jsontype(responseJson, "error") = "string" then return provider || " error: " || jsonget(responseJson, "error")
  return ""

_clean_env_value: procedure = .string
  arg value = .string
  value = changestr("0d"x, value, "")
  value = changestr("0a"x, value, "")
  return strip(value)
