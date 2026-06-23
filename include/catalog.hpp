#pragma once
#include "include/table.hpp"
#include "include/globals.hpp"
#include "include/backend/memory_manager.hpp"
#include "include/backend/table_manager.hpp"
#include <unordered_map>
#include <string>
#include <atomic>
#include <memory>
#include <mutex>
#include <fstream>
#include <cstdint>
#include <filesystem>


/*
Thread-safe singleton registry for all table metadata and storage. Maps table names to OIDs, stores TableSchemas and their corresponding TableManagers. 
Acts as the single source of truth during semantic analysis and execution—queries and executors look up tables here.
*/

namespace db::semantic{
	using table_oid_t = std::uint32_t;
    using column_oid_t = std::uint32_t;


	class Catalog{
	private:

	Catalog();
	Catalog& operator=(const Catalog&) = delete;
	Catalog(const Catalog&) = delete;
	
	std::unordered_map<std::string, table_oid_t> table_names_;
	std::unordered_map<table_oid_t, std::shared_ptr<db::table::TableSchema>> tables_;
	std::unordered_map<table_oid_t, std::shared_ptr<db::memory::TableManager>> table_managers_;
	table_oid_t next_table_id_{0};
	column_oid_t next_column_id{0};
	
	mutable std::mutex catalog_mutex;
	
	//handlers
	void storage_insertTable(const std::string& name, table_oid_t id, std::shared_ptr<db::table::TableSchema> schema);
	
	bool storage_tableExists(const std::string& name) const;
	table_oid_t storage_getTableId(const std::string& name) const;
	std::shared_ptr<db::table::TableSchema> storage_getTableSchema(table_oid_t table_id) const;
	
	public:
	~Catalog();
	inline static std::atomic<page_id_t> page_count;
	static Catalog& getInstance(); //is thread safe

	//DDL
	std::shared_ptr<db::table::TableSchema> createTable(const std::string& name, std::vector<db::table::Column>& columns);
	
	table_oid_t getTableId(const std::string& name) const;
	std::shared_ptr<db::table::TableSchema> getTableSchema(table_oid_t table_id) const;
	std::shared_ptr<db::memory::TableManager> getTableManager(table_oid_t table_id);
	void flush();
	};

}