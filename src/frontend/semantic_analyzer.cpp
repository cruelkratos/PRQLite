#include<include/frontend/semantic_analyzer.hpp>

namespace db::semantic{

	void SemanticAnalyzer::analyze(db::parser::ASTNode* root){
		root->accept(*this);
	}

	void SemanticAnalyzer::visit(db::parser::InsertStatement& node) {
		//get table schema
		auto t_id = db::semantic::Catalog::getInstance().getTableId(node.tableName);
		auto t_schema = db::semantic::Catalog::getInstance().getTableSchema(t_id); //shared_ptr

		node.tableId = t_id;

		const auto& t_col = t_schema->columns;
		const auto& statement_col = node.values;
		if (t_col.size() != statement_col.size()){
			throw std::runtime_error("SEMANTIC ERROR: shape of table doesn't match query");
		}
		for(size_t i = 0;i<t_col.size();i++){
			if(t_col[i].type != statement_col[i].type){
				throw std::runtime_error("SEMANTIC ERROR:" + std::to_string(i) + "th item's type doesn't match");
			}
		}
	}
}