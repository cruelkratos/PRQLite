#include "include/backend/executor.hpp"

namespace db::executor{
	ProjectionOperator::ProjectionOperator(db::parser::ASTNode* select_node, AbstractExecutor* child){
		this->node = select_node;
		this->_child = child;
	}

	void ProjectionOperator::init(){
		if(_child == nullptr || node == nullptr){
			throw std::runtime_error("Executor Error: Child/Node Object is NULL.");
		}
		_child->init();
		auto sel_node = static_cast<db::parser::SelectStatement*>(node);	
		this->colList = sel_node->resolvedColumns;
		this->projected_schema.columns = this->colList;
		auto full_schema = db::semantic::Catalog::getInstance().getTableSchema(
            db::semantic::Catalog::getInstance().getTableId(sel_node->tableName)
        );

		uint32_t current_offset = 0;

		for(const auto& o_col: full_schema->columns){
			uint32_t col_size = 0;
			if(o_col.type == db::table::ColumnType::INT) col_size = sizeof(int);
			else if (o_col.type == db::table::ColumnType::BOOL) col_size = sizeof(bool);
            else if (o_col.type == db::table::ColumnType::TEXT) col_size = 255;
			else throw std::runtime_error("EXECUTION ERROR: WRONG COLUMN TYPE");

			bool is_selected = false;
			for(const auto& sel_col : colList){
				if(o_col.colName == sel_col.colName){
					is_selected = true;
					break;
				}
			}

			if(is_selected){
				this->projection_offsets.push_back({current_offset, col_size});
			}
			current_offset += col_size; 
		}
	}

	std::optional<db::memory::Tuple> ProjectionOperator::next(){
		auto child_tuple = _child->next();
		if (!child_tuple) return std::nullopt;

		std::vector<char> new_buffer;
		for(const auto& [offset, size] : this->projection_offsets){
			auto start_ptr = child_tuple->data.begin() + offset;
			new_buffer.insert(new_buffer.end(), start_ptr, start_ptr + size);
		}

		return db::memory::Tuple(new_buffer);
	}
}