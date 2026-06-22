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
		evictable_frames.reserve(this->poolSize);
		for(int i = 0;i<this->poolSize;i++){
			evictable_frames.push_back(i);
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
		frame_id_t victim_frame_id;
		if(!randomEvict(&victim_frame_id)){
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
			evictable_frames.push_back(frame_id);
		}
	}

	bool BufferPoolManager::randomEvict(frame_id_t * victim_frame_id){
		if(evictable_frames.empty()){
			return false;
		}
		std::srand(std::time(nullptr));
		size_t random_index = std::rand()%(evictable_frames.size());
		*victim_frame_id = evictable_frames[random_index];
		evictable_frames.erase(evictable_frames.begin() + random_index);
		return true;

	}

}