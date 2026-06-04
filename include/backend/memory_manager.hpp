#pragma once

#include<cstdint>
#include<vector>
#include<stdexcept>

namespace db::memory{

	struct RecordID {
        std::uint32_t page_id;
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
	};
	class Page{
		public:
		std::uint16_t slotCount = 0;
		bool insertTuple(Tuple &t);
		void deleteTuple(const RecordID &r);
		Tuple getTuple(std::uint16_t slot_id);
		bool isSlotValid(std::uint16_t slot_id) const;
		Page(std::uint32_t pId): pageId(pId){}
		
		private:
		Page() = delete;
		std::uint16_t freeSpacePointer = 4096;
		char data[4096] = {0};
		std::uint32_t pageId;

	};
}