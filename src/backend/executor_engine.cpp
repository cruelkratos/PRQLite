#include<include/backend/executor.hpp>

namespace db::executor{
	void ExecutorEngine::visit(db::parser::InsertStatement& node){
		auto table_manager = db::semantic::Catalog::getInstance().getTableManager(node.tableId);

		InsertOperator insert_plan (&node,table_manager);
		insert_plan.next();

		while (insert_plan.next().has_value()) {
        }

	}

};