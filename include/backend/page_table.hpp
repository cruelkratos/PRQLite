#pragma once
#include <shared_mutex>
#include <unordered_map>
#include <stdexcept>
#include "include/globals.hpp"
#include <mutex>
/*
High Throughput Concurrent Page Table which will tell what pages are in memory and what aren't

points to the frame_id and returns page from there else trap to BufferPoolManager.
*/
 
namespace db::storage{
	class PageTable{
		private:
		std::unordered_map<page_id_t,frame_id_t> map_;
		mutable std::shared_mutex mutex_;

		public:

		bool get(const page_id_t& key, frame_id_t& value) const;

		void set(const page_id_t& key, const frame_id_t& value);

		void remove(const page_id_t& key);
	};

}