#pragma once
#include<unordered_map>
#include<string>
#include<memory>
#include "include/table.hpp"
#include<mutex>
#include<cstdint>
#include "include/backend/memory_manager.hpp"
#include "include/backend/table_manager.hpp"

namespace db::semantic{
	using table_oid_t = std::uint32_t;
    using column_oid_t = std::uint32_t;


	class Catalog{
	private:
	Catalog() = default;
	std::unordered_map<std::string, table_oid_t> table_names_;
	std::unordered_map<table_oid_t, std::shared_ptr<db::table::TableSchema>> tables_;
	std::unordered_map<table_oid_t, std::unique_ptr<db::memory::TableManager>> table_managers_;
	table_oid_t next_table_id_{0};
	column_oid_t next_column_id{0};

	mutable std::mutex catalog_mutex;

	//handlers
	void storage_insertTable(const std::string& name, table_oid_t id, std::shared_ptr<db::table::TableSchema> schema);

	bool storage_tableExists(const std::string& name) const;
	table_oid_t storage_getTableId(const std::string& name) const;
	std::shared_ptr<db::table::TableSchema> storage_getTableSchema(table_oid_t table_id) const;

	public:
	static Catalog& getInstance(); //is thread safe
	Catalog(const Catalog&) = delete;
    Catalog& operator=(const Catalog&) = delete;

	//DDL
	std::shared_ptr<db::table::TableSchema> createTable(const std::string& name, std::vector<db::table::Column>& columns);
	
	table_oid_t getTableId(const std::string& name) const;
	std::shared_ptr<db::table::TableSchema> getTableSchema(table_oid_t table_id) const;
	db::memory::TableManager* getTableManager(table_oid_t table_id);
	};

}