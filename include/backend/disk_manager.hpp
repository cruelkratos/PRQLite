#pragma once
#include "include/globals.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

/*
Stores and Reads bytes from disk.
*/

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>

	#ifdef TRUE
	#undef TRUE
	#endif

	#ifdef FALSE
	#undef FALSE
	#endif
    typedef HANDLE FileHandle;
#else
    #include <unistd.h>
    #include <fcntl.h>
    typedef int FileHandle;
#endif


namespace db::storage{

	class DiskManager{
		private:
		FileHandle db_file;
		std::string file_name;
		public:
		DiskManager(const std::string& filename = db_filename);
		~DiskManager();

		//below methods assume that two threads will never fight for same page.
		void readPage(page_id_t page_id, char* frame_buffer); 
		void writePage(page_id_t page_id, const char* frame_buffer);
	};
}