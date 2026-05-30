#pragma once
#include<vector>
#include <string>
#include<include/frontend/lexer.hpp>


namespace db::parser {
	class ASTNode{
	public:
		std::vector<ASTNode*> children;
		ASTNode(); // make paramaterized constructor.
		// virtual void parseStatement() = 0;
		virtual ~ASTNode() = default;
		
	};

	struct BinaryExpr : public ASTNode {
        std::string op;   // "=", "!=", ">", "AND", "OR", etc.
        ASTNode* left  = nullptr;
        ASTNode* right = nullptr;
    };

    struct LiteralExpr : public ASTNode {
        db::lexer::Token value; // NUMBER, STRING, TRUE, FALSE
    };

    struct IdentifierExpr : public ASTNode {
        std::string name;
    };

	class Statement : public ASTNode{};

	class SelectStatement : public ASTNode{
	public:
		bool selectStar = false;
        std::vector<std::string> columns;
        std::string tableName;
        ASTNode* whereClause  = nullptr;
        std::string orderBy;                    
        std::string orderDir  = "ASC";
        int limitVal          = -1; 

	};

	class InsertStatement : public ASTNode{
	public:
		std::string tableName;
        std::vector<db::lexer::Token> values;
	};

	class DeleteStatement : public ASTNode{
	public:
		std::string tableName;
        ASTNode* whereClause = nullptr;
	};

}