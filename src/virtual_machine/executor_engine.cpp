#include "include/virtual_machine/executor.hpp"
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

		FilterOperator filter (&node,&seq_scan);
		filter.setWhereClause(node.whereClause);
		if(node.selectStar){
			filter.init();
			//add where clause logic here!
			for (const auto& col : schema->columns) {
				std::cout << std::left << std::setw(15) << col.colName; 
			}
			std::cout << "\n-------------------------------------------------\n";
			int rows = 0;
			while(auto o_tup = filter.next()){
				db::utils::printTuple(*o_tup,*schema);
				std::cout << "\n";
				++rows;
			}
			std::cout<<"("<<rows<<" rows)\n";
			return;
		}

		//add where clause logic here!!
		ProjectionOperator projector(&node, &filter);
		projector.init();

		int rows = 0;
		auto proj_schema = projector.getOutputSchema();
		for (const auto& col : proj_schema.columns) {
			std::cout << std::left << std::setw(15) << col.colName; 
		}
		std::cout << "\n-------------------------------------------------\n";
		while(auto o_tup = projector.next()){
				db::utils::printTuple(*o_tup,projector.getOutputSchema());
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

	void ExecutorEngine::visit(db::parser::DeleteStatement& node){
		auto table_manager = db::semantic::Catalog::getInstance().getTableManager(node.tableId);

		SelectOperator seq_scan (&node, table_manager, this->interrupt);
		seq_scan.init();

		FilterOperator filter (&node,&seq_scan);
		filter.setWhereClause(node.whereClause);

		DeleteOperator deleter (&node,&filter,table_manager);
		deleter.init();
		int rows = 0;
		while(auto d_tup = deleter.next()){
			++rows;
		}
		std::cout<<"("<<rows<<" rows) Affected\n";
	}


	void ExecutorEngine::visit(db::parser::CreateIdxStatement& node){
		
	}


};