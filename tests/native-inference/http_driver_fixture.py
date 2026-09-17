#!/usr/bin/env python3
"""Local HTTP conformance fixture. Receives an already-built VM command; no keys/network."""
import http.server
import json
import subprocess
import sys
import threading

failures = []
class Handler(http.server.BaseHTTPRequestHandler):
    protocol_version = 'HTTP/1.1'
    def log_message(self, *_):
        pass
    def do_POST(self):
        try:
            body = json.loads(self.rfile.read(int(self.headers['Content-Length'])))
            if self.path == '/api/generate':
                prompt = body['prompt']; provider = 'ollama'
            elif self.path == '/v1/responses':
                prompt = body['input']; provider = 'openai'
                assert self.headers['Authorization'] == 'Bearer fixture-key'
            elif self.path == '/v1/messages':
                prompt = body['messages'][0]['content']; provider = 'anthropic'
                assert self.headers['x-api-key'] == 'fixture-key'
            elif self.path == '/v1beta/models/test-model:generateContent':
                prompt = body['contents'][0]['parts'][0]['text']; provider = 'gemini'
                assert self.headers['x-goog-api-key'] == 'fixture-key'
            else:
                raise AssertionError('unexpected path ' + self.path)
            if provider != 'gemini':
                assert body['model'] == 'test-model'
            code = 503 if prompt == 'http-error' else 200
            text = 'Hello 🌍'
            payload = {
                'ollama': {'response': text, 'done': True},
                'openai': {'output': [{'content': [{'type': 'output_text', 'text': text}]}]},
                'anthropic': {'content': [{'type': 'text', 'text': text}]},
                'gemini': {'candidates': [{'content': {'parts': [{'text': text}]}}]},
            }[provider]
            if prompt == 'http-error': payload = {'error': {'message': 'fixture unavailable'}}
            data = b'not json' if prompt == 'bad-json' else json.dumps(payload, ensure_ascii=False).encode()
        except Exception as error:
            failures.append(str(error)); code = 500; data = b'fixture contract failed'
        self.send_response(code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', str(len(data)))
        self.send_header('Connection', 'close')
        self.end_headers()
        self.wfile.write(data)
        self.close_connection = True

with http.server.ThreadingHTTPServer(('127.0.0.1', 0), Handler) as server:
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        run = subprocess.run(sys.argv[1:] + ['-a', str(server.server_port)], timeout=300)
    finally:
        server.shutdown(); thread.join()
if failures:
    print('FAIL: HTTP fixture:', failures)
sys.exit(run.returncode or bool(failures))
