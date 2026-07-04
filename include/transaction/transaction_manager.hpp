#pragma once

#include "include/transaction/transaction_context.hpp"
#include "include/recovery/log_manager.hpp"

#include <functional>
#include <memory>
#include <unordered_set>
#include <vector>

namespace db::memory{
	class TableManager;
	class Tuple;
}

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
		void setWriteAheadLog(db::recovery::WriteAheadLog* wal);
		void recordInsert(std::shared_ptr<db::memory::TableManager> tableManager, const db::memory::Tuple& tuple);
		void recordDelete(std::shared_ptr<db::memory::TableManager> tableManager, const db::memory::Tuple& tuple);

	private:
		TransactionManager();
		void ensureActive(const char* action) const;

		TransactionContext _context{};
		transaction_id_t _nextTransactionId{1};
		db::recovery::WriteAheadLog* _wal{nullptr};
		std::vector<std::function<void()>> undo_actions_;
		std::unordered_set<page_id_t> touched_pages_;
	};
}
