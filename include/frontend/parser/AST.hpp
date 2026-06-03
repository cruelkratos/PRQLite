#pragma once
#include<vector>
#include <string>
#include<memory>
#include<include/frontend/lexer.hpp>
#include<include/table.hpp>
#include<cstdint>


namespace db::semantic { class ASTVisitor; }

using table_oid_t = std::uint32_t;

namespace db::parser {
	
	class ASTNode{
	public:
		ASTNode(); // make paramaterized constructor.
		// virtual void parseStatement() = 0;
		ASTNode* left  = nullptr;
        ASTNode* right = nullptr;
		table_oid_t tableId;
		virtual void accept(db::semantic::ASTVisitor& visitor) = 0;
		virtual ~ASTNode();
		
	};

	struct BinaryExpr : public ASTNode {
        std::string op;   // "=", "!=", ">", "AND", "OR", etc.
		// ~BinaryExpr();
		void accept(db::semantic::ASTVisitor& visitor) override;
    };

    struct LiteralExpr : public ASTNode {
        db::lexer::Token value; // NUMBER, STRING, TRUE, FALSE
		void accept(db::semantic::ASTVisitor& visitor) override;
    };

    struct IdentifierExpr : public ASTNode {
        std::string name;
		db::table::Column resolvedColumn;
		bool isResolved = false;
		void accept(db::semantic::ASTVisitor& visitor) override;
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
		std::vector<db::table::Column> resolvedColumns; // actual Column objects from schema
    	int orderByColId = -1;                          
		void accept(db::semantic::ASTVisitor& visitor) override;
		~SelectStatement();

	};

	class InsertStatement : public ASTNode{
	public:
		std::string tableName;
        std::vector<db::lexer::Token> values;
		void accept(db::semantic::ASTVisitor& visitor) override;
	};

	class DeleteStatement : public ASTNode{
	public:
		std::string tableName;
        ASTNode* whereClause = nullptr;
		void accept(db::semantic::ASTVisitor& visitor) override;
		~DeleteStatement();
	};

	class CreateStatement : public ASTNode{
		public:
		CreateStatement(std::string name, std::vector<db::table::Column> &c);
		std::string tableName;
		std::shared_ptr<db::table::TableSchema> tableSchema;
		void accept(db::semantic::ASTVisitor& visitor) override;
		// ~CreateStatement();
 	};

}