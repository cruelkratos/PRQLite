#include<include/backend/disk_manager.hpp>


namespace db::storage{
	/// @brief 
	/// @param filename 
	DiskManager::DiskManager(const std::string& filename){
		this->file_name = filename;
		std::filesystem::path projectRoot = PROJECT_ROOT; //defined at compile time.

		std::filesystem::path db_dir = projectRoot / "data";
		std::filesystem::create_directories(db_dir);
		std::filesystem::path file = db_dir / filename;
		std::string full_path = file.string();


		#if defined(_WIN32)
		this->db_file = CreateFileA(
			full_path.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (db_file == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("INIT ERROR: Failed to open/create .db file on Windows. Path: " + full_path);
        }
		#else
		db_file = open(full_path.c_str(),O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
		if(db_file == -1){
			throw std::runtime_error("INIT ERROR: Failed to open/create .db file on POSIX. Path: " + full_path);
		}
		#endif

	}

	void DiskManager::readPage(page_id_t page_id, char* frame_buffer){
		uint64_t offset = static_cast<uint64_t>(page_id) * PAGE_SIZE;
		#if defined(_WIN32)
		OVERLAPPED overlapped = {0};
		overlapped.Offset = offset&0xFFFFFFFF; //bottom32 bits
		overlapped.OffsetHigh = offset>>32; //top 32 bits
		DWORD bytes;
		if(!ReadFile(this->db_file,frame_buffer,PAGE_SIZE,&bytes,&overlapped)){
			throw std::runtime_error("STORAGE ERROR: Windows ReadFile failed.");
		}
		if(bytes!=PAGE_SIZE && bytes !=0){
			throw std::runtime_error("STORAGE ERROR: Full page wasn't read.");
		}

		#else 
		ssize_t bytes = pread(db_file, frame_buffer, PAGE_SIZE, offset);
		if (bytes == -1) {
            throw std::runtime_error("STORAGE ERROR: POSIX pread failed.");
        }
        if (bytes != PAGE_SIZE && bytes != 0) {
            throw std::runtime_error("STORAGE ERROR: Full page wasn't read.");
        }
		#endif
	}

	void DiskManager::writePage(page_id_t page_id, const char* frame_buffer){
		uint64_t offset = static_cast<uint64_t>(page_id) * PAGE_SIZE;
		#if defined(_WIN32)
		OVERLAPPED overlapped = {0};
        overlapped.Offset = offset & 0xFFFFFFFF;
        overlapped.OffsetHigh = (offset >> 32);

        DWORD bytes_written;
        if (!WriteFile(db_file, frame_buffer, PAGE_SIZE, &bytes_written, &overlapped)) {
            throw std::runtime_error("STORAGE ERROR: Windows WriteFile failed.");
        }

		#else

		ssize_t bytes_written = pwrite(db_file, frame_buffer, PAGE_SIZE, offset);
        
        if (bytes_written != PAGE_SIZE) {
            throw std::runtime_error("STORAGE ERROR: POSIX pwrite failed.");
        }

		#endif

	}

	DiskManager::~DiskManager(){
		#if defined(_WIN32)
		if(db_file !=INVALID_HANDLE_VALUE){
			CloseHandle(db_file);
		}
		#else
		if(db_file !=-1){
			close(db_file);
		}
		#endif
	}
}