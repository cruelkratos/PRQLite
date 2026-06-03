#pragma once
#include<string>
#include<vector>
#include<include/frontend/lexer.hpp>
#include<include/frontend/parser/AST.hpp>

namespace db::parser{

	

	class Parser{
		public:
		Parser();
		void insert(std::string statement);
		ASTNode * getTree();
		void reset();
		void parse_(){parseStatement();} // temp for testing

		private:
		bool isAtEnd();
		db::lexer::Token peak();
		void advance();
		bool match(const db::lexer::TokenType t);
		bool expect(const db::lexer::Token &t);
		std::vector<db::lexer::Token> tokenStream;
		void parseStatement(); //sets root node of statement
		SelectStatement* parseSelect();
		InsertStatement* parseInsert();
		DeleteStatement* parseDelete();
		CreateStatement* parseCreate();
		std::vector<db::table::Column> parseColumnDefs();
		db::table::Column parseColumnDef();
		ASTNode* parseExpression();
		ASTNode* parseOrExpr();
		ASTNode* parseAndExpr();
		ASTNode* parseEqualityExpr();
		ASTNode* parseComparisonExpr();
		ASTNode* parsePrimary();
		std::string parseIdentifier();
		std::vector<db::lexer::Token> parseValues();
		int token_pos;
		ASTNode* statementTree;
	};

	
}