#include<catalog.hpp>
#include<stdexcept>

namespace db::semantic{
	
	Catalog& Catalog::getInstance(){
		static Catalog instance;
		return instance;
	}
	std::shared_ptr<db::table::TableSchema> Catalog::createTable(const std::string& name, std::vector<db::table::Column>& columns) {
		std::unique_lock<std::mutex> lock(catalog_mutex);

		if(storage_tableExists(name)){
			throw std::runtime_error("SEMANTIC ERROR: two tables with same name attempted to be created.");
		}

		auto new_id = next_table_id_++;
		for (size_t i = 0; i < columns.size(); i++) {
        	columns[i].colId = next_column_id++;  // global counter in Catalog
    	}
		auto new_schema = std::make_shared<db::table::TableSchema>(new_id,name,columns);

		storage_insertTable(name, new_id, new_schema);

		return new_schema;
	}

	table_oid_t Catalog::getTableId(const std::string& name) const{
		std::lock_guard<std::mutex> lock(catalog_mutex);

		if (!storage_tableExists(name)) {
            throw std::runtime_error("SEMANTIC ERROR: This Table Doesn't Exist.");
        }

		return storage_getTableId(name);
	}

	std::shared_ptr<db::table::TableSchema> Catalog::getTableSchema(table_oid_t table_id) const{
		std::lock_guard<std::mutex> lock(catalog_mutex);

		auto schema = storage_getTableSchema(table_id);
        if (schema == nullptr) {
            throw std::runtime_error("SEMANTIC ERROR: This Table Doesn't Exist.");
        }

        return schema;
	}


	void Catalog::storage_insertTable(const std::string& name, table_oid_t id, std::shared_ptr<db::table::TableSchema> schema) {
        table_names_[name] = id;
        tables_[id] = schema;
    }

    bool Catalog::storage_tableExists(const std::string& name) const {
        return table_names_.find(name) != table_names_.end();
    }

    table_oid_t Catalog::storage_getTableId(const std::string& name) const {
       
        return table_names_.at(name);
    }

    std::shared_ptr<db::table::TableSchema> Catalog::storage_getTableSchema(table_oid_t table_id) const {
        auto it = tables_.find(table_id);
        if (it == tables_.end()) {
            return nullptr;
        }
        return it->second;
    }

}