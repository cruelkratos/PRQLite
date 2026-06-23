#pragma once
#include "include/frontend/lexer.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>


/*
Defines table metadata primitives: Column wraps name, type, and ID; TableSchema groups columns under a table name and ID.
*/


namespace db::table{

	enum class ColumnType { INT, TEXT, BOOL };

	inline ColumnType tokenToColumnType(db::lexer::TokenType t) {
		switch (t) {
			case db::lexer::TokenType::INT:    return ColumnType::INT;
			case db::lexer::TokenType::TEXT:   return ColumnType::TEXT;
			case db::lexer::TokenType::BOOL:   return ColumnType::BOOL;
			// literal tokens map to their storage type
			case db::lexer::TokenType::NUMBER: return ColumnType::INT;
			case db::lexer::TokenType::STRING: return ColumnType::TEXT;
			case db::lexer::TokenType::TRUE:
			case db::lexer::TokenType::FALSE:  return ColumnType::BOOL;
			default:
				throw std::runtime_error("SEMANTIC ERROR: '" + std::to_string((int)t) + "' is not a valid column type");
		}
		}

	using table_oid_t = std::uint32_t;
	using column_oid_t = std::uint32_t;
	
	class Column{
		public:
		column_oid_t colId;
		std::string colName;
		ColumnType type;
	};
	
	class TableSchema{
		public:
		TableSchema(table_oid_t id, std::string name, const std::vector<Column> &c);
		table_oid_t tableId;
		std::string tableName;
		std::vector<Column> columns;
};


}