#pragma once
#include<include/frontend/parser/AST.hpp>
#include<include/catalog.hpp>

namespace db::semantic{

	class ASTVisitor{
		public:
		virtual ~ASTVisitor() = default;
		virtual void visit(db::parser::BinaryExpr& node) = 0;
		virtual void visit(db::parser::LiteralExpr& node) =0;
		virtual void visit(db::parser::IdentifierExpr& node) =0;
		virtual void visit(db::parser::SelectStatement& node) =0;
		virtual void visit(db::parser::InsertStatement& node) =0;
		virtual void visit(db::parser::DeleteStatement& node) =0;
		virtual void visit(db::parser::CreateStatement& node) =0;
	};

	class SemanticAnalyzer: public ASTVisitor{
		public:
		void analyze(db::parser::ASTNode* root);
		void visit(db::parser::InsertStatement& node);
	};
}