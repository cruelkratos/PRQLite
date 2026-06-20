#pragma once
#include<atomic>
#include<cstdint>

/*
Buffer Pool will have const M frames each frame holds a page (or just page memory and we translate it to a page object later). 

Frame will maintain a pin count to stop frame from being evicted (can get unpinned) frame will also lock page access if needed
*/

namespace db::storage{
	struct Frame{
		char page[4096];
		std::atomic<std::uint32_t> pinCount {0};
		void unpin();
		bool dirtyBit;
		bool validBit;
		//maybe add a Last Access Time if we use LRU policy.
	};
	class BufferPoolManager;
};