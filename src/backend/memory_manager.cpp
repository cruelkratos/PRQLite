#include<include/backend/memory_manager.hpp>
#include <cstring>

namespace db::memory{

	bool Page::insertTuple(Tuple& t){
		std::uint16_t spaceNeeded = t.getSize() + sizeof(Slot);
		std::uint16_t headerEnd = slotCount * sizeof(Slot);
		if(spaceNeeded > freeSpacePointer - headerEnd){
			return false;
		}

		freeSpacePointer -=t.getSize();

		std::memcpy(&data[freeSpacePointer], t.data.data(), t.getSize());

		Slot newSlot;
		newSlot.offset = freeSpacePointer;
		newSlot.size = t.getSize();

		std::memcpy(&data[slotCount] , &newSlot,sizeof(Slot));
		t.rid.slot_id = slotCount++;
		return true;
	}

	void Page::deleteTuple(const RecordID& r){
		auto slotPos = r.slot_id;
		if(slotPos >=slotCount){
			throw std::runtime_error("DB Error: Invalid Page Access");
		}
		// data[slotPos]
		slotPos *= sizeof(Slot);
		Slot* slot = reinterpret_cast<Slot*>(&data[slotPos]);
		if (slot->size == 0 && slot->offset == 0) {
    		throw std::runtime_error("DB Error: Double Deletion of a slot");
        }

		slot->size = 0;
		slot->offset = 0;

	}
	
	Tuple Page::getTuple(std::uint16_t slot_id){
		if(slot_id>=slotCount){
			throw std::runtime_error("DB Error: Invalid Slot Access");
		}

		auto slotPos = slot_id * sizeof(Slot);
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
        // If the slot hasn't been created yet, it's not valid
        if (slot_id >= slotCount) {
            return false;
        }
        
        std::uint16_t slot_byte_position = slot_id * sizeof(Slot);
        const Slot* slot = reinterpret_cast<const Slot*>(&data[slot_byte_position]);
        
        return slot->size > 0;
    }
}