#include "include/recovery/log_record.hpp"

#include <cstring>
#include <stdexcept>

namespace db::recovery{
	namespace{
		template <typename T>
		void append(std::vector<char>& out, const T& value){
			const char* bytes = reinterpret_cast<const char*>(&value);
			out.insert(out.end(), bytes, bytes + sizeof(T));
		}

		template <typename T>
		T read(const std::vector<char>& in, std::size_t& offset){
			if(offset + sizeof(T) > in.size()){
				throw std::runtime_error("RECOVERY Error: Invalid log record");
			}
			T value;
			std::memcpy(&value, in.data() + offset, sizeof(T));
			offset += sizeof(T);
			return value;
		}

		std::vector<char> read_bytes(const std::vector<char>& in, std::size_t& offset, std::size_t len){
			if(offset + len > in.size()){
				throw std::runtime_error("RECOVERY Error: Invalid log record");
			}
			std::vector<char> bytes(in.begin() + offset, in.begin() + offset + len);
			offset += len;
			return bytes;
		}
	}

	void LogRecord::set_before(const db::memory::Tuple& tuple){
		before = tuple.data;
		b_len = before.size();
	}

	void LogRecord::set_after(const db::memory::Tuple& tuple){
		after = tuple.data;
		a_len = after.size();
	}

	std::vector<char> LogRecord::serialize() const{
		std::vector<char> out;
		const std::size_t before_len = before.size();
		const std::size_t after_len = after.size();
		const auto type = static_cast<std::uint32_t>(r_type);
		out.reserve(sizeof(lsn) + sizeof(prev_lsn) + sizeof(txn_id) + sizeof(type) +
			sizeof(pid) + sizeof(slot_id) + sizeof(before_len) + before_len +
			sizeof(after_len) + after_len);

		append(out, lsn);
		append(out, prev_lsn);
		append(out, txn_id);
		append(out, type);
		append(out, pid);
		append(out, slot_id);
		append(out, before_len);
		out.insert(out.end(), before.begin(), before.end());
		append(out, after_len);
		out.insert(out.end(), after.begin(), after.end());
		return out;
	}

	LogRecord LogRecord::deserialize(const std::vector<char>& bytes){
		std::size_t offset = 0;
		LogRecord record;
		record.lsn = read<lsn_t>(bytes, offset);
		record.prev_lsn = read<lsn_t>(bytes, offset);
		record.txn_id = read<transaction_id_t>(bytes, offset);
		record.r_type = static_cast<Type>(read<std::uint32_t>(bytes, offset));
		record.pid = read<page_id_t>(bytes, offset);
		record.slot_id = read<std::uint32_t>(bytes, offset);
		record.b_len = read<std::size_t>(bytes, offset);
		record.before = read_bytes(bytes, offset, record.b_len);
		record.a_len = read<std::size_t>(bytes, offset);
		record.after = read_bytes(bytes, offset, record.a_len);
		if(offset != bytes.size()){
			throw std::runtime_error("RECOVERY Error: Invalid log record");
		}
		return record;
	}
}
