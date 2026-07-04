#pragma once

#include "include/globals.hpp"
#include "include/table.hpp"
#include <cstdint>
#include <vector>
#include <stdexcept>



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
		std::uint16_t slot_offset{0};
		std::uint16_t slot_size{0};
		std::vector<char> data;
		Tuple(std::vector<char> bytes) : data(std::move(bytes)){};

		uint32_t getSize() const { return data.size(); }
	};
	class Page{
		public:
		bool insertTuple(Tuple &t);
		void deleteTuple(const RecordID &r);
		void restoreTuple(const Tuple& t);
		Tuple getTuple(std::uint16_t slot_id);
		bool isSlotValid(std::uint16_t slot_id) const;
		Page(page_id_t pId): pageId(pId), slotCount(0) , freeSpacePointer(4096){}
		void readFromBuffer(const char* raw_guard_buffer);
		void writeToBuffer(char* raw_guard_buffer) const;
		friend bool operator<(const Page&p , int slot_count_given);
		friend bool operator<(int slot_count_given, const Page&p);
		
		private:
		Page() = delete;
		char data[4096] = {0};
		page_id_t pageId;
		std::uint16_t slotCount = 0;
		std::uint16_t freeSpacePointer = 4096;

	};
}
