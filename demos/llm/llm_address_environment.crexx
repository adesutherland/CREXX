/* LLM ADDRESS environment demonstrator.
 *
 * This provider routes ADDRESS environment names such as GPT_4_1,
 * CLAUDE_SONNET_4_5, GEMINI_2_5_FLASH, GEMMA4_LATEST, and LLM_OLLAMA to
 * the Rexx Level G rxfnsg LLM clients.
 */
options levelg
namespace _rxsysb expose llmaddressenvironment
import _rxsysb
import rxfnsb
import rxfnsg

llmaddressenvironment: class implements .addressenvironment .addressinstance .addressfunctionenvironment
  _driver = .string
  _environment_id = .string
  _environment_name = .string
  _error = .string
  _model = .string
  _status = .int

  *: match
    arg env_name = .string
    if llm_driver_for_environment(env_name) \= "" then return 90
    return 0

  *: factory
    arg env_name = .string
    _environment_name = _address_normalize_environment_name(env_name)
    _driver = llm_driver_for_environment(_environment_name)
    _model = llm_model_for_environment(_environment_name, _driver)
    _environment_id = "rexx:" || _environment_name || ":" || _driver
    _status = 0
    _error = ""
    return

  bind_environment: method = .void
    arg env_name = .string, instance_id = .string
    _environment_name = _address_normalize_environment_name(env_name)
    _driver = llm_driver_for_environment(_environment_name)
    _model = llm_model_for_environment(_environment_name, _driver)
    _environment_id = instance_id
    if _environment_id = "" then _environment_id = "rexx:" || _environment_name || ":" || _driver
    _status = 0
    _error = ""
    return

  environment_name: method = .string
    return _environment_name

  environment_id: method = .string
    return _environment_id

  execute: method = .addressresponse
    arg request = .addressrequest

    command = .string
    normalized = .string
    target = .string
    payload = .string
    result = .string

    command = llm_command_without_into(request.get_command())
    target = llm_into_anchor(request.get_command())
    normalized = _address_normalize_command(command)

    if normalized = "INFO" then return llm_complete(request, "DRIVER=" || _driver || " MODEL=" || _model, target)
    if normalized = "DRIVERS" then return llm_complete(request, llm_supported_drivers(), target)
    if normalized = "STATUS" then return llm_complete(request, "" || (_status + 0), target)
    if normalized = "ERROR" then return llm_complete(request, _error, target)

    if normalized = "MODEL" then return llm_complete(request, _model, target)
    if _address_first_chars(normalized, 6) = "MODEL " then do
      payload = strip(substr(command, 7))
      _model = llm_resolve_host_text(request, payload)
      return llm_complete(request, _model, target)
    end

    if _address_first_chars(normalized, 4) = "BODY" then do
      payload = llm_resolve_host_text(request, strip(substr(command, 5)))
      result = _body(payload)
      return llm_finish_command(request, result, target)
    end

    if _address_first_chars(normalized, 7) = "REQUEST" then do
      payload = llm_resolve_host_text(request, strip(substr(command, 8)))
      result = _request(payload)
      return llm_finish_command(request, result, target)
    end

    if _address_first_chars(normalized, 7) = "EXTRACT" then do
      payload = llm_resolve_host_text(request, strip(substr(command, 8)))
      result = _extract(payload)
      return llm_finish_command(request, result, target)
    end

    if _address_first_chars(normalized, 8) = "GENERATE" then do
      payload = llm_resolve_host_text(request, strip(substr(command, 9)))
      result = _generate(payload)
      return llm_finish_command(request, result, target)
    end

    return _address_unknown_command_response(_environment_name, request.get_command())

  invoke: method = .addressfunctionresponse
    arg request = .addressfunctionrequest

    function_name = .string
    result = .string

    function_name = _address_normalize_environment_name(request.get_function_name())
    if function_name = "ID" then return .addressfunctionresponse(0, _environment_id)
    if function_name = "NAME" then return .addressfunctionresponse(0, _environment_name)
    if function_name = "DRIVER" then return .addressfunctionresponse(0, _driver)
    if function_name = "DRIVERS" then return .addressfunctionresponse(0, llm_supported_drivers())
    if function_name = "MODEL" then return .addressfunctionresponse(0, _model)
    if function_name = "STATUS" then return .addressfunctionresponse(0, "" || (_status + 0))
    if function_name = "ERROR" then return .addressfunctionresponse(0, _error)

    if function_name = "SET_MODEL" then do
      _model = request.get_argument(1)
      return .addressfunctionresponse(0, _model)
    end

    if function_name = "BODY" then do
      result = _body(request.get_argument(1))
      return llm_function_result(_status, result, _error)
    end

    if function_name = "REQUEST" then do
      result = _request(request.get_argument(1))
      return llm_function_result(_status, result, _error)
    end

    if function_name = "EXTRACT" then do
      result = _extract(request.get_argument(1))
      return llm_function_result(_status, result, _error)
    end

    if function_name = "GENERATE" then do
      result = _generate(request.get_argument(1))
      return llm_function_result(_status, result, _error)
    end

    return llm_function_result(-3, "", "Unknown LLM function " || request.get_function_name())

  _body: method = .string
    arg prompt = .string
    _status = 0
    _error = ""
    if _driver = "OPENAI" then do
      client = .openai(_model, "demo-key")
      return client.buildBody(prompt)
    end
    if _driver = "ANTHROPIC" then do
      client = .anthropic(_model, "demo-key")
      return client.buildBody(prompt)
    end
    if _driver = "GEMINI" then do
      client = .gemini(_model, "demo-key")
      return client.buildBody(prompt)
    end
    client = .llm(_model)
    return client.buildBody(prompt)

  _request: method = .string
    arg prompt = .string
    _status = 0
    _error = ""
    if _driver = "OPENAI" then do
      client = .openai(_model, "demo-key")
      return llm_mask_request(client.buildRequest(prompt))
    end
    if _driver = "ANTHROPIC" then do
      client = .anthropic(_model, "demo-key")
      return llm_mask_request(client.buildRequest(prompt))
    end
    if _driver = "GEMINI" then do
      client = .gemini(_model, "demo-key")
      return llm_mask_request(client.buildRequest(prompt))
    end
    client = .llm(_model)
    return client.buildRequest(prompt)

  _extract: method = .string
    arg response_json = .string
    result = .string
    if _driver = "OPENAI" then do
      client = .openai(_model, "demo-key")
      result = client.extractText(response_json)
      _status = client.status()
      _error = client.error()
      return result
    end
    if _driver = "ANTHROPIC" then do
      client = .anthropic(_model, "demo-key")
      result = client.extractText(response_json)
      _status = client.status()
      _error = client.error()
      return result
    end
    if _driver = "GEMINI" then do
      client = .gemini(_model, "demo-key")
      result = client.extractText(response_json)
      _status = client.status()
      _error = client.error()
      return result
    end
    client = .llm(_model)
    result = client.extractText(response_json)
    _status = client.status()
    _error = client.error()
    return result

  _generate: method = .string
    arg prompt = .string
    response_json = .string
    result = .string
    if _driver = "OPENAI" then do
      client = .openai(_model)
      response_json = client.generateJson(prompt)
      if client.status() <> 0 then do
        _status = client.status()
        _error = client.error()
        return ""
      end
      result = client.extractText(response_json)
      _status = client.status()
      _error = client.error()
      return result
    end
    if _driver = "ANTHROPIC" then do
      client = .anthropic(_model)
      response_json = client.generateJson(prompt)
      if client.status() <> 0 then do
        _status = client.status()
        _error = client.error()
        return ""
      end
      result = client.extractText(response_json)
      _status = client.status()
      _error = client.error()
      return result
    end
    if _driver = "GEMINI" then do
      client = .gemini(_model)
      response_json = client.generateJson(prompt)
      if client.status() <> 0 then do
        _status = client.status()
        _error = client.error()
        return ""
      end
      result = client.extractText(response_json)
      _status = client.status()
      _error = client.error()
      return result
    end
    client = .llm(_model)
    response_json = client.generateJson(prompt)
    if client.status() <> 0 then do
      _status = client.status()
      _error = client.error()
      return ""
    end
    result = client.extractText(response_json)
    _status = client.status()
    _error = client.error()
    return result

