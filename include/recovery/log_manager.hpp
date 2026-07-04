#pragma once
#include "include/recovery/lock_manager.hpp"
#include "include/recovery/log_record.hpp"
#include "include/virtual_machine/memory_manager.hpp"

#include <atomic>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>


namespace db::recovery{
	class WriteAheadLog{
	public:
		WriteAheadLog(const WriteAheadLog&) = delete;
		WriteAheadLog& operator=(const WriteAheadLog&) = delete;

		static WriteAheadLog& getInstance();

		~WriteAheadLog();
		lsn_t append(LogRecord record);
		lsn_t appendBegin(transaction_id_t transactionId);
		lsn_t appendCommit(transaction_id_t transactionId);
		lsn_t appendRollback(transaction_id_t transactionId);
		lsn_t appendInsert(transaction_id_t transactionId, const db::memory::Tuple& after);
		lsn_t appendDelete(transaction_id_t transactionId, const db::memory::Tuple& before);
		void flush();
		LockManager& lockManager();

	private:
		WriteAheadLog();
		lsn_t nextLsn();
		lsn_t previousLsn(transaction_id_t transactionId) const;
		void rememberLsn(transaction_id_t transactionId, lsn_t lsn);
		std::string logPath() const;
		void loadNextLsn();

		mutable std::mutex mutex_;
		std::ofstream out_;
		std::atomic<lsn_t> next_lsn_{1};
		std::unordered_map<transaction_id_t, lsn_t> txn_last_lsn_;
		LockManager _lock_manager; 
	};
}
