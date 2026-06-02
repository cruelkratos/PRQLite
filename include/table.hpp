#pragma once
#include<string>
#include<vector>
#include<include/frontend/lexer.hpp>

	namespace db::table{
		using table_oid_t = uint32_t;
    	using column_oid_t = uint32_t;

		class Column{
		public:
		column_oid_t colId;
		std::string colName;
		db::lexer::TokenType type;
	};

	class TableSchema{
		public:
		TableSchema(table_oid_t id, std::string name, std::vector<Column> &c);
		table_oid_t tableId;
		std::string tableName;
		std::vector<Column> columns;
	};
}