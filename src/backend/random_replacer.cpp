#include "include/backend/replacer.hpp"
#include <cstdlib> 

namespace db::storage {

    void RandomReplacer::RecordUnpin(frame_id_t frame_id) {
        std::lock_guard<std::mutex> lock(latch_);

        if (frame_indices_.find(frame_id) != frame_indices_.end()) {
            return; 
        }

        frames_.push_back(frame_id);
        
        frame_indices_[frame_id] = frames_.size() - 1; 
    }

    void RandomReplacer::RecordPin(frame_id_t frame_id) {
        std::lock_guard<std::mutex> lock(latch_);

        auto it = frame_indices_.find(frame_id);
        if (it == frame_indices_.end()) {
            return; 
        }

        size_t index_to_remove = it->second;
        frame_id_t last_frame_id = frames_.back();

        frames_[index_to_remove] = last_frame_id;
        
        frame_indices_[last_frame_id] = index_to_remove;

        frames_.pop_back();
        
        frame_indices_.erase(frame_id);
    }

    bool RandomReplacer::Evict(frame_id_t* victim) {
        std::lock_guard<std::mutex> lock(latch_);

        if (frames_.empty()) {
            return false; // OOM!
        }


        size_t random_index = rand() % frames_.size();
        *victim = frames_[random_index];


        frame_id_t last_frame_id = frames_.back();
        frames_[random_index] = last_frame_id;
        frame_indices_[last_frame_id] = random_index;
        
        frames_.pop_back();
        frame_indices_.erase(*victim);

        return true;
    }
}