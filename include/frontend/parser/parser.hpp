#pragma once
#include<string>
#include<vector>
#include<frontend/lexer.hpp>
#include<frontend/parser/AST.hpp>

namespace db::parser{

	

	class Parser{
		public:
		Parser();
		void insert(std::string statement);
		ASTNode * getTree();
		void reset();

		private:
		db::lexer::Token peak();
		void advance();
		bool match(db::lexer::Token &t);
		bool expect(db::lexer::Token &t);
		std::vector<db::lexer::Token> tokenStream;
		void parseStatement(); //sets root node of statement
		int token_pos;
		ASTNode* statementTree;
	};

	
}