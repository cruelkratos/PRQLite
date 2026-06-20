#pragma once
#include<include/frontend/parser/AST.hpp>
#include<memory>
#include<optional>
#include <include/backend/memory_manager.hpp>
#include<vector>
#include<iostream>
#include<cstring>
#include<include/backend/table_manager.hpp>
#include<include/frontend/semantic_analyzer.hpp>


/*
Executes parsed SQL statements through a pipeline of chained operators. Each operator pulls tuples from its child operator and applies its Operator. ExecutorEngine drives the whole flow using the visitor pattern on the AST, 
dispatching to the right operator based on statement type.
*/


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


	class SelectOperator: public AbstractExecutor{
		//Simply scan the row and return it back no checks to be done.
		//give tuple above other operators can check validity/project

		public:
		SelectOperator(db::parser::ASTNode* select_node, std::shared_ptr<db::memory::TableManager> tm){
			node = select_node;
			table_manager = tm;
		}

		std::optional<db::memory::Tuple> next() override;
		void init() override;

		private:
		std::optional<db::memory::TableIterator> it;
		std::shared_ptr<db::memory::TableManager> table_manager;
	};

	class InsertOperator : public AbstractExecutor{
		//iterate over the values in the node passed to the executor.
		//convert lexeme to actual type.
		//create required bytes
		//do std::memcpy
		//push into a buffer
		public:
		InsertOperator(db::parser::ASTNode* insert_node, std::shared_ptr<db::memory::TableManager> tm) {
			node = insert_node;
			table_manager = tm;
		}
		std::optional<db::memory::Tuple> next() override;
		void init() override {}

		private:
		db::memory::Tuple serializeToTuple();
		bool _hasInserted = false;
		std::shared_ptr<db::memory::TableManager> table_manager;
		
		
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
		void visit(db::parser::SelectStatement& node) override;

	};


}