#include "include/frontend/semantic_analyzer.hpp"
#include "table.hpp"
#include <stdexcept>
#include <string>
#include <unordered_set>

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

		if(node.tableSchema->columns.size()==0){
			throw std::runtime_error("SEMANTIC ERROR: can't have empty columns");
		}

		std::unordered_set<std::string> seen;

		for(const auto &col: node.tableSchema->columns){
			if(seen.find(col.colName)!=seen.end()){
				throw std::runtime_error("SEMANTIC ERROR: duplicate column name '" + col.colName + 
                "' in table '" + node.tableName + "'");
			}
			seen.insert(col.colName);
		}
	}

	void SemanticAnalyzer::visit(db::parser::DeleteStatement& node){
		auto t_id = Catalog::getInstance().getTableId(node.tableName);
        auto t_schema = Catalog::getInstance().getTableSchema(t_id);

		node.tableId = t_id;

		if(node.whereClause != nullptr){
			this->current_schema_ = t_schema;

			node.whereClause->accept(*this);

			if (this->last_expr_type_ != db::table::ColumnType::BOOL) {
                throw std::runtime_error("SEMANTIC ERROR: WHERE clause must evaluate to a boolean condition");
            }
		}
		this->current_schema_ = nullptr;
	}

	// EXPR:
	void SemanticAnalyzer::visit(db::parser::LiteralExpr& node) {
        // Set the state variable for the parent to read
        this->last_expr_type_ = db::table::tokenToColumnType(node.value.type);
    }

	void SemanticAnalyzer::visit(db::parser::IdentifierExpr& node) {
        if (!current_schema_) {
            throw std::runtime_error("INTERNAL ERROR: No schema set for identifier evaluation");
        }

        for (const auto& col : current_schema_->columns) {
            if (col.colName == node.name){
                node.resolvedColumn = col;  // bind
                node.isResolved     = true;
                this->last_expr_type_ = col.type; // Pass type up to parent
                return;
            }
        }
        throw std::runtime_error("SEMANTIC ERROR: column '" + node.name + 
                                 "' does not exist in table '" + current_schema_->tableName + "'");
    }


	void SemanticAnalyzer::visit(db::parser::BinaryExpr& node) {
        // 1. Visit left child and grab its type
        node.left->accept(*this);
        auto leftType = this->last_expr_type_;

        // 2. Visit right child and grab its type
        node.right->accept(*this);
        auto rightType = this->last_expr_type_;

        // 3. AND / OR logic
        if (node.op == "AND" || node.op == "OR") {
            if (leftType != db::table::ColumnType::BOOL || rightType != db::table::ColumnType::BOOL) {
                throw std::runtime_error("SEMANTIC ERROR: AND/OR requires boolean operands");
            }
            this->last_expr_type_ = db::table::ColumnType::BOOL;
            return;
        }

        // 4. Comparison logic
        if (node.op == "=" || node.op == "!=" || node.op == ">" || 
            node.op == ">=" || node.op == "<" || node.op == "<=") {
            if (leftType != rightType) {
                throw std::runtime_error("SEMANTIC ERROR: type mismatch in expression — left and right of '" 
                                         + node.op + "' must be same type");
            }
            this->last_expr_type_ = db::table::ColumnType::BOOL;
            return;
        }

        throw std::runtime_error("SEMANTIC ERROR: unknown operator '" + node.op + "'");
    }

	void SemanticAnalyzer::visit(db::parser::CreateIdxStatement& node){
		if(node.indexColumns.size() == 0){
			throw std::runtime_error("SEMANTIC ERROR: can't create index without columns");
		}

		auto t_id = Catalog::getInstance().getTableId(node.tableName);
        auto t_schema = Catalog::getInstance().getTableSchema(t_id);

		node.tableId = t_id;

		for(auto& c: node.indexColumns){
			bool exists = false;
			for(auto &t_c : t_schema->columns){
				if(c==t_c.colName){
					exists = true;
					break;
				}
			}
			if(!exists) throw std::runtime_error("SEMANTIC ERROR: column " + c + " doesn't exist in table " + node.tableName);
		}

	}

}