#pragma once
#include<string>
#include<include/frontend/lexer.hpp>

	namespace db::table{
		class Column{
		public:
		std::string colName;
		db::lexer::TokenType type;
	};

	class TableSchema{
		public:
		TableSchema(std::string name, std::vector<Column> &c);
		std::string tableName;
		std::vector<Column> columns;
	};
}