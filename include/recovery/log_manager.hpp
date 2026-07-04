#pragma once
#include <cstdint>


namespace db::recovery{
	using transaction_id_t = std::uint32_t;
	class WriteAheadLog{
	public:
		virtual ~WriteAheadLog() = default;
		virtual void appendBegin(transaction_id_t transactionId) = 0;
		virtual void appendCommit(transaction_id_t transactionId) = 0;
		virtual void appendRollback(transaction_id_t transactionId) = 0;
		virtual void flush() = 0;
	};
}