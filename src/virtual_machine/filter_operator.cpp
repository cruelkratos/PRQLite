#include "include/virtual_machine/executor.hpp"


namespace db::executor{


	FilterOperator::FilterOperator(db::parser::ASTNode* select_node, AbstractExecutor* child){
		this->node = select_node;
		this->_child = child;
	}

	void FilterOperator::init(){
		if(_child == nullptr || node == nullptr){
			throw std::runtime_error("Executor Error: Child/Node Object is NULL.");
		}
		_child->init();
		this->child_schema = _child->getOutputSchema();

		//PRE-PROCESSING
		uint32_t current_offset = 0;
        for (const auto& col : child_schema.columns) {
            column_offsets[col.colName] = current_offset;
            column_types[col.colName] = col.type;

            if (col.type == db::table::ColumnType::INT) current_offset += sizeof(int);
            else if (col.type == db::table::ColumnType::BOOL) current_offset += sizeof(bool);
            else if (col.type == db::table::ColumnType::TEXT) current_offset += 255;
        }


		// this->whereClause = static_cast<db::parser::SelectStatement*>(node)->whereClause;
		if(this->whereClause != nullptr) this->hasClause = true;
	}

	void FilterOperator::setWhereClause(db::parser::ASTNode* _whereClause){
		this->whereClause = _whereClause;
	}

	std::optional<db::memory::Tuple> FilterOperator::next(){
		if(!hasClause){
			return _child->next();
		}

		while(auto tuple = _child->next()){
			if(consider(*tuple)){
				return tuple;
			}
		}

		return std::nullopt;
	}

	bool FilterOperator::consider(const db::memory::Tuple& tuple){
		SQLValue result = evaluateExpr(whereClause, tuple);
		return (std::holds_alternative<bool>(result) && std::get<bool>(result));
	}


	SQLValue FilterOperator::evaluateExpr(db::parser::ASTNode* expr, const db::memory::Tuple& tuple) {
        
        
        // BASE CASE 1: It is a column name (Identifier)
        if (auto* id_node = dynamic_cast<db::parser::IdentifierExpr*>(expr)) {
			if (column_offsets.find(id_node->name) == column_offsets.end()) {
                throw std::runtime_error("Column not found in WHERE clause: " + id_node->name);
            }
			uint32_t offset = column_offsets.at(id_node->name);
            auto type = column_types.at(id_node->name);

			if (type == db::table::ColumnType::INT) {
                int val;
                std::memcpy(&val, &tuple.data[offset], sizeof(int));
                return val;
            } 
            else if (type == db::table::ColumnType::BOOL) {
                bool val;
                std::memcpy(&val, &tuple.data[offset], sizeof(bool));
                return val;
            } 
            else if (type == db::table::ColumnType::TEXT) {
                return std::string(&tuple.data[offset]); 
            }
        }

        // BASE CASE 2: It is a hardcoded value (Literal)
        else if (auto* lit_node = dynamic_cast<db::parser::LiteralExpr*>(expr)) {
            if (lit_node->value.type == db::lexer::TokenType::NUMBER) {
                return std::stoi(lit_node->value.lexeme);
            } else if (lit_node->value.type == db::lexer::TokenType::STRING) {
                return lit_node->value.lexeme;
            } else if (lit_node->value.type == db::lexer::TokenType::TRUE) {
                return true;
            } else if (lit_node->value.type == db::lexer::TokenType::FALSE) {
                return false;
            }
        }

        // RECURSIVE CASE: It is an Operation (>, <, =, AND, OR)
        else if (auto* bin_node = dynamic_cast<db::parser::BinaryExpr*>(expr)) {
            SQLValue left_val = evaluateExpr(bin_node->left, tuple);
            SQLValue right_val = evaluateExpr(bin_node->right, tuple);

            if (bin_node->op == "=") {
                return left_val == right_val; 
            } 
            else if (bin_node->op == "!=") {
                return left_val != right_val;
            }
            else if (bin_node->op == ">") {
                return std::get<int>(left_val) > std::get<int>(right_val);
            }
            else if (bin_node->op == "<") {
                return std::get<int>(left_val) < std::get<int>(right_val);
            }
            else if (bin_node->op == "AND") {
                return std::get<bool>(left_val) && std::get<bool>(right_val);
            }
            else if (bin_node->op == "OR") {
                return std::get<bool>(left_val) || std::get<bool>(right_val);
            }
        }

        throw std::runtime_error("Unsupported expression in WHERE clause.");
    }
}
