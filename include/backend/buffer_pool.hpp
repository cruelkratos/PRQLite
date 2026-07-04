#pragma once
#include "include/globals.hpp"
#include "include/backend/page_table.hpp"
#include "include/virtual_machine/memory_manager.hpp"
#include "include/backend/disk_manager.hpp"
#include "include/backend/replacer.hpp"
#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <memory>
#include <vector>
#include <cstdlib> 
#include <list>
#include <ctime>
#include <unordered_set>


/*
Buffer Pool will have const M frames each frame holds a page (or just page memory and we translate it to a page object later). 

Frame will maintain a pin count to stop frame from being evicted (can get unpinned) frame will also lock page access if needed
*/

const int M_FRAMES = 40;

namespace db::storage{
	class BufferPoolManager;
	

	struct Frame{
		public:
		std::atomic<std::uint32_t> pinCount {0};
		frame_id_t frame_id;
		page_id_t page_id{UINT32_MAX};
		char page[4096];
		void unpin();
		bool dirtyBit{false};
		bool committedDirty{false};
		std::shared_mutex latch;
		//maybe add a Last Access Time if we use LRU policy.
	};

	class PageGuard {
		protected:
		BufferPoolManager* bpm;
		
		public:
		Frame* frame_;
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
		WritePageGuard(BufferPoolManager* bpm, Frame* frame);
		WritePageGuard(BufferPoolManager* bpm, page_id_t page_id);
		~WritePageGuard();
	};

	class BufferPoolManager{
		private:
		Frame* bufferPool;
		size_t poolSize = {M_FRAMES};
		std::unique_ptr<PageTable> pageTable;
		std::unique_ptr<DiskManager> diskManager;
		std::list<frame_id_t> free_list_;
		std::unique_ptr<Replacer> replacer;
		BufferPoolManager();
		BufferPoolManager(const BufferPoolManager&) = delete;
    	BufferPoolManager& operator=(const BufferPoolManager&) = delete;
    	BufferPoolManager(BufferPoolManager&&) = delete;
    	BufferPoolManager& operator=(BufferPoolManager&&) = delete;
		Frame * fetchFrame(page_id_t page_id);
		void unpinFrame(frame_id_t frame_id);
		void pinFrame(frame_id_t frame_id);
		friend class ReadPageGuard;
		friend class WritePageGuard;


		public:
		ReadPageGuard fetchPage(page_id_t page_id);
		WritePageGuard writePage(page_id_t page_id);
		static BufferPoolManager& getInstance();
		~BufferPoolManager();
		Frame* allocateNewFrame(page_id_t new_page_id);
		WritePageGuard newPage(page_id_t page_id);
		void flushPagestoDisk(); //only to be used if DB is shutting down.
		void markDirtyPagesCommitted(const std::unordered_set<page_id_t>& page_ids);
		void flushCommittedPagestoDisk();

	};


};
