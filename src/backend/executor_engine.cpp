#include "include/backend/executor.hpp"
#include <iomanip>

namespace db::executor{
	void ExecutorEngine::visit(db::parser::InsertStatement& node){
		auto table_manager = db::semantic::Catalog::getInstance().getTableManager(node.tableId);

		InsertOperator insert_plan (&node,table_manager);
		insert_plan.next();


	}

	void ExecutorEngine::visit(db::parser::SelectStatement& node){
		//only writing for seq scan right now.
		//select statement with only *
		auto table_manager = db::semantic::Catalog::getInstance().getTableManager(node.tableId);
		auto& catalog = db::semantic::Catalog::getInstance();
		auto schema = catalog.getTableSchema(node.tableId);

		SelectOperator seq_scan (&node, table_manager, this->interrupt);
		seq_scan.init();
		for (const auto& col : schema->columns) {
            // Print column names with a fixed width of 15 characters
            std::cout << std::left << std::setw(15) << col.colName; 
        }
        std::cout << "\n-------------------------------------------------\n";
		int rows = 0;
		while(auto o_tup = seq_scan.next()){
			o_tup.value().print(std::cout, *schema);
			std::cout << "\n";
			++rows;
		}
		std::cout<<"("<<rows<<" rows)\n";
	}
	
	void ExecutorEngine::visit(db::parser::CreateStatement& node){
		node.tableSchema = db::semantic::Catalog::getInstance().createTable(node.tableName, node.tableSchema->columns);
		node.tableId = node.tableSchema->tableId;

		db::semantic::Catalog::getInstance().flush();
	}

};