llm_finish_command: procedure = .addressresponse
  arg request = .addressrequest, result = .string, target = .string
  return llm_complete(request, result, target)

llm_complete: procedure = .addressresponse
  arg request = .addressrequest, value = .string, target = .string
  response = .addressresponse
  response = .addressresponse(0)
  if target \= "" then do
    call response.add_updated_binding("var", target, target, value, "")
    return response
  end
  return llm_emit_text(request, value)

llm_emit_text: procedure = .addressresponse
  arg request = .addressrequest, text = .string
  return _address_execute_system_command(request, "printf '%s\n' " || llm_shell_quote(text))

llm_function_result: procedure = .addressfunctionresponse
  arg code = .int, result = .string, message = .string
  response = .addressfunctionresponse
  response = .addressfunctionresponse(code, result)
  if code \= 0 then do
    call response.set_condition_name("FAILURE")
    if message \= "" then call response.add_diagnostic(message)
  end
  return response

llm_driver_registry: procedure = .addressdriverregistry
  registry = .addressdriverregistry
  registry = .addressdriverregistry()
  call llm_register_ollama_driver(registry)
  call llm_register_openai_driver(registry)
  call llm_register_anthropic_driver(registry)
  call llm_register_gemini_driver(registry)
  return registry

llm_register_ollama_driver: procedure = .void
  arg registry = .addressdriverregistry
  call registry.add("OLLAMA", "LLM OLLAMA GEMMA4 GEMMA4_LATEST", "GEMMA LLAMA MISTRAL")
  return

