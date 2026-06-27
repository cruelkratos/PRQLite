#pragma once 
#include "frontend/lexer.hpp"
#include "include/frontend/semantic_analyzer.hpp"
#include "include/frontend/parser/parser.hpp"
#include "include/virtual_machine/executor.hpp"
#include "include/backend/buffer_pool.hpp"
#include <memory>
#include <csignal>

namespace db {

class DB{
public:
	
	DB();
	int connect();
	~DB();

private:
	std::unique_ptr<db::parser::Parser> _parser;
	std::unique_ptr<db::semantic::SemanticAnalyzer> _semantic_analyzer;
	std::unique_ptr<db::executor::ExecutorEngine> _executor;
	inline static volatile sig_atomic_t interrupt{0};

	static void handleSignal(int);
	int REPL();

};

}