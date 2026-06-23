#pragma once
#include <vector>
#include <memory>
#include "include/backend/memory_manager.hpp"
#include "include/globals.hpp"
#include "include/backend/buffer_pool.hpp"

/*
Manages physical table storage as a sequence of fixed-size pages, and provides sequential iteration over tuples via TableIterator. Iterator state lives on the stack (current page and slot) and advances through valid slots, 
skipping deleted or invalid ones.
*/



namespace db::memory{

	class TableManager;

	struct TableMetadata{
		//fits exactly in a page.
		page_id_t total_pages;
		uint32_t total_tuples;

		page_id_t page_directory[1017];
	};

	class TableIterator{
		private:
		const TableManager& manager;
		size_t currentPageIdx=0;
        std::uint16_t currentSlotId=0;
		uint32_t currentDirectoryIndex;
		

		public:
		bool hasNext() const;
		Tuple nextTuple();
		void advanceToNext();
		TableIterator(const TableManager& m) : manager(m) {advanceToNext();}
		TableIterator(const TableManager& m , uint32_t dirIndex, std::uint16_t curr_slotId) : manager(m) , currentDirectoryIndex(dirIndex) , currentSlotId(curr_slotId){advanceToNext();}

	};

	class TableManager{
		private:
		friend class TableIterator;
		std::vector<std::unique_ptr<Page>> _pageList;
		//use a table meta data page to get the pagelist indices.
		//replace pagelist_ logic by querying the read/write page guards
		//add a way to change metadata page if pages are added/removed 
		std::atomic<page_id_t> pageNos{0};
		page_id_t metadata_page_id;
		
		public:
		TableManager();
		TableIterator begin();
		TableIterator end();
		void createTuple(Tuple &t);
		TableManager(page_id_t existing_metadata_page_id);
		page_id_t getMetadataPageId() const { return metadata_page_id; }

	};

}