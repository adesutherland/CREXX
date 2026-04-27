#!/usr/local/crexx/rexx.sh
options levelg
import rxfnsb
import rxfnsg

arg args = .string[]

prompt = "Reply with exactly: cREXX socket LLM demo OK."
model = "gemma4:latest"

if args.0 >= 1 then prompt = args[1]
if args.0 >= 2 then model = args[2]

client = .llm(model)
json = client.generateJson(prompt)
if client.status() <> 0 then do
  say "Ollama request failed:" client.status() client.error()
  if json <> "" then do
    say "Ollama JSON:"
    say json
  end
  exit 1
end

answer = client.extractText(json)
if client.status() <> 0 then do
  say "Ollama response parse failed:" client.status() client.error()
  if json <> "" then do
    say "Ollama JSON:"
    say json
  end
  exit 1
end

say answer
exit 0
