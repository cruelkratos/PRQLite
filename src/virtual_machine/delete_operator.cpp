#include "include/virtual_machine/executor.hpp"


namespace db::executor{

	DeleteOperator::DeleteOperator(db::parser::ASTNode* delete_node, AbstractExecutor* child,std::shared_ptr<db::memory::TableManager> tm){
		this->node = delete_node;
		this->_child = child;
		this->table_manager = tm;
	}
	void DeleteOperator::init(){
		_child->init();
	}
	std::optional<db::memory::Tuple> DeleteOperator::next(){
		auto tuple = _child->next();
		if(!tuple) return std::nullopt;
		try{
			this->table_manager->deleteTuple(tuple->rid);
			db::transaction::TransactionManager::current().recordDelete(table_manager, *tuple);
		} catch(...){
			throw std::runtime_error("EXECUTOR ERROR: Delete Operation Failed.");
		}

		return tuple;

	}

}
