#pragma once

#include<cstdint>
#include<vector>
#include<stdexcept>
#include<iostream>
#include<include/table.hpp>
#include<include/globals.hpp>



/*
Core in-memory row and page structures.
Page organizes tuples into fixed slots within a 4KB block using a free-space pointer—insertions allocate from the bottom and slots are at top.
*/

namespace db::memory{

	struct PageHeader {
    std::uint16_t slotCount;
    std::uint16_t freeSpacePointer;
};

	struct RecordID {
        page_id_t page_id;
        std::uint32_t slot_id;
        
        //checking if a tuple hasn't been inserted yet
        bool isValid() const { return page_id != UINT32_MAX; } 
    };

	struct Slot {
        std::uint16_t offset;
        std::uint16_t size;
    };

	class Tuple{
		public:
		RecordID rid{UINT32_MAX, UINT32_MAX};
		std::vector<char> data;
		Tuple(std::vector<char> bytes) : data(std::move(bytes)){};

		uint32_t getSize() const { return data.size(); }
		void print(std::ostream& os, const db::table::TableSchema& schema) const;
	};
	class Page{
		public:
		std::uint16_t slotCount = 0;
		bool insertTuple(Tuple &t);
		void deleteTuple(const RecordID &r);
		Tuple getTuple(std::uint16_t slot_id);
		bool isSlotValid(std::uint16_t slot_id) const;
		Page(page_id_t pId): pageId(pId), slotCount(0) , freeSpacePointer(4096){}
		char data[4096] = {0};
		void readFromBuffer(const char* raw_guard_buffer);
		void writeToBuffer(char* raw_guard_buffer) const;
		
		private:
		Page() = delete;
		std::uint16_t freeSpacePointer = 4096;
		page_id_t pageId;

	};
}