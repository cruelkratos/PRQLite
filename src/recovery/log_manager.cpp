#include "include/recovery/log_manager.hpp"

#include <algorithm>
#include <filesystem>
#include <limits>
#include <stdexcept>

namespace db::recovery{
	std::uint64_t LockManager::key(const db::memory::RecordID& rid){
		return (static_cast<std::uint64_t>(rid.page_id) << 32) | rid.slot_id;
	}

	void LockManager::lockShared(transaction_id_t txn_id, const db::memory::RecordID& rid){
		std::unique_lock<std::mutex> lock(mutex_);
		auto lock_key = key(rid);
		cv_.wait(lock, [&]{
			const auto& state = locks_[lock_key];
			return state.exclusive_owner == 0 || state.exclusive_owner == txn_id;
		});
		locks_[lock_key].shared_owners.insert(txn_id);
	}

	void LockManager::lockExclusive(transaction_id_t txn_id, const db::memory::RecordID& rid){
		std::unique_lock<std::mutex> lock(mutex_);
		auto lock_key = key(rid);
		cv_.wait(lock, [&]{
			const auto& state = locks_[lock_key];
			const bool no_other_shared = state.shared_owners.empty() ||
				(state.shared_owners.size() == 1 && state.shared_owners.count(txn_id) == 1);
			return no_other_shared && (state.exclusive_owner == 0 || state.exclusive_owner == txn_id);
		});
		auto& state = locks_[lock_key];
		state.shared_owners.erase(txn_id);
		state.exclusive_owner = txn_id;
	}

	void LockManager::unlock(transaction_id_t txn_id, const db::memory::RecordID& rid){
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = locks_.find(key(rid));
		if(it == locks_.end()){
			return;
		}
		it->second.shared_owners.erase(txn_id);
		if(it->second.exclusive_owner == txn_id){
			it->second.exclusive_owner = 0;
		}
		if(it->second.shared_owners.empty() && it->second.exclusive_owner == 0){
			locks_.erase(it);
		}
		cv_.notify_all();
	}

	void LockManager::unlockAll(transaction_id_t txn_id){
		std::lock_guard<std::mutex> lock(mutex_);
		for(auto it = locks_.begin(); it != locks_.end();){
			it->second.shared_owners.erase(txn_id);
			if(it->second.exclusive_owner == txn_id){
				it->second.exclusive_owner = 0;
			}
			if(it->second.shared_owners.empty() && it->second.exclusive_owner == 0){
				it = locks_.erase(it);
			}
			else{
				++it;
			}
		}
		cv_.notify_all();
	}

	WriteAheadLog& WriteAheadLog::getInstance(){
		static WriteAheadLog instance;
		return instance;
	}

	WriteAheadLog::WriteAheadLog(){
		loadNextLsn();
		out_.open(logPath(), std::ios::binary | std::ios::app);
		if(!out_.is_open()){
			throw std::runtime_error("RECOVERY Error: Can't open write-ahead log.");
		}
	}

	WriteAheadLog::~WriteAheadLog(){
		flush();
	}

	std::string WriteAheadLog::logPath() const{
		std::filesystem::path db_dir = std::filesystem::path(PROJECT_ROOT) / "data";
		std::filesystem::create_directories(db_dir);
		return (db_dir / ".wal.db").string();
	}

	void WriteAheadLog::loadNextLsn(){
		std::ifstream in(logPath(), std::ios::binary);
		if(!in.is_open()){
			next_lsn_.store(1);
			return;
		}

		lsn_t max_lsn = 0;
		while(true){
			std::uint32_t len = 0;
			in.read(reinterpret_cast<char*>(&len), sizeof(len));
			if(!in){
				break;
			}
			std::vector<char> bytes(len);
			in.read(bytes.data(), len);
			if(!in){
				break;
			}
			max_lsn = std::max(max_lsn, LogRecord::deserialize(bytes).lsn);
		}
		next_lsn_.store(max_lsn + 1);
	}

	lsn_t WriteAheadLog::nextLsn(){
		return next_lsn_++;
	}

	lsn_t WriteAheadLog::previousLsn(transaction_id_t transactionId) const{
		auto it = txn_last_lsn_.find(transactionId);
		return it == txn_last_lsn_.end() ? 0 : it->second;
	}

	void WriteAheadLog::rememberLsn(transaction_id_t transactionId, lsn_t lsn){
		txn_last_lsn_[transactionId] = lsn;
	}

	lsn_t WriteAheadLog::append(LogRecord record){
		std::lock_guard<std::mutex> lock(mutex_);
		record.lsn = nextLsn();
		record.prev_lsn = previousLsn(record.txn_id);

		auto bytes = record.serialize();
		if(bytes.size() > std::numeric_limits<std::uint32_t>::max()){
			throw std::runtime_error("RECOVERY Error: Log record is too large.");
		}
		const auto len = static_cast<std::uint32_t>(bytes.size());
		out_.write(reinterpret_cast<const char*>(&len), sizeof(len));
		out_.write(bytes.data(), bytes.size());
		if(!out_){
			throw std::runtime_error("RECOVERY Error: Failed to append log record.");
		}
		rememberLsn(record.txn_id, record.lsn);
		return record.lsn;
	}

	lsn_t WriteAheadLog::appendBegin(transaction_id_t transactionId){
		LogRecord record;
		record.txn_id = transactionId;
		record.r_type = LogRecord::Type::BEGIN;
		return append(record);
	}

	lsn_t WriteAheadLog::appendCommit(transaction_id_t transactionId){
		LogRecord record;
		record.txn_id = transactionId;
		record.r_type = LogRecord::Type::COMMIT;
		const auto lsn = append(record);
		{
			std::lock_guard<std::mutex> lock(mutex_);
			txn_last_lsn_.erase(transactionId);
			_lock_manager.unlockAll(transactionId);
		}
		flush();
		return lsn;
	}

	lsn_t WriteAheadLog::appendRollback(transaction_id_t transactionId){
		LogRecord record;
		record.txn_id = transactionId;
		record.r_type = LogRecord::Type::ROLLBACK;
		const auto lsn = append(record);
		{
			std::lock_guard<std::mutex> lock(mutex_);
			txn_last_lsn_.erase(transactionId);
			_lock_manager.unlockAll(transactionId);
		}
		flush();
		return lsn;
	}

	lsn_t WriteAheadLog::appendInsert(transaction_id_t transactionId, const db::memory::Tuple& after){
		LogRecord record;
		record.txn_id = transactionId;
		record.r_type = LogRecord::Type::INSERT;
		record.pid = after.rid.page_id;
		record.slot_id = after.rid.slot_id;
		record.set_after(after);
		return append(record);
	}

	lsn_t WriteAheadLog::appendDelete(transaction_id_t transactionId, const db::memory::Tuple& before){
		LogRecord record;
		record.txn_id = transactionId;
		record.r_type = LogRecord::Type::DELETE_RECORD;
		record.pid = before.rid.page_id;
		record.slot_id = before.rid.slot_id;
		record.set_before(before);
		return append(record);
	}

	void WriteAheadLog::flush(){
		std::lock_guard<std::mutex> lock(mutex_);
		if(out_.is_open()){
			out_.flush();
		}
	}

	LockManager& WriteAheadLog::lockManager(){
		return _lock_manager;
	}
}
