#include "include/virtual_machine/table_manager.hpp"
#include "include/catalog.hpp"
#include <stdexcept>
#include <optional>
#include <cstring>

namespace db::memory{

	TableManager::TableManager(){
		this->metadata_page_id = db::semantic::Catalog::page_count.fetch_add(1);
		db::storage::BufferPoolManager& bpm = db::storage::BufferPoolManager::getInstance();

		auto meta_guard = bpm.newPage(this->metadata_page_id);
		TableMetadata meta; 
		
		std::memset(&meta, 0, sizeof(TableMetadata));

		meta.total_pages = 0;
		meta.total_tuples = 0;

		for (int i = 0; i < 1018; i++) {
            meta.page_directory[i] = -1;
        }
		std::memcpy(meta_guard.frame_->page, &meta, sizeof(TableMetadata));
	}

	TableManager::TableManager(page_id_t existing_metadata_page_id) {
    	this->metadata_page_id = existing_metadata_page_id;
	}

	TableIterator TableManager::begin() {
        db::storage::BufferPoolManager& bpm = db::storage::BufferPoolManager::getInstance();
        auto meta_guard = bpm.fetchPage(metadata_page_id); 
        
        TableMetadata meta;
        std::memcpy(&meta, meta_guard.frame_->page, sizeof(TableMetadata));

        // If the table is completely empty, begin() just equals end()
        if (meta.total_pages == 0) {
            return end(); 
        }

        TableIterator it(*this, 0, 0);
        
        it.advanceToNext(); 
        
        return it;
    }

    TableIterator TableManager::end() {
        db::storage::BufferPoolManager& bpm = db::storage::BufferPoolManager::getInstance();
        auto meta_guard = bpm.fetchPage(metadata_page_id); 
        
        TableMetadata meta;
        std::memcpy(&meta, meta_guard.frame_->page, sizeof(TableMetadata));

        return TableIterator(*this, meta.total_pages, 0);
    }

	void TableManager::deleteTuple(const RecordID& rid){
		db::storage::BufferPoolManager& bpm = db::storage::BufferPoolManager::getInstance();

		auto data_guard = bpm.writePage(rid.page_id);
        db::memory::Page data_page(rid.page_id);
		data_page.readFromBuffer(data_guard.frame_->page);

		data_page.deleteTuple(rid);
		data_page.writeToBuffer(data_guard.frame_->page);

		auto meta_guard = bpm.writePage(this->metadata_page_id);
        TableMetadata meta;
        std::memcpy(&meta, meta_guard.frame_->page, sizeof(TableMetadata));
        
        if (meta.total_tuples > 0) {
            meta.total_tuples--;
            std::memcpy(meta_guard.frame_->page, &meta, sizeof(TableMetadata));
			return;
        }
		throw std::runtime_error("Memory Error: Couldn't Update Table Meta Data.");
	}

	void TableManager::restoreTuple(const Tuple& t){
		db::storage::BufferPoolManager& bpm = db::storage::BufferPoolManager::getInstance();

		auto data_guard = bpm.writePage(t.rid.page_id);
        db::memory::Page data_page(t.rid.page_id);
		data_page.readFromBuffer(data_guard.frame_->page);
		data_page.restoreTuple(t);
		data_page.writeToBuffer(data_guard.frame_->page);

		auto meta_guard = bpm.writePage(this->metadata_page_id);
        TableMetadata meta;
        std::memcpy(&meta, meta_guard.frame_->page, sizeof(TableMetadata));
		meta.total_tuples++;
		std::memcpy(meta_guard.frame_->page, &meta, sizeof(TableMetadata));
	}

	void TableManager::createTuple(Tuple& t){

		db::storage::BufferPoolManager& bpm = db::storage::BufferPoolManager::getInstance();

        auto meta_guard = bpm.writePage(metadata_page_id);
        TableMetadata meta;
        std::memcpy(&meta, meta_guard.frame_->page, sizeof(TableMetadata));


		if(meta.total_pages>0){
			auto idx = meta.page_directory[meta.total_pages-1];
			//now get this page.
			auto page_w = bpm.writePage(idx);
			db::memory::Page pg(idx);
			pg.readFromBuffer(page_w.frame_->page);

			if(pg.insertTuple(t)){
				t.rid.page_id = idx;
				meta.total_tuples++;
				pg.writeToBuffer(page_w.frame_->page);
				std::memcpy(meta_guard.frame_->page, &meta, sizeof(TableMetadata));
				return;
			}
		}

		//now create new page.
		auto new_idx = db::semantic::Catalog::page_count.fetch_add(1);
		auto page_w = bpm.newPage(new_idx);
		db::memory::Page pg(new_idx);
		pg.readFromBuffer(page_w.frame_->page);


		if(pg.insertTuple(t)){
			t.rid.page_id = new_idx;
			meta.total_pages++;
			meta.page_directory[meta.total_pages - 1] = new_idx;
			meta.total_tuples++;
			pg.writeToBuffer(page_w.frame_->page);
			std::memcpy(meta_guard.frame_->page, &meta, sizeof(TableMetadata));
			return ;
		}

		throw std::runtime_error("DB Error: Can't insert tuple!");
	}
	
	void TableIterator::advanceToNext() {
    auto& bpm = db::storage::BufferPoolManager::getInstance();
    while (true) {
        page_id_t data_page_id;
        uint32_t total_pages;
        {
            auto meta_guard = bpm.fetchPage(manager.metadata_page_id);
            TableMetadata meta;
            std::memcpy(&meta, meta_guard.frame_->page, sizeof(TableMetadata));
            total_pages = meta.total_pages;
            if (currentDirectoryIndex >= total_pages) return;
            data_page_id = meta.page_directory[currentDirectoryIndex];
        } 

        {
            auto data_guard = bpm.fetchPage(data_page_id); // READ
            db::memory::Page data_page(data_page_id);
            data_page.readFromBuffer(data_guard.frame_->page);

            while (currentSlotId < data_page) {
                if (data_page.isSlotValid(currentSlotId)) return;
                currentSlotId++;
            }
        } 

        currentDirectoryIndex++;
        currentSlotId = 0;
    }
}

    bool TableIterator::hasNext() const {
        db::storage::BufferPoolManager& bpm = db::storage::BufferPoolManager::getInstance();
        auto meta_guard = bpm.fetchPage(manager.metadata_page_id);
        
        TableMetadata meta;
        std::memcpy(&meta, meta_guard.frame_->page, sizeof(TableMetadata));

        return currentDirectoryIndex < meta.total_pages;
    }

    Tuple TableIterator::nextTuple() {
        if (!hasNext()) {
            throw std::runtime_error("STORAGE ERROR: No more tuples to read.");
        }

		std::optional<Tuple> result_tuple;

		{
			db::storage::BufferPoolManager& bpm = db::storage::BufferPoolManager::getInstance();
			

			auto meta_guard = bpm.writePage(manager.metadata_page_id);
			TableMetadata meta;
			std::memcpy(&meta, meta_guard.frame_->page, sizeof(TableMetadata));

			page_id_t data_page_id = meta.page_directory[currentDirectoryIndex];

			auto data_guard = bpm.writePage(data_page_id);
			db::memory::Page data_page(data_page_id);
			data_page.readFromBuffer(data_guard.frame_->page);

			result_tuple = data_page.getTuple(currentSlotId);
			result_tuple->rid.page_id = data_page_id;
			result_tuple->rid.slot_id = currentSlotId;

			currentSlotId++;
	}
        advanceToNext();

        return std::move(result_tuple.value());
    }

	
}
