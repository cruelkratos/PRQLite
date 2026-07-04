#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace db::transaction{

	using transaction_id_t = std::uint64_t;

	enum class TransactionState{
		Idle,
		Active
	};

	struct TransactionContext{
		transaction_id_t transactionId{0};
		TransactionState state{TransactionState::Idle};
	};

	class WriteAheadLog{
	public:
		virtual ~WriteAheadLog() = default;
		virtual void appendBegin(transaction_id_t transactionId) = 0;
		virtual void appendCommit(transaction_id_t transactionId) = 0;
		virtual void appendRollback(transaction_id_t transactionId) = 0;
		virtual void flush() = 0;
	};

	struct TransactionResult{
		transaction_id_t transactionId{0};
		TransactionState state{TransactionState::Idle};
		std::string message;
	};

}