llm_register_openai_driver: procedure = .void
  arg registry = .addressdriverregistry
  call registry.add("OPENAI", "OPENAI GPT GPT_4_1 GPT_4O", "GPT_ O1 O3 O4")
  return

llm_register_anthropic_driver: procedure = .void
  arg registry = .addressdriverregistry
  call registry.add("ANTHROPIC", "ANTHROPIC CLAUDE CLAUDE_SONNET_4_5", "CLAUDE_")
  return

llm_register_gemini_driver: procedure = .void
  arg registry = .addressdriverregistry
  call registry.add("GEMINI", "GEMINI GEMINI_2_5_FLASH", "GEMINI_")
  return

llm_driver_for_environment: procedure = .string
  arg env_name = .string
  alias = .string
  alias = llm_environment_alias(env_name)
  return llm_lookup_driver(alias)

llm_lookup_driver: procedure = .string
  arg alias = .string
  registry = .addressdriverregistry
  registry = llm_driver_registry()
  return registry.lookup(alias)

llm_model_for_environment: procedure = .string
  arg env_name = .string, driver = .string
  alias = .string
  configured = .string

  alias = llm_environment_alias(env_name)

  if driver = "OPENAI" then do
    configured = llm_clean_env_value(getenv("OPENAI_MODEL"))
    if configured \= "" & alias = "OPENAI" then return configured
    if alias = "GPT_4_1" | alias = "GPT" | alias = "OPENAI" then return "gpt-4.1"
    if alias = "GPT_4O" then return "gpt-4o"
    return llm_alias_to_model(alias)
  end

  if driver = "ANTHROPIC" then do
    configured = llm_clean_env_value(getenv("ANTHROPIC_MODEL"))
    if configured \= "" & alias = "ANTHROPIC" then return configured
    if alias = "CLAUDE_SONNET_4_5" | alias = "CLAUDE" | alias = "ANTHROPIC" then return "claude-sonnet-4-5"
    return llm_alias_to_model(alias)
  end

  if driver = "GEMINI" then do
    configured = llm_clean_env_value(getenv("GEMINI_MODEL"))
    if configured \= "" & alias = "GEMINI" then return configured
    if alias = "GEMINI_2_5_FLASH" | alias = "GEMINI" then return "gemini-2.5-flash"
    return llm_alias_to_model(alias)
  end

  configured = llm_clean_env_value(getenv("OLLAMA_MODEL"))
  if configured \= "" & (alias = "LLM" | alias = "OLLAMA") then return configured
  if alias = "GEMMA4" | alias = "GEMMA4_LATEST" | alias = "LLM" | alias = "OLLAMA" then return "gemma4:latest"
  return llm_alias_to_ollama_model(alias)

