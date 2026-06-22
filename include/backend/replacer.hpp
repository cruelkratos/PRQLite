#pragma once
#include <vector>
#include <unordered_map>
#include <mutex>
#include "include/globals.hpp"

namespace db::storage {

	class Replacer{
		protected:
		std::mutex latch_;
		public:
		virtual bool Evict(frame_id_t* victim) = 0;
        virtual void RecordUnpin(frame_id_t frame_id) = 0;
        virtual void RecordPin(frame_id_t frame_id) = 0;
		virtual ~Replacer() = default;

	};

    class RandomReplacer : public Replacer {
    private:
        std::vector<frame_id_t> frames_;
        std::unordered_map<frame_id_t, size_t> frame_indices_;

    public:
        RandomReplacer() = default;
        ~RandomReplacer() = default;

        bool Evict(frame_id_t* victim) override;
        void RecordUnpin(frame_id_t frame_id) override;
        void RecordPin(frame_id_t frame_id) override;
    };
}