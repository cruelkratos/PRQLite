#include "include/virtual_machine/executor.hpp"
#include <stdexcept>
#include <algorithm>


namespace db::executor{
	// InsertOperator::
	db::memory::Tuple InsertOperator::serializeToTuple(){
		auto* node_ = static_cast<db::parser::InsertStatement*>(node);
		std::vector<char> buffer;

		for(auto &v : node_->values){
			if(db::table::tokenToColumnType(v.type) == db::table::ColumnType::INT){
				int val = std::stoi(v.lexeme);
				char raw_bytes[sizeof(int)];

				std::memcpy(raw_bytes,&val,sizeof(int));
				buffer.insert(buffer.end(),raw_bytes,raw_bytes+sizeof(int));
			}

			else if(db::table::tokenToColumnType(v.type) == db::table::ColumnType::BOOL){
				bool a = false;
				if(v.type == db::lexer::TokenType::TRUE){
					a = true;
				}
				char raw_bytes[sizeof(bool)];
				std::memcpy(raw_bytes,&a,sizeof(bool));
				buffer.insert(buffer.end(),raw_bytes,raw_bytes+sizeof(bool));
			}

			else if(db::table::tokenToColumnType(v.type) == db::table::ColumnType::TEXT){
				auto str_val = v.lexeme;
				char text_bytes[255] = {0};
				size_t copy_length = std::min(int(str_val.length()), 255 - 1);
    			std::memcpy(text_bytes, str_val.c_str(), copy_length);
				buffer.insert(buffer.end(),text_bytes,text_bytes+255);
			}
			else{
				throw std::runtime_error("EXECUTION ERROR: Unsupported Column type for insertion.");
			}
		}

		return db::memory::Tuple(buffer);
	}

	std::optional<db::memory::Tuple> InsertOperator::next(){
		if(_hasInserted){
			return std::nullopt;
		}

		db::memory::Tuple physical_tuple = serializeToTuple();
		try{
			table_manager->createTuple(physical_tuple);
			db::transaction::TransactionManager::current().recordInsert(table_manager, physical_tuple);
		}
		catch(const std::runtime_error& e){
			std::cerr<<"EXECUTOR ERROR: CANT ADD TUPLE"<<std::endl;
			throw e;
		}

		_hasInserted = true;
		return physical_tuple;

	}
}
