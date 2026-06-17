#pragma once
#include<include/frontend/parser/AST.hpp>
#include<memory>
#include<optional>
#include<memory_manager.hpp>
#include<vector>
#include<cstring>
#include<table_manager.hpp>


namespace db::executor{
	class AbstractExecutor{
		protected:
		std::unique_ptr<AbstractExecutor> _child;
		db::parser::ASTNode* node;
		public:
		virtual void init() = 0;
		virtual std::optional<db::memory::Tuple> next() = 0; 
		virtual ~AbstractExecutor();
	};

	class InsertOperator : public AbstractExecutor{
		//iterate over the values in the node passed to the executor.
		//convert lexeme to actual type.
		//create required bytes
		//do std::memcpy
		//push into a buffer
		public:
		std::optional<db::memory::Tuple> next() override;
		private:
		db::memory::Tuple serializeToTuple();
		bool _hasInserted = false;
		db::memory::TableManager* table_manager;
		
		
	};


}