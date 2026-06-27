#pragma once
#include "include/virtual_machine/memory_manager.hpp"
#include "include/frontend/parser/AST.hpp"
#include "include/virtual_machine/table_manager.hpp"
#include "include/frontend/semantic_analyzer.hpp"
#include "include/catalog.hpp"
#include "include/utils.hpp"
#include <memory>
#include <optional>
#include <vector>
#include <iostream>
#include <cstring>
#include <csignal>
#include <variant>
#include <unordered_map>



/*
Executes parsed SQL statements through a pipeline of chained operators. Each operator pulls tuples from its child operator and applies its Operator. ExecutorEngine drives the whole flow using the visitor pattern on the AST, 
dispatching to the right operator based on statement type.
*/


namespace db::executor{

	using SQLValue = std::variant<int, bool, std::string>;
	class AbstractExecutor{
		protected:
		AbstractExecutor* _child;
		db::parser::ASTNode* node;
		public:
		virtual void init() = 0;
		virtual std::optional<db::memory::Tuple> next() = 0; 
		virtual ~AbstractExecutor() = default;
		virtual db::table::TableSchema getOutputSchema() const = 0;
	};


	class SelectOperator: public AbstractExecutor{
		//Simply scan the row and return it back no checks to be done.
		//give tuple above other operators can check validity/project

		public:
		SelectOperator(db::parser::ASTNode* select_node, std::shared_ptr<db::memory::TableManager> tm, volatile std::sig_atomic_t* i){
			node = select_node;
			table_manager = tm;
			interrupt = i;
		}

		std::optional<db::memory::Tuple> next() override;
		void init() override;
		db::table::TableSchema getOutputSchema() const override;

		private:
		volatile std::sig_atomic_t* interrupt;
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
		db::table::TableSchema getOutputSchema() const override{}

		private:
		db::memory::Tuple serializeToTuple();
		bool _hasInserted = false;
		std::shared_ptr<db::memory::TableManager> table_manager;
		
		
	};


	class ProjectionOperator : public AbstractExecutor{
		public:
		ProjectionOperator(db::parser::ASTNode* select_node, AbstractExecutor* child);
		std::optional<db::memory::Tuple> next() override;
		void init() override;
		db::table::TableSchema getOutputSchema() const override { return projected_schema; }

		private:
		std::vector<db::table::Column> colList;
		std::vector<std::pair<uint32_t, uint32_t>> projection_offsets;
		db::table::TableSchema projected_schema{110, "projection", {}};
	};

	class FilterOperator : public AbstractExecutor{
		public:
		FilterOperator(db::parser::ASTNode* select_node, AbstractExecutor* child);
		std::optional<db::memory::Tuple> next() override;
		void init() override;
		db::table::TableSchema getOutputSchema() const override { return child_schema; }

		private:
		db::parser::ASTNode* whereClause;
		bool hasClause{false};
		db::table::TableSchema child_schema{0, "", {}};

		std::unordered_map<std::string, uint32_t> column_offsets;
        std::unordered_map<std::string, db::table::ColumnType> column_types;

		SQLValue evaluateExpr(db::parser::ASTNode* expr, const db::memory::Tuple& tuple);
	};

	class ExecutorEngine :  public db::parser::ASTVisitor{
		private:
		volatile std::sig_atomic_t* interrupt;
		public:
		ExecutorEngine(volatile std::sig_atomic_t* i) : interrupt(i){}
		void execute(db::parser::ASTNode* root){
			if(root){
				root->accept(*this);
			}
		}
		// void visit(db::parser::InsertStatement* node) override;
		void visit(db::parser::InsertStatement& node) override;
		void visit(db::parser::CreateStatement& node) override;
        void visit(db::parser::DeleteStatement& node) override {}

		void visit(db::parser::BinaryExpr& node) override {}
        void visit(db::parser::LiteralExpr& node) override {}
        void visit(db::parser::IdentifierExpr& node) override {}
		void visit(db::parser::SelectStatement& node) override;

	};



}