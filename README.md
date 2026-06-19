# PRQLite

> A lightweight relational database built from scratch in modern C++.

[![CMake Build](https://github.com/cruelkratos/PRQLite/actions/workflows/cmake-build.yml/badge.svg?event=push)](https://github.com/cruelkratos/PRQLite/actions/workflows/cmake-build.yml)

PRQLite is an attempt to make a fully functioning SQL Database in C++ from first principles, while undestanding various systems concepts and database internals.

The goal is not to compete with production databases, but to explore how systems such as SQLite and PostgreSQL work under the hood while building a maintainable and extensible database engine.

What does PRQL stand for? well it sounds like a `prequel` to `sequel` but could mean anything, I thought of **P**artially **R**igorous **Q**uery **L**anguage, but I am open to better names :) 

## Features

### Implemented
- SQL Lexer
- SQL Parser
- Abstract Syntax Tree (AST)
- Semantic Analysis / Binding
- Catalog Management
- In-Memory Storage Engine

### In Progress
- Query Execution Engine
- Table Scans
- Filtering and Projection
- Persistent Storage
- Query Planner

### Planned
- B+ Tree Indexes
- Transactions
- Write-Ahead Logging (WAL)
- Buffer Pool Manager
- Make DB MCP Compliant

## Use of Large Language Models
I tried to use minimal AI in making this project, i used Gemini 3.1 Pro for deep understanding of DB concepts before implementing and used Claude Sonnet 4.6 for writing tests and filling out mundane code blocks.
