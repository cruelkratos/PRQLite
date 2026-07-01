#pragma once
#include "include/frontend/parser/AST.hpp"
#include "include/catalog.hpp"

/*
Walks AST via visitor pattern using polymorphism to get child type at runtime and according semantic checking logic is executed.
*/


namespace db::semantic{

	class SemanticAnalyzer: public db::parser::ASTVisitor{
		public:
		SemanticAnalyzer() = default;
		void analyze(db::parser::ASTNode* root);
		void visit(db::parser::InsertStatement& node) override;
		void visit(db::parser::SelectStatement& node) override;
		void visit(db::parser::CreateStatement& node) override;
		void visit(db::parser::DeleteStatement& node) override;
		void visit(db::parser::CreateIdxStatement& node) override;

		//expr:
		void visit(db::parser::BinaryExpr& node) override;
        void visit(db::parser::IdentifierExpr& node) override;
        void visit(db::parser::LiteralExpr& node) override;

		private:
		db::table::ColumnType resolveExpression(
        db::parser::ASTNode* node,
        std::shared_ptr<db::table::TableSchema> schema);
		std::shared_ptr<db::table::TableSchema> current_schema_ = nullptr; 
        // Stores the type of the last evaluated expression
        db::table::ColumnType last_expr_type_;
	};
}