#pragma once            
#include <string>
#include <atomic>
#include <cstdint>


using page_id_t = std::uint32_t;
using frame_id_t = std::uint32_t;
inline std::string db_filename = "prqlite.db";
inline const int PAGE_SIZE = 4096;

