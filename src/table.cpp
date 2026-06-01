#include<include/table.hpp>

namespace db::table{

	TableSchema::TableSchema(std::string name , std::vector<Column> &c){
		this->tableName = name;
		this->columns = c;
	}
}