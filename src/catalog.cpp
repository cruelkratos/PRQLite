#include "include/catalog.hpp"
#include <stdexcept>

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
		table_managers_[id] = std::make_shared	<db::memory::TableManager>();
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


	std::shared_ptr<db::memory::TableManager> Catalog::getTableManager(table_oid_t table_id) {
		//add a check for if table exists later
		try{

			return table_managers_[table_id];
		}
		catch(const std::runtime_error &e){
			throw std::runtime_error("BACKEND ERROR: CAN'T FIND TABLE MANAGER FOR TABLE_ID" + std::to_string(table_id));
		}
	}

	Catalog::Catalog(){

		std::filesystem::path projectRoot = PROJECT_ROOT; 
        std::filesystem::path db_dir = projectRoot / "data";
        std::filesystem::create_directories(db_dir);
        std::string full_path = (db_dir / ".catalog.db").string();
		
		std::ifstream in(full_path, std::ios::binary);


		//no file
		if(!in.is_open()){
			this->page_count.store(0);
			return;
		}

		//empty file
		in.seekg(0,std::ios::end);
			if(in.tellg() == 0){
				page_count.store(0);
				return;
		}

		in.seekg(0, std::ios::beg);

		try{
			uint32_t saved_page_count;
			in.read(reinterpret_cast<char*>(&saved_page_count), sizeof(uint32_t));
            page_count.store(saved_page_count);
			in.read(reinterpret_cast<char*>(&next_table_id_), sizeof(table_oid_t));
			in.read(reinterpret_cast<char*>(&next_column_id), sizeof(column_oid_t));

			for(uint32_t i = 0;i<next_table_id_;++i){
				size_t name_len;
				in.read(reinterpret_cast<char*>(&name_len), sizeof(size_t));
				std::string name(name_len, '\0');
                in.read(&name[0], name_len);

				table_oid_t oid;
				in.read(reinterpret_cast<char*>(&oid), sizeof(table_oid_t));
				page_id_t metadata_page_id;
				in.read(reinterpret_cast<char*>(&metadata_page_id), sizeof(page_id_t));

				this->table_names_[name] = oid;
				
				uint32_t num_cols;
                in.read(reinterpret_cast<char*>(&num_cols), sizeof(uint32_t));
				
				std::vector<db::table::Column> columns;

				for (uint32_t j = 0; j < num_cols; j++) {
                    db::table::Column col;
                    
                    in.read(reinterpret_cast<char*>(&col.colId), sizeof(column_oid_t));
                    
                    size_t col_name_len;
                    in.read(reinterpret_cast<char*>(&col_name_len), sizeof(size_t));
                    col.colName.resize(col_name_len); 
                    in.read(&col.colName[0], col_name_len);
                    
                    int32_t col_type_int;
                    in.read(reinterpret_cast<char*>(&col_type_int), sizeof(int32_t));
                    col.type = static_cast<db::table::ColumnType>(col_type_int);

                    columns.push_back(col);
                }
				
				if(!in.good()) throw std::runtime_error("CATALOG ERROR: File cut off during read.");

				this->tables_[oid] = std::make_shared<db::table::TableSchema>(oid, name, columns);
				this->table_managers_[oid] = std::make_shared<db::memory::TableManager>(metadata_page_id);
			}
		}
		catch(...){
			std::cerr << "WARNING: .catalog.db corrupted. Starting fresh." << std::endl;
			this->table_names_.clear();
            this->tables_.clear();
            this->table_managers_.clear();
            page_count.store(0);
            next_table_id_ = 0;
            next_column_id = 0;
		}
		
	}

	Catalog::~Catalog(){
		try{
			this->flush();
		}
		catch(const std::runtime_error &e){
			std::cerr<<e.what()<<std::endl;
			throw std::runtime_error("CATALOG ERROR: State of the Table is not stored, data might be lost.");
		}
	}

	void Catalog::flush(){
		std::lock_guard<std::mutex> lock(catalog_mutex);
		std::filesystem::path projectRoot = PROJECT_ROOT; 
        std::filesystem::path db_dir = projectRoot / "data";
        std::filesystem::create_directories(db_dir);
        std::string full_path = (db_dir / ".catalog.db").string();

		std::ofstream out(full_path, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            throw std::runtime_error("CATALOG ERROR: Can't Flush to .catalog.db");
        }
		uint32_t current_pc = page_count.load();
		out.write(reinterpret_cast<const char*>(&current_pc), sizeof(uint32_t));
        out.write(reinterpret_cast<const char*>(&next_table_id_), sizeof(table_oid_t));
        out.write(reinterpret_cast<const char*>(&next_column_id), sizeof(column_oid_t));

		for (const auto& [name, oid] : this->table_names_) {
            size_t name_len = name.size();
            out.write(reinterpret_cast<const char*>(&name_len), sizeof(size_t));
            out.write(name.data(), name_len);
		
            out.write(reinterpret_cast<const char*>(&oid), sizeof(table_oid_t));

            page_id_t metadata_page_id = this->table_managers_[oid]->getMetadataPageId();
            out.write(reinterpret_cast<const char*>(&metadata_page_id), sizeof(page_id_t));
            
            auto schema = this->tables_[oid];
			uint32_t num_cols = schema->columns.size();
            out.write(reinterpret_cast<const char*>(&num_cols), sizeof(uint32_t));
			
			for(const auto& col: schema->columns){
				out.write(reinterpret_cast<const char*>(&col.colId), sizeof(column_oid_t));


				size_t col_name_len = col.colName.size();
				out.write(reinterpret_cast<const char*>(&col_name_len), sizeof(size_t));
				out.write(col.colName.data(), col_name_len);

				int32_t col_type_int = static_cast<int32_t>(col.type);
				out.write(reinterpret_cast<const char*>(&col_type_int), sizeof(int32_t));
			}
        }
	}
}