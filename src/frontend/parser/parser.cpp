#include<include/frontend/parser/parser.hpp>
#include<include/frontend/parser/AST.hpp>
#include<include/frontend/lexer.hpp>
#include<memory>
#include<iostream>
#include<stdexcept>
#include<string>

namespace db::parser{

	Parser::Parser() : token_pos(0), statementTree(nullptr){}

	ASTNode::ASTNode() {}

	void Parser::insert(std::string statement){
		try{
			std::unique_ptr<db::lexer::Lexer> _lexer = std::make_unique<db::lexer::Lexer>(statement);
			db::lexer::Token temp = _lexer.get()->next_token();

			while(temp.type!= db::lexer::TokenType::EOF_TOKEN){
				tokenStream.push_back(temp);
			}
			
		}
		catch (const std::runtime_error& e) {
        std::cerr << "Runtime error in creating LEXER: " << e.what() << std::endl;
		throw;
    	}

		catch (...) {
        std::cerr << "Unknown exception occurred! when creating LEXER" << std::endl;
		throw;
    	}


		return ;
	}

	ASTNode* Parser::getTree(){
		return statementTree;
	}

	void Parser::reset(){
		statementTree = nullptr;
		token_pos = 0;
	}
	
	db::lexer::Token Parser::peak() {
		if(tokenStream.size()<1){
			throw std::runtime_error("Token Stream is Empty");
		}
		if(token_pos >= tokenStream.size()){
			throw std::exception("At End of token stream");
		}
		return tokenStream[token_pos];
	}

	void Parser::advance(){
		++token_pos;
	}

	bool Parser::match(db::lexer::Token &t){
		if (this->peak().lexeme == t.lexeme && this->peak().type == t.type) {
			this->advance();
			return true;
		} 
		return false;
	}
	bool Parser::expect(db::lexer::Token &t){
		if (this->peak().lexeme == t.lexeme && this->peak().type == t.type) {
			this->advance();
			return true;
		} 
		throw std::runtime_error("SYNTAX ERROR: expected " + t.lexeme);
	}

	void Parser::parseStatement(){
		// implement according to grammar.
		statementTree = new Statement(); // simple for now
	}
}