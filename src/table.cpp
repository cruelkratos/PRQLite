#include "include/table.hpp"

namespace db::table{

	TableSchema::TableSchema(table_oid_t id, std::string name, const std::vector<Column> &c) 
    : tableId(id), tableName(name), columns(c) // <--- Initializer List
	{}

}