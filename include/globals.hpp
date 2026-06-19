#pragma once
#include<atomic>
#include<cstdint>

using page_id_t = std::uint32_t;
namespace global{
	inline std::atomic<page_id_t> page_count(0);
}