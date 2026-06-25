#include "include/backend/buffer_pool.hpp"
#include <cstring>
namespace db::storage{

	void Frame::unpin(){
		if(this->pinCount.load() == 0){
			throw std::runtime_error("STORAGE ERROR: Frame is already free.");
		}

		this->pinCount.fetch_sub(1);
	}

	ReadPageGuard::ReadPageGuard(BufferPoolManager* bpm, page_id_t page_id){
		this->bpm = bpm;
        frame_ = bpm->fetchFrame(page_id);
        rlock = std::shared_lock<std::shared_mutex>(frame_->latch);
	}

	ReadPageGuard::~ReadPageGuard(){
		if(frame_ != nullptr){
			bpm->unpinFrame(frame_->frame_id);
		}
	}

	WritePageGuard::WritePageGuard(BufferPoolManager* bpm, page_id_t page_id){
		this->bpm = bpm;
        frame_ = bpm->fetchFrame(page_id);
        wlock = std::unique_lock<std::shared_mutex>(frame_->latch);
	}

	WritePageGuard::WritePageGuard(BufferPoolManager* bpm, Frame* frame){
		frame_ = frame;
		this->bpm = bpm;
		wlock = std::unique_lock<std::shared_mutex>(frame_->latch);
	}

	WritePageGuard BufferPoolManager::newPage(page_id_t page_id) {
    	return WritePageGuard(this, allocateNewFrame(page_id)); 
	}

	WritePageGuard::~WritePageGuard(){
		if(frame_ != nullptr){
			frame_->dirtyBit = true;
			bpm->unpinFrame(frame_->frame_id);
		}
	}

	BufferPoolManager::BufferPoolManager() {
    // initialize members
		bufferPool = new Frame[this->poolSize];
		diskManager = std::make_unique<DiskManager>();
		pageTable = std::make_unique<PageTable>();
		replacer = std::make_unique<RandomReplacer>();
		for (frame_id_t i = 0; i < poolSize; i++) {
            free_list_.push_back(i);
            bufferPool[i].frame_id = i; 
        }
	
	}

	BufferPoolManager& BufferPoolManager::getInstance() {
    	static BufferPoolManager instance;
    	return instance;
	}
	BufferPoolManager::~BufferPoolManager(){
		try{
			this->flushPagestoDisk();
		}catch(...){
			std::cerr<<"Critical Error: Pages Might be Corrupted.\n";
		}
		delete [] bufferPool;
	}

	Frame* BufferPoolManager::fetchFrame(page_id_t page_id){
		frame_id_t f;
		if(this->pageTable->get(page_id,f)){
			Frame* frame =  &bufferPool[f];
			frame->pinCount.fetch_add(1);
			if (frame->pinCount.load() == 1) {
            	replacer->RecordPin(f);
        	}
			return frame;
		}
		//Not in Memory must load from disk.

		if (!free_list_.empty()) {
        	f = free_list_.front();
        	free_list_.pop_front();
        	// return &bufferPool[out_frame_id]; 
    	} else{
			if (!replacer->Evict(&f)) {
            throw std::runtime_error("OUT OF MEMORY ERROR: All Frames are currently pinned!");
       		}
			Frame* victim = &bufferPool[f];
			if (victim->dirtyBit) {
            this->diskManager->writePage(victim->page_id,victim->page); //FIXXXXX
        	}
			this->pageTable->remove(victim->page_id);
		}
		
		Frame* new_frame = &bufferPool[f];

		std::memset(new_frame->page, 0, 4096);


		this->diskManager->readPage(page_id, new_frame->page);
		this->pageTable->set(page_id, f);
		new_frame->dirtyBit = false;
		new_frame->pinCount.store(1);
		new_frame->page_id = page_id;

		return new_frame;
	}

	void BufferPoolManager::unpinFrame(frame_id_t frame_id){
		this->bufferPool[frame_id].unpin();
		if(this->bufferPool[frame_id].pinCount.load() == 0){
			this->replacer->RecordUnpin(frame_id);
		}
	}

	void BufferPoolManager::pinFrame(frame_id_t frame_id){
		auto pcount = this->bufferPool[frame_id].pinCount.fetch_add(1);
		if(pcount == 0) this->replacer->RecordPin(frame_id);
	}

	ReadPageGuard BufferPoolManager::fetchPage(page_id_t page_id) {
        return ReadPageGuard(this, page_id);
    }

    WritePageGuard BufferPoolManager::writePage(page_id_t page_id) {
        return WritePageGuard(this, page_id);
    }


	Frame* BufferPoolManager::allocateNewFrame(page_id_t new_page_id) {
		frame_id_t target_frame_id;

		if (!free_list_.empty()) {
			target_frame_id = free_list_.front();
			free_list_.pop_front();
		} 
		else {
			if (!replacer->Evict(&target_frame_id)) {
				throw std::runtime_error("OUT OF MEMORY ERROR!");
			}
			Frame* victim = &bufferPool[target_frame_id];
			
			// Save old data if needed
			if (victim->dirtyBit) {
				this->diskManager->writePage(victim->page_id, victim->page);
			}
			this->pageTable->remove(victim->page_id);
		}

		Frame* new_frame = &bufferPool[target_frame_id];


		std::memset(new_frame->page, 0, 4096);

		db::memory::PageHeader header;
		header.slotCount = 0;
		header.freeSpacePointer = 4096; 
		std::memcpy(new_frame->page, &header, sizeof(db::memory::PageHeader));

		this->pageTable->set(new_page_id, target_frame_id);
		new_frame->page_id = new_page_id;
		new_frame->dirtyBit = true;  
		new_frame->pinCount.store(1);

		return new_frame;
	}

	void BufferPoolManager::flushPagestoDisk(){
		//not to be called unless needed by DB to flush pages in shutdown.
		//we can assume this is only called if interrupt is raised hence will be called only once.

		for(size_t i = 0; i < this->poolSize; ++i){
			if(bufferPool[i].dirtyBit){
				try{
					diskManager->writePage(bufferPool[i].page_id,bufferPool[i].page);
					bufferPool[i].dirtyBit = false;
				}
				catch(const std::runtime_error &e){
					std::cerr << "Buffer Pool Error: Failed to flush page " 
                              << bufferPool[i].page_id << " to disk. Reason: " 
                              << e.what() << std::endl;
				}
			}
		}

	}

}