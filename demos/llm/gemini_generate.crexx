#!/usr/local/crexx/rexx.sh
options levelg
import rxfnsb
import rxfnsg

arg args = .string[]

prompt = "Reply with exactly: cREXX socket Gemini LLM demo OK."
model = getenv("GEMINI_MODEL")
apiKey = getenv("GOOGLE_API_KEY")
if apiKey = "" then apiKey = getenv("GEMINI_API_KEY")

if model = "" then model = "gemini-2.5-flash"
if args.0 >= 1 then prompt = args[1]
if args.0 >= 2 then model = args[2]

if apiKey = "" then do
  say "GOOGLE_API_KEY or GEMINI_API_KEY is not set"
  exit 2
end

client = .gemini(model, apiKey)
json = client.generateJson(prompt)
if client.status() <> 0 then do
  say "Gemini request failed:" client.status() client.error()
  if json <> "" then do
    say "Gemini JSON:"
    say json
  end
  exit 1
end

answer = client.extractText(json)
if client.status() <> 0 then do
  say "Gemini response parse failed:" client.status() client.error()
  if json <> "" then do
    say "Gemini JSON:"
    say json
  end
  exit 1
end

say answer
exit 0
