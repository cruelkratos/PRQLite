#pragma once 
#include<frontend/parser.hpp>
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
// std::unique_ptr<db::lexer::Lexer> _lexer;

};

}