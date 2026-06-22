#include<include/backend/buffer_pool.hpp>

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
		delete [] bufferPool;
	}

	Frame* BufferPoolManager::fetchFrame(page_id_t page_id){
		//add pin method
		frame_id_t f;
		if(this->pageTable->get(page_id,f)){
			return &bufferPool[f];
		}
		//Not in Memory must load from disk.

		if (!free_list_.empty()) {
        	auto out_frame_id = free_list_.front();
        	free_list_.pop_front();
        	return &bufferPool[out_frame_id]; 
    	}

		frame_id_t victim_frame_id;
		if(!replacer->Evict(&victim_frame_id)){
			throw std::runtime_error("OUT OF MEMORY ERROR: All Frames are currently pinned!");
		}
		
		if(bufferPool[victim_frame_id].dirtyBit){
			this->diskManager->writePage(page_id,bufferPool[victim_frame_id].page);
		}

		this->diskManager->readPage(page_id,bufferPool[victim_frame_id].page);
		return &bufferPool[victim_frame_id];
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

}