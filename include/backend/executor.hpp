#pragma once
#include<include/frontend/parser/AST.hpp>
#include<memory>
#include<optional>
#include <include/backend/memory_manager.hpp>
#include<vector>
#include<cstring>
#include<include/backend/table_manager.hpp>
#include<include/frontend/semantic_analyzer.hpp>


namespace db::executor{
	class AbstractExecutor{
		protected:
		std::unique_ptr<AbstractExecutor> _child;
		db::parser::ASTNode* node;
		public:
		virtual void init() = 0;
		virtual std::optional<db::memory::Tuple> next() = 0; 
		virtual ~AbstractExecutor() = default;
	};

	class InsertOperator : public AbstractExecutor{
		//iterate over the values in the node passed to the executor.
		//convert lexeme to actual type.
		//create required bytes
		//do std::memcpy
		//push into a buffer
		public:
		InsertOperator(db::parser::ASTNode* insert_node, db::memory::TableManager* tm) {
			node = insert_node;
			table_manager = tm;
		}
		std::optional<db::memory::Tuple> next() override;
		void init() override {}

		private:
		db::memory::Tuple serializeToTuple();
		bool _hasInserted = false;
		db::memory::TableManager* table_manager;
		
		
	};

	class ExecutorEngine :  public db::parser::ASTVisitor{
		public:
		void execute(db::parser::ASTNode* root){
			if(root){
				root->accept(*this);
			}
		}
		// void visit(db::parser::InsertStatement* node) override;
		void visit(db::parser::InsertStatement& node) override;
		void visit(db::parser::CreateStatement& node) override {}
        void visit(db::parser::DeleteStatement& node) override {}

		void visit(db::parser::BinaryExpr& node) override {}
        void visit(db::parser::LiteralExpr& node) override {}
        void visit(db::parser::IdentifierExpr& node) override {}
		void visit(db::parser::SelectStatement& node) override {}

	};


}