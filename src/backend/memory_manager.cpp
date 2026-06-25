#include "include/backend/memory_manager.hpp"
#include <cstring>
#include <iomanip>

namespace db::memory{

	bool Page::insertTuple(Tuple& t) {
		constexpr uint16_t HEADER_SIZE = sizeof(PageHeader);
		uint16_t spaceNeeded = t.getSize() + sizeof(Slot);
		uint16_t headerEnd = HEADER_SIZE + slotCount * sizeof(Slot);  
		if (spaceNeeded > freeSpacePointer - headerEnd) {
			return false;
		}
		freeSpacePointer -= t.getSize();
		std::memcpy(&data[freeSpacePointer], t.data.data(), t.getSize());

		Slot newSlot;
		newSlot.offset = freeSpacePointer;
		newSlot.size = t.getSize();

		uint16_t slotByteOffset = HEADER_SIZE + slotCount * sizeof(Slot);
		std::memcpy(&data[slotByteOffset], &newSlot, sizeof(Slot));
		t.rid.slot_id = slotCount++;
		return true;
	}

	void Page::deleteTuple(const RecordID& r){
		constexpr uint16_t HEADER_SIZE = sizeof(PageHeader);
		auto slotPos = r.slot_id;
		if(slotPos >=slotCount){
			throw std::runtime_error("DB Error: Invalid Page Access");
		}
		// data[slotPos]
		uint16_t slotByteOffset = HEADER_SIZE + slotPos * sizeof(Slot);
		Slot* slot = reinterpret_cast<Slot*>(&data[slotByteOffset ]);
		if (slot->size == 0 && slot->offset == 0) {
    		throw std::runtime_error("DB Error: Double Deletion of a slot");
        }

		slot->size = 0;
		slot->offset = 0;

	}
	
	Tuple Page::getTuple(std::uint16_t slot_id){
		constexpr uint16_t HEADER_SIZE = sizeof(PageHeader);
		if(slot_id>=slotCount){
			throw std::runtime_error("DB Error: Invalid Slot Access");
		}

		auto slotPos = HEADER_SIZE +  slot_id * sizeof(Slot);
		Slot* slot = reinterpret_cast<Slot*>(&data[slotPos]);
		if (slot->size == 0 && slot->offset == 0) {
    		throw std::runtime_error("STORAGE Error: Tuple has been deleted");
        }


		std::vector<char> tuple_bytes(slot->size);
		std::memcpy(tuple_bytes.data(), &data[slot->offset], slot->size);

		Tuple t(tuple_bytes);
		t.rid.page_id = this->pageId;
		t.rid.slot_id = slot_id;

		return t;
	}

	bool Page::isSlotValid(std::uint16_t slot_id) const {
		constexpr uint16_t HEADER_SIZE = sizeof(PageHeader);
        // If the slot hasn't been created yet, it's not valid
        if (slot_id >= slotCount) {
            return false;
        }
        
        std::uint16_t slot_byte_position = HEADER_SIZE +  slot_id * sizeof(Slot);
        const Slot* slot = reinterpret_cast<const Slot*>(&data[slot_byte_position]);
        
        return slot->size > 0;
    }

	void Page::readFromBuffer(const char* raw_guard_buffer) {
        std::memcpy(this->data, raw_guard_buffer, 4096);
		PageHeader header;
		std::memcpy(&header, this->data, sizeof(PageHeader));
    	this->slotCount = header.slotCount;
    	this->freeSpacePointer = header.freeSpacePointer;
    }

    void Page::writeToBuffer(char* raw_guard_buffer) const {
        PageHeader header;
		header.slotCount = this->slotCount;
		header.freeSpacePointer = this->freeSpacePointer;
		std::memcpy(const_cast<char*>(this->data), &header, sizeof(PageHeader));
		std::memcpy(raw_guard_buffer, this->data, 4096);
    }

	void Tuple::print(std::ostream& os, const db::table::TableSchema& schema) const {
        uint32_t current_offset = 0;
            
        for (const auto& col : schema.columns) {
            if (col.type == db::table::ColumnType::INT) {
                int val;
                std::memcpy(&val, &data[current_offset], sizeof(int));
                os << std::left << std::setw(15) << val;
                current_offset += sizeof(int);
            } 
            else if (col.type == db::table::ColumnType::BOOL) {
                bool val;
                std::memcpy(&val, &data[current_offset], sizeof(bool));
                os << std::left << std::setw(15) << (val ? "true" : "false");
                current_offset += sizeof(bool);
            } 
            else if (col.type == db::table::ColumnType::TEXT) {
                std::string val(&data[current_offset]); 
                os << std::left << std::setw(15) << val;
                current_offset += 255; 
            }
        }
    }

	bool operator<(const Page& p, int slot_count){
		return p.slotCount < slot_count;
	}
	bool operator<(int slot_count, const Page& p){
		return slot_count < p.slotCount;
	}
}