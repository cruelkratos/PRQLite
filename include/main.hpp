#pragma once 
#include<include/frontend/parser/parser.hpp>
#include<include/frontend/semantic_analyzer.hpp>
#include<frontend/lexer.hpp>
#include<memory>

namespace db {

class DB{
public:
	
	DB();
	int REPL();
	~DB();

private:
std::unique_ptr<db::parser::Parser> _parser;
std::unique_ptr<db::semantic::SemanticAnalyzer> _semantic_analyzer;
// std::unique_ptr<db::lexer::Lexer> _lexer;

};

}