llm_environment_alias: procedure = .string
  arg env_name = .string
  alias = .string
  alias = _address_normalize_environment_name(env_name)
  if _address_first_chars(alias, 4) = "LLM_" then alias = substr(alias, 5)
  return alias

llm_alias_to_model: procedure = .string
  arg alias = .string
  return lower(changestr("_", alias, "-"))

llm_alias_to_ollama_model: procedure = .string
  arg alias = .string
  if right(alias, 7) = "_LATEST" then return lower(changestr("_", substr(alias, 1, length(alias) - 7), "-")) || ":latest"
  return lower(changestr("_", alias, "-"))

llm_supported_drivers: procedure = .string
  registry = .addressdriverregistry
  registry = llm_driver_registry()
  return registry.drivers()

llm_command_without_into: procedure = .string
  arg command = .string
  marker_pos = .int
  marker_pos = llm_into_marker_pos(command)
  if marker_pos = 0 then return strip(command)
  return strip(substr(command, 1, marker_pos - 1))

llm_into_anchor: procedure = .string
  arg command = .string
  marker_pos = .int
  token = .string
  marker_pos = llm_into_marker_pos(command)
  if marker_pos = 0 then return ""
  token = word(strip(substr(command, marker_pos + 6)), 1)
  return llm_host_anchor_name(token)

llm_into_marker_pos: procedure = .int
  arg command = .string
  marker_pos = .int
  token = .string

  marker_pos = lastpos(" INTO ", upper(command))
  if marker_pos = 0 then return 0

  token = word(strip(substr(command, marker_pos + 6)), 1)
  if llm_host_anchor_name(token) = "" then return 0
  return marker_pos

llm_resolve_host_text: procedure = .string
  arg request = .addressrequest, text = .string
  anchor_name = .string
  anchor_name = llm_host_anchor_name(strip(text))
  if anchor_name = "" then return text
  return request.get_binding_value(anchor_name)

llm_host_anchor_name: procedure = .string
  arg token = .string
  name = .string

  if length(token) > 1 & left(token, 1) = ":" then do
    name = substr(token, 2)
    if llm_host_anchor_name_ok(name) = 1 then return name
  end

  if length(token) > 3 & left(token, 2) = "${" & right(token, 1) = "}" then do
    name = substr(token, 3, length(token) - 3)
    if llm_host_anchor_name_ok(name) = 1 then return name
  end

  return ""

llm_host_anchor_name_ok: procedure = .int
  arg name = .string
  if name = "" then return 0
  do i = 1 to length(name)
    ch = substr(name, i, 1)
    if pos(ch, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.") = 0 then return 0
  end
  return 1

llm_shell_quote: procedure = .string
  arg text = .string
  quote = .string
  quote = d2c(39)
  return quote || changestr(quote, text, quote || '"' || quote || '"' || quote) || quote

llm_mask_request: procedure = .string
  arg request = .string
  masked = .string
  masked = changestr("Bearer demo-key", request, "Bearer <redacted>")
  masked = changestr("x-api-key: demo-key", masked, "x-api-key: <redacted>")
  masked = changestr("x-goog-api-key: demo-key", masked, "x-goog-api-key: <redacted>")
  return masked

llm_clean_env_value: procedure = .string
  arg value = .string
  value = changestr("0d"x, value, "")
  value = changestr("0a"x, value, "")
  return strip(value)
