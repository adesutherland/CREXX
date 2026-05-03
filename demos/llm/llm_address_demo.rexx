#!/usr/local/crexx/rexx.sh
options levelg
import rxfnsb
import rxfnsg

arg args = .string[]

prompt = .string
ran = .int
failures = .int
answer = .string
provider_status = .string
provider_error = .string

prompt = "Reply with one short cREXX greeting."
if args.0 >= 1 then prompt = args[1]

ran = 0
failures = 0

if has_env("OPENAI_API_KEY") = 1 then do
  ran = ran + 1
  answer = ""
  provider_status = ""
  provider_error = ""

  say ""
  say "== OpenAI via ADDRESS LLM_GPT_4_1"
  address llm_gpt_4_1
  "GENERATE :prompt INTO ${answer}"
  "STATUS INTO ${provider_status}"
  "ERROR INTO ${provider_error}"
  failures = failures + print_result(provider_status, provider_error, answer)
end

if has_env("ANTHROPIC_API_KEY") = 1 then do
  ran = ran + 1
  answer = ""
  provider_status = ""
  provider_error = ""

  say ""
  say "== Anthropic via ADDRESS CLAUDE_SONNET_4_5"
  address claude_sonnet_4_5
  "GENERATE :prompt INTO ${answer}"
  "STATUS INTO ${provider_status}"
  "ERROR INTO ${provider_error}"
  failures = failures + print_result(provider_status, provider_error, answer)
end

if has_env("GOOGLE_API_KEY") = 1 | has_env("GEMINI_API_KEY") = 1 then do
  ran = ran + 1
  answer = ""
  provider_status = ""
  provider_error = ""

  say ""
  say "== Gemini via ADDRESS GEMINI_2_5_FLASH"
  address gemini_2_5_flash
  "GENERATE :prompt INTO ${answer}"
  "STATUS INTO ${provider_status}"
  "ERROR INTO ${provider_error}"
  failures = failures + print_result(provider_status, provider_error, answer)
end

#if has_env("OLLAMA_MODEL") = 1 then do
  ran = ran + 1
  answer = ""
  provider_status = ""
  provider_error = ""

  say ""
  say "== Ollama via ADDRESS LLM"
  address llm
  "GENERATE :prompt INTO ${answer}"
  "STATUS INTO ${provider_status}"
  "ERROR INTO ${provider_error}"
  failures = failures + print_result(provider_status, provider_error, answer)
#end

if ran = 0 then do
  say "No LLM provider keys found."
  say "Set OPENAI_API_KEY, ANTHROPIC_API_KEY, GOOGLE_API_KEY/GEMINI_API_KEY, or OLLAMA_MODEL."
  exit 2
end

if failures <> 0 then exit 1
exit 0

print_result: procedure = .int
  arg provider_status = .string, provider_error = .string, answer = .string

  if provider_status = "0" then do
    say answer
    return 0
  end

  if provider_status = "" then provider_status = "(empty)"
  if provider_error = "" then provider_error = "(no diagnostic)"
  say "FAILED status=" || provider_status || " error=" || provider_error
  return 1

has_env: procedure = .int
  arg name = .string
  value = .string

  value = getenv(name)
  value = changestr("0d"x, value, "")
  value = changestr("0a"x, value, "")
  if strip(value) = "" then return 0
  return 1
