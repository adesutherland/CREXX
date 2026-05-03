#!/usr/local/crexx/rexx.sh
options levelg
import rxfnsb
import rxfnsg

arg args = .string[]

prompt = "Reply with exactly: cREXX socket Anthropic LLM demo OK."
model = getenv("ANTHROPIC_MODEL")
apiKey = getenv("ANTHROPIC_API_KEY")

if model = "" then model = "claude-sonnet-4-5"
if args.0 >= 1 then prompt = args[1]
if args.0 >= 2 then model = args[2]

if apiKey = "" then do
  say "ANTHROPIC_API_KEY is not set"
  exit 2
end

client = .anthropic(model, apiKey)
json = client.generateJson(prompt)
if client.status() <> 0 then do
  say "Anthropic request failed:" client.status() client.error()
  if json <> "" then do
    say "Anthropic JSON:"
    say json
  end
  exit 1
end

answer = client.extractText(json)
if client.status() <> 0 then do
  say "Anthropic response parse failed:" client.status() client.error()
  if json <> "" then do
    say "Anthropic JSON:"
    say json
  end
  exit 1
end

say answer
exit 0
