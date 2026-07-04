#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace db::transaction{

	using transaction_id_t = std::uint32_t;

	enum class TransactionState{
		Idle,
		Active
	};

	struct TransactionContext{
		transaction_id_t transactionId{0};
		TransactionState state{TransactionState::Idle};
	};

	

	struct TransactionResult{
		transaction_id_t transactionId{0};
		TransactionState state{TransactionState::Idle};
		std::string message;
	};

}
