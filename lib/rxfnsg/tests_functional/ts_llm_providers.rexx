options levelg
import rxfnsg
import rxfnsb
import rxjson

failures = 0

openaiClient = .openai(" gpt-test ", "0a"x || " openai-test-key " || "0d"x, 1000)
anthropicClient = .anthropic(" claude-test ", "0a"x || " anthropic-test-key " || "0d"x, 1000)
geminiClient = .gemini(" gemini-test ", "0a"x || " gemini-test-key " || "0d"x, 1000)

openaiBody = openaiClient.buildBody("Say hi")
failures = failures + check_int("openai body valid", jsonvalid(openaiBody), 1)
failures = failures + check_str("openai body model", jsonget(openaiBody, "model"), "gpt-test")
failures = failures + check_str("openai body input", jsonget(openaiBody, "input"), "Say hi")
openaiRequest = openaiClient.buildRequest("Say hi")
failures = failures + check_int("openai request path", left(openaiRequest, 23) = "POST /v1/responses HTTP", 1)
failures = failures + check_int("openai request host", pos("Host: api.openai.com:443", openaiRequest) > 0, 1)
failures = failures + check_int("openai auth header", pos("Authorization: Bearer openai-test-key", openaiRequest) > 0, 1)
failures = failures + check_int("openai content type", pos("Content-Type: application/json", openaiRequest) > 0, 1)
failures = failures + check_int("openai content length", pos("Content-Length: " || utf8_len(openaiBody), openaiRequest) > 0, 1)
openaiJson = '{"output":[{"content":[{"type":"output_text","text":"Hello OpenAI"}]}]}'
failures = failures + check_str("openai extract text", openaiClient.extractText(openaiJson), "Hello OpenAI")
failures = failures + check_int("openai extract status", openaiClient.status(), 0)
openaiTopJson = '{"output_text":"Hello top OpenAI"}'
failures = failures + check_str("openai output text", openaiClient.extractText(openaiTopJson), "Hello top OpenAI")
openaiError = openaiClient.extractText('{"error":{"message":"bad OpenAI key"}}')
failures = failures + check_int("openai error status", openaiClient.status(), -43)
failures = failures + check_int("openai error message", pos("bad OpenAI key", openaiClient.error()) > 0, 1)

anthropicBody = anthropicClient.buildBody("Say hi")
failures = failures + check_int("anthropic body valid", jsonvalid(anthropicBody), 1)
failures = failures + check_str("anthropic body model", jsonget(anthropicBody, "model"), "claude-test")
failures = failures + check_str("anthropic body prompt", jsonget(anthropicBody, "messages.1.content"), "Say hi")
failures = failures + check_str("anthropic body max tokens", jsonget(anthropicBody, "max_tokens"), "1024")
anthropicRequest = anthropicClient.buildRequest("Say hi")
failures = failures + check_int("anthropic request path", left(anthropicRequest, 22) = "POST /v1/messages HTTP", 1)
failures = failures + check_int("anthropic request host", pos("Host: api.anthropic.com:443", anthropicRequest) > 0, 1)
failures = failures + check_int("anthropic auth header", pos("x-api-key: anthropic-test-key", anthropicRequest) > 0, 1)
failures = failures + check_int("anthropic version header", pos("anthropic-version: 2023-06-01", anthropicRequest) > 0, 1)
anthropicJson = '{"content":[{"type":"text","text":"Hello Claude"}]}'
failures = failures + check_str("anthropic extract text", anthropicClient.extractText(anthropicJson), "Hello Claude")
failures = failures + check_int("anthropic extract status", anthropicClient.status(), 0)
anthropicError = anthropicClient.extractText('{"error":{"message":"bad Claude key"}}')
failures = failures + check_int("anthropic error status", anthropicClient.status(), -53)
failures = failures + check_int("anthropic error message", pos("bad Claude key", anthropicClient.error()) > 0, 1)

geminiBody = geminiClient.buildBody("Say hi")
failures = failures + check_int("gemini body valid", jsonvalid(geminiBody), 1)
failures = failures + check_str("gemini body prompt", jsonget(geminiBody, "contents.1.parts.1.text"), "Say hi")
geminiRequest = geminiClient.buildRequest("Say hi")
failures = failures + check_int("gemini request path", pos("POST /v1beta/models/gemini-test:generateContent HTTP", geminiRequest) > 0, 1)
failures = failures + check_int("gemini request host", pos("Host: generativelanguage.googleapis.com:443", geminiRequest) > 0, 1)
failures = failures + check_int("gemini auth header", pos("x-goog-api-key: gemini-test-key", geminiRequest) > 0, 1)
geminiJson = '{"candidates":[{"content":{"parts":[{"text":"Hello Gemini"}]}}]}'
failures = failures + check_str("gemini extract text", geminiClient.extractText(geminiJson), "Hello Gemini")
failures = failures + check_int("gemini extract status", geminiClient.status(), 0)
geminiError = geminiClient.extractText('{"error":{"message":"bad Gemini key"}}')
failures = failures + check_int("gemini error status", geminiClient.status(), -63)
failures = failures + check_int("gemini error message", pos("bad Gemini key", geminiClient.error()) > 0, 1)

if failures <> 0 then do
  say "rxfnsg llm provider failures:" failures
  exit 1
end

say "PASS: rxfnsg llm provider helpers"
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
