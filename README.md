# Illama

A lightweight, Ollama-inspired application for local LLM inference using llama.cpp.

## Features

- CLI tool for model management and chat
- REST API server with OpenAI-compatible endpoints
- Model pulling from registry with checksum verification
- Streaming responses
- CMake-based build with llama.cpp integration

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### CLI

Pull a model:
```bash
./illama_cli pull llama2:7b
```

Run inference:
```bash
./illama_cli run llama2:7b "Hello, how are you?"
```

Interactive chat:
```bash
./illama_cli chat llama2:7b
```

### Server

Start server:
```bash
./illama_server serve llama2:7b
```

API endpoint: `POST /v1/chat/completions`

Example request:
```json
{
  "model": "llama2:7b",
  "messages": [
    {"role": "user", "content": "Hello!"}
  ],
  "stream": false
}
```

Example curl:
```bash
curl -X POST http://localhost:11434/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model": "llama2:7b",
    "messages": [{"role": "user", "content": "Hello!"}]
  }'
```

## Dependencies

- llama.cpp (submodule)
- cpp-httplib
- nlohmann/json
- CLI11
- Catch2