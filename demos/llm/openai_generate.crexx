#!/usr/local/crexx/rexx.sh
options levelg
import rxfnsb
import rxfnsg

arg args = .string[]

prompt = "Reply with exactly: cREXX socket OpenAI LLM demo OK."
model = getenv("OPENAI_MODEL")
apiKey = getenv("OPENAI_API_KEY")

if model = "" then model = "gpt-4.1"
if args.0 >= 1 then prompt = args[1]
if args.0 >= 2 then model = args[2]

if apiKey = "" then do
  say "OPENAI_API_KEY is not set"
  exit 2
end

client = .openai(model, apiKey)
json = client.generateJson(prompt)
if client.status() <> 0 then do
  say "OpenAI request failed:" client.status() client.error()
  if json <> "" then do
    say "OpenAI JSON:"
    say json
  end
  exit 1
end

answer = client.extractText(json)
if client.status() <> 0 then do
  say "OpenAI response parse failed:" client.status() client.error()
  if json <> "" then do
    say "OpenAI JSON:"
    say json
  end
  exit 1
end

say answer
exit 0
