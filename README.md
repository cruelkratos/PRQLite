# PRQLite

> A lightweight relational database built from scratch in modern C++.

[![CMake Build](https://github.com/cruelkratos/PRQLite/actions/workflows/cmake-build.yml/badge.svg?event=push)](https://github.com/cruelkratos/PRQLite/actions/workflows/cmake-build.yml)
[![Docker Image Build and Run](https://github.com/cruelkratos/PRQLite/actions/workflows/docker-image.yml/badge.svg)](https://github.com/cruelkratos/PRQLite/actions/workflows/docker-image.yml)

PRQLite is an attempt to make a fully functioning SQL Database in C++ from first principles, while undestanding various systems concepts and database internals.

The goal is not to compete with production databases, but to explore how systems such as SQLite and PostgreSQL work under the hood while building a maintainable and extensible database engine.

What does PRQL stand for? well it sounds like a `prequel` to `sequel` but could mean anything, I thought of **P**artially **R**igorous **Q**uery **L**anguage, but I am open to better names :) 

Another name I liked particulary was [Larp DB](https://media.tenor.com/KoOyzRzGUU0AAAAd/god-of-larp.gif).

---

## Architecture

PRQLite implements a full database pipeline from SQL text to disk:

```rust
SQL → Lexer → Parser → AST → Semantic Analysis → Executor → Storage
```

Each layer is written from scratch with no external database dependencies.

---

## Status

**Complete**
- SQL lexer, recursive-descent parser, and AST
- Semantic analysis and type binding
- Catalog management
- Query execution engine (SELECT, INSERT, DELETE, CREATE TABLE)
- Page-based persistent storage with buffer pool manager

**In Progress**
- Filtering and projection
- Query planner
- Transactions and write-ahead logging (WAL)
- Test and Benchmark using GTest and Google Benchmark.

**Planned**
- B+ Tree indexes
- Wire protocol
- MCP compliance

---



## Running with Docker

You can run PRQLite directly as a Docker container without installing any dependencies.

### 1. Create a persistent volume

This stores your database files outside the container so they persist across restarts.

```console
docker volume create prqlite-data
```

### 2. Run PRQLite

```console
docker run -it \
    -v prqlite-data:/data \
    prqlite:latest
```

The database will automatically create its storage files inside `/data` on first startup. Any data written to the database will remain available even if the container is removed, as long as the `prqlite-data` volume is retained.


## Use of Large Language Models
AI was used for concept walkthroughs and boilerplate; all core systems designed and implemented by hand, used Claude Sonnet and Gemini 3.1 Pro (also used for mundane functions and writing test files).
