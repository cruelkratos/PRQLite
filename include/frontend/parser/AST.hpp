#pragma once
#include<vector>
#include <string>
#include<memory>
#include<include/frontend/lexer.hpp>
#include<include/table.hpp>
#include<cstdint>


namespace db::parser { class ASTVisitor; }

using table_oid_t = std::uint32_t;

namespace db::parser {


	class BinaryExpr;
    class LiteralExpr;
    class IdentifierExpr;
    class SelectStatement;
    class InsertStatement;
    class DeleteStatement;
    class CreateStatement;
	class ASTVisitor{
		public:
		virtual ~ASTVisitor() = default;
		virtual void visit(db::parser::BinaryExpr& node) =0;
		virtual void visit(db::parser::LiteralExpr& node) =0;
		virtual void visit(db::parser::IdentifierExpr& node) =0;
		virtual void visit(db::parser::SelectStatement& node) =0;
		virtual void visit(db::parser::InsertStatement& node) =0;
		virtual void visit(db::parser::DeleteStatement& node) =0;
		virtual void visit(db::parser::CreateStatement& node) =0;

	};
	
	class ASTNode{
	public:
		ASTNode(); // make paramaterized constructor.
		// virtual void parseStatement() = 0;
		ASTNode* left  = nullptr;
        ASTNode* right = nullptr;
		table_oid_t tableId;
		virtual void accept(db::parser::ASTVisitor& visitor) = 0;
		virtual ~ASTNode();
		
	};

	struct BinaryExpr : public ASTNode {
        std::string op;   // "=", "!=", ">", "AND", "OR", etc.
		// ~BinaryExpr();
		void accept(db::parser::ASTVisitor& visitor) override;
    };

    struct LiteralExpr : public ASTNode {
        db::lexer::Token value; // NUMBER, STRING, TRUE, FALSE
		void accept(db::parser::ASTVisitor& visitor) override;
    };

    struct IdentifierExpr : public ASTNode {
        std::string name;
		db::table::Column resolvedColumn;
		bool isResolved = false;
		void accept(db::parser::ASTVisitor& visitor) override;
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
		void accept(db::parser::ASTVisitor& visitor) override;
		~SelectStatement();

	};

	class InsertStatement : public ASTNode{
	public:
		std::string tableName;
        std::vector<db::lexer::Token> values;
		void accept(db::parser::ASTVisitor& visitor) override;
	};

	class DeleteStatement : public ASTNode{
	public:
		std::string tableName;
        ASTNode* whereClause = nullptr;
		void accept(db::parser::ASTVisitor& visitor) override;
		~DeleteStatement();
	};

	class CreateStatement : public ASTNode{
		public:
		CreateStatement(std::string name, std::vector<db::table::Column> &c);
		std::string tableName;
		std::shared_ptr<db::table::TableSchema> tableSchema;
		void accept(db::parser::ASTVisitor& visitor) override;
		// ~CreateStatement();
 	};

}