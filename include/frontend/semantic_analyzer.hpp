#pragma once
#include<include/frontend/parser/AST.hpp>
#include<include/catalog.hpp>

namespace db::semantic{

	class ASTVisitor{
		public:
		virtual ~ASTVisitor() = default;
		virtual void visit(db::parser::BinaryExpr& node){}
		virtual void visit(db::parser::LiteralExpr& node) {}
		virtual void visit(db::parser::IdentifierExpr& node) {}
		virtual void visit(db::parser::SelectStatement& node) =0;
		virtual void visit(db::parser::InsertStatement& node) =0;
		virtual void visit(db::parser::DeleteStatement& node) {}
		virtual void visit(db::parser::CreateStatement& node) =0;
	};

	class SemanticAnalyzer: public ASTVisitor{
		public:
		SemanticAnalyzer() = default;
		void analyze(db::parser::ASTNode* root);
		void visit(db::parser::InsertStatement& node);
		void visit(db::parser::SelectStatement& node);
		void visit(db::parser::CreateStatement& node);

		private:
		db::table::ColumnType resolveExpression(
        db::parser::ASTNode* node,
        std::shared_ptr<db::table::TableSchema> schema);
	};
}