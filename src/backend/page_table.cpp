#include "include/backend/page_table.hpp"

namespace db::storage{
	bool PageTable::get(const page_id_t& key, frame_id_t& value) const{
			std::shared_lock lock(mutex_);

			auto it = map_.find(key);
			if(it == map_.end()){
				return false;
			}
			value = it->second;
			return true;
	}

	void PageTable::set(const page_id_t& key, const frame_id_t& value){
		std::unique_lock lock(mutex_);
		map_[key] = value;
	}

	void PageTable::remove(const page_id_t& key){
		std::unique_lock lock(mutex_);
		auto it = map_.find(key);
		if(it == map_.end()){
			throw std::runtime_error("PAGING ERROR: This key doesn't exist.");
		}
		map_.erase(map_.find(key));
	}
}