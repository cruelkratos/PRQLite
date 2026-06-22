#pragma once
#include<atomic>
#include<cstdint>
#include<string>

using page_id_t = std::uint32_t;
using frame_id_t = std::uint32_t;
inline std::string db_filename = "prqlite.db";
inline const int PAGE_SIZE = 4096;


namespace global{
	inline std::atomic<page_id_t> page_count(0);
}