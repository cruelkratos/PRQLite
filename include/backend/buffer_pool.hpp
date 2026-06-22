#pragma once
#include<atomic>
#include<cstdint>
#include<stdexcept>
#include<memory>
#include<vector>
#include <cstdlib> 
#include<ctime>
#include "include/globals.hpp"
#include "include/backend/page_table.hpp"
#include "include/backend/memory_manager.hpp"
#include "include/backend/disk_manager.hpp"

/*
Buffer Pool will have const M frames each frame holds a page (or just page memory and we translate it to a page object later). 

Frame will maintain a pin count to stop frame from being evicted (can get unpinned) frame will also lock page access if needed
*/

const int M_FRAMES = 10;

namespace db::storage{
	class BufferPoolManager;
	

	struct Frame{
		public:
		std::atomic<std::uint32_t> pinCount {0};
		frame_id_t frame_id;
		char page[4096];
		void unpin();
		bool dirtyBit;
		std::shared_mutex latch;
		//maybe add a Last Access Time if we use LRU policy.
	};

	class PageGuard {
		protected:
		BufferPoolManager* bpm;
		Frame* frame_;

		public:
		db::memory::Page* operator->() { return reinterpret_cast< db::memory::Page*>(frame_->page); }
		// PageGuard(BufferPoolManager* bpm, Frame* frame) : bpm(bpm), frame_(frame) {}
	};

	class ReadPageGuard : public PageGuard{

		private:
		std::shared_lock<std::shared_mutex> rlock;

		public:
		ReadPageGuard(BufferPoolManager* bpm, page_id_t page_id);
		~ReadPageGuard();
	};

	class WritePageGuard : public PageGuard{
		private:
		std::unique_lock<std::shared_mutex> wlock;

		public:
		WritePageGuard(BufferPoolManager* bpm, page_id_t page_id);
		~WritePageGuard();
	};

	class BufferPoolManager{
		private:
		Frame* bufferPool;
		size_t poolSize = {M_FRAMES};
		std::unique_ptr<PageTable> pageTable;
		std::unique_ptr<DiskManager> diskManager;
		std::vector<frame_id_t> evictable_frames;
		BufferPoolManager();
		BufferPoolManager(const BufferPoolManager&) = delete;
    	BufferPoolManager& operator=(const BufferPoolManager&) = delete;
    	BufferPoolManager(BufferPoolManager&&) = delete;
    	BufferPoolManager& operator=(BufferPoolManager&&) = delete;
		Frame * fetchFrame(page_id_t page_id);
		friend class ReadPageGuard;
		friend class WritePageGuard;


		public:
		bool randomEvict(frame_id_t* victim_frame_id);
		void unpinFrame(frame_id_t frame_id);
		static BufferPoolManager& getInstance();
		~BufferPoolManager();

	};


};