#pragma once
#include<vector>
#include <string>
#include<memory>
#include<include/frontend/lexer.hpp>
#include<include/table.hpp>


namespace db::parser {
	class ASTNode{
	public:
		ASTNode(); // make paramaterized constructor.
		// virtual void parseStatement() = 0;
		ASTNode* left  = nullptr;
        ASTNode* right = nullptr;
		virtual ~ASTNode();
		
	};

	struct BinaryExpr : public ASTNode {
        std::string op;   // "=", "!=", ">", "AND", "OR", etc.
		// ~BinaryExpr();
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
		~SelectStatement();

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
		~DeleteStatement();
	};

	class CreateStatement : public ASTNode{
		public:
		CreateStatement(std::string name, std::vector<db::table::Column> &c);
		std::string tableName;
		std::shared_ptr<db::table::TableSchema> tableSchema;
		// ~CreateStatement();
 	};

}