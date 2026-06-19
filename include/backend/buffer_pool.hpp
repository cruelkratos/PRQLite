#pragma once

/*
Buffer Pool will have const M frames each frame holds a page (or just page memory and we translate it to a page object later). 

Frame will maintain a pin count to stop frame from being evicted (can get unpinned) frame will also lock page access if needed
*/

namespace db::storage{
	struct Frame;
	class BufferPoolManager;
};