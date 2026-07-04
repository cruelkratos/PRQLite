#pragma once

#include "include/transaction/transaction_context.hpp"

namespace db::transaction{

	class TransactionManager{
	public:
		enum class Action{
			Begin,
			Commit,
			Rollback
		};

		TransactionManager(const TransactionManager&) = delete;
		TransactionManager& operator=(const TransactionManager&) = delete;

		static TransactionManager& current();

		TransactionResult begin();
		TransactionResult commit();
		TransactionResult rollback();
		TransactionResult apply(Action action);
		bool hasActiveTransaction() const;
		const TransactionContext& context() const;
		void setWriteAheadLog(WriteAheadLog* wal);

	private:
		TransactionManager();
		void ensureActive(const char* action) const;

		TransactionContext _context{};
		transaction_id_t _nextTransactionId{1};
		WriteAheadLog* _wal{nullptr};
	};
}
