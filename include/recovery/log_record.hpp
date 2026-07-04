#pragma once

#include "include/globals.hpp"
#include "include/virtual_machine/memory_manager.hpp"

#include <cstddef>
#include <vector>


namespace db::recovery{
	struct LogRecord{
		enum class Type{
			INSERT = 0,
			DELETE_RECORD = 1,
			BEGIN = 2,
			COMMIT = 3,
			ROLLBACK = 4
		};
		lsn_t lsn{0};
		lsn_t prev_lsn{0};
		transaction_id_t txn_id{0};
		LogRecord::Type r_type{LogRecord::Type::BEGIN};

		//payload
		page_id_t pid{0};
		std::uint32_t slot_id{0};

		std::size_t b_len{0};
		std::vector<char> before;
		std::size_t a_len{0};
		std::vector<char> after;

		void set_before(const db::memory::Tuple& tuple);
		void set_after(const db::memory::Tuple& tuple);
		std::vector<char> serialize() const;
		static LogRecord deserialize(const std::vector<char>& bytes);
	};
}
