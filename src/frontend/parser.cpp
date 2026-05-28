#include<include/frontend/parser.hpp>
#include<include/frontend/lexer.hpp>
#include<memory>
#include<string>

namespace db::parser{
	void Parser::insert(std::string statement){
		std::unique_ptr<db::lexer::Lexer> lexer = std::make_unique<db::lexer::Lexer>(statement);
		return ;
	}
}