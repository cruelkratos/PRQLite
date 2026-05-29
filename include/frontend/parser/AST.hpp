#pragma once
#include<vector>


namespace db::parser {
	class ASTNode{
	public:
		std::vector<ASTNode*> children;
		ASTNode(); // make paramaterized constructor.
		virtual void parseStatement() = 0;
		
	};

	class Statement : public ASTNode{
	public:
		void parseStatement() override {}
	};

	class SelectStatement : public ASTNode{
	public:
		void parseStatement() override {}
	};

	class InsertStatement : public ASTNode{
	public:
		void parseStatement() override {}
	};

	class DeleteStatement : public ASTNode{
	public:
		void parseStatement() override {}
	};
	// add more this is for now
}