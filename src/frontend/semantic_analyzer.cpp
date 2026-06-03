#include<include/frontend/semantic_analyzer.hpp>
#include<table.hpp>
#include<stdexcept>

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

			db::table::ColumnType literalType = db::table::tokenToColumnType(statement_col[i].type);
			if(t_col[i].type != literalType){
				throw std::runtime_error("SEMANTIC ERROR:" + std::to_string(i) + "th item's type doesn't match");
			}
		}
	}

	void SemanticAnalyzer::visit(db::parser::SelectStatement& node){
		auto t_id = db::semantic::Catalog::getInstance().getTableId(node.tableName);
		auto t_schema = db::semantic::Catalog::getInstance().getTableSchema(t_id);
		node.tableId = t_id;
		const auto& t_cols = t_schema->columns;

		if(!node.selectStar){
			for (const auto& colName : node.columns) {
			bool found = false;
			for (const auto& col : t_cols) {
				if (col.colName == colName) { node.resolvedColumns.push_back(col); found = true; break; }
			}
			if (!found)
				throw std::runtime_error("SEMANTIC ERROR: column '" + colName + "' does not exist in table '" + node.tableName + "'");
       		}
		}

		else{
			node.resolvedColumns = t_cols;
		}

		if (!node.orderBy.empty()) {
        bool found = false;
        for (const auto& col : t_cols) {
            if (col.colName == node.orderBy) { node.orderByColId = col.colId;found = true; break; }
        }
        if (!found)
            throw std::runtime_error("SEMANTIC ERROR: ORDER BY column '" + node.orderBy + "' does not exist in table '" + node.tableName + "'");
    	}

  
		if (node.whereClause != nullptr) {
			resolveExpression(node.whereClause, t_schema);
		}

	}

	db::table::ColumnType SemanticAnalyzer::resolveExpression(
		db::parser::ASTNode* node, 
		std::shared_ptr<db::table::TableSchema> schema)
	{
		if (auto* bin = dynamic_cast<db::parser::BinaryExpr*>(node)) {
			auto leftType  = resolveExpression(bin->left,  schema);
			auto rightType = resolveExpression(bin->right, schema);

			// AND / OR — both sides must be BOOL
			if (bin->op == "AND" || bin->op == "OR") {
				if (leftType != db::table::ColumnType::BOOL || 
					rightType != db::table::ColumnType::BOOL)
					throw std::runtime_error("SEMANTIC ERROR: AND/OR requires boolean operands");
				return db::table::ColumnType::BOOL;
			}

			// comparison operators — both sides must be same type, result is BOOL
			if (bin->op == "=" || bin->op == "!=" ||
				bin->op == ">" || bin->op == ">=" ||
				bin->op == "<" || bin->op == "<=") {
				if (leftType != rightType)
					throw std::runtime_error(
						"SEMANTIC ERROR: type mismatch in expression — left and right of '" 
						+ bin->op + "' must be same type");
				return db::table::ColumnType::BOOL;
			}

			throw std::runtime_error("SEMANTIC ERROR: unknown operator '" + bin->op + "'");
		}

		if (auto* ident = dynamic_cast<db::parser::IdentifierExpr*>(node)) {
			// look up the column in the schema
			for (const auto& col : schema->columns) {
				if (col.colName == ident->name){
					ident->resolvedColumn = col;  // bind
            		ident->isResolved     = true;
					return col.type;
				}
			}
			throw std::runtime_error("SEMANTIC ERROR: column '" + ident->name + "' does not exist in table '" + schema->tableName + "'");
		}

		if (auto* lit = dynamic_cast<db::parser::LiteralExpr*>(node)) {
			return db::table::tokenToColumnType(lit->value.type);
		}

		throw std::runtime_error("SEMANTIC ERROR: unknown expression node type");
	}

	void SemanticAnalyzer::visit(db::parser::CreateStatement& node) { 
		node.tableSchema = Catalog::getInstance().createTable(node.tableName, node.tableSchema->columns);
	}

	// void SemanticAnalyzer::visit(db::parser::CreateStatement& node){}
}