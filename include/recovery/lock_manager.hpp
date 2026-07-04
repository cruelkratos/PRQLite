#pragma once

#include "include/globals.hpp"
#include "include/virtual_machine/memory_manager.hpp"

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace db::recovery{
	class LockManager{
	public:
		void lockShared(transaction_id_t txn_id, const db::memory::RecordID& rid);
		void lockExclusive(transaction_id_t txn_id, const db::memory::RecordID& rid);
		void unlock(transaction_id_t txn_id, const db::memory::RecordID& rid);
		void unlockAll(transaction_id_t txn_id);

	private:
		struct LockState{
			std::unordered_set<transaction_id_t> shared_owners;
			transaction_id_t exclusive_owner{0};
		};

		static std::uint64_t key(const db::memory::RecordID& rid);

		std::mutex mutex_;
		std::condition_variable cv_;
		std::unordered_map<std::uint64_t, LockState> locks_;
	};
}
