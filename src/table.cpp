#include<include/table.hpp>

namespace db::table{

	TableSchema::TableSchema(table_oid_t id ,std::string name , std::vector<Column> &c){
		this->tableName = name;
		this->columns = c;
	}
}