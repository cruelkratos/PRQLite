#include "include/transaction/transaction_manager.hpp"
#include "include/backend/buffer_pool.hpp"
#include "include/virtual_machine/table_manager.hpp"

#include <stdexcept>

namespace db::transaction{

	TransactionManager::TransactionManager()
		: _wal(&db::recovery::WriteAheadLog::getInstance()){}

	TransactionManager& TransactionManager::current(){
		static thread_local TransactionManager manager;
		return manager;
	}

	TransactionResult TransactionManager::begin(){
		if(_context.state != TransactionState::Idle){
			throw std::runtime_error("TRANSACTION ERROR: BEGIN called while a transaction is already active.");
		}

		_context.transactionId = _nextTransactionId++;
		_context.state = TransactionState::Active;
		if(_wal){
			_wal->appendBegin(_context.transactionId);
		}
		return {_context.transactionId, _context.state, "BEGIN"};
	}

	TransactionResult TransactionManager::commit(){
		ensureActive("COMMIT");
		const auto transactionId = _context.transactionId;
		if(_wal){
			_wal->appendCommit(transactionId);
		}
		undo_actions_.clear();
		db::storage::BufferPoolManager::getInstance().markDirtyPagesCommitted(touched_pages_);
		db::storage::BufferPoolManager::getInstance().flushCommittedPagestoDisk();
		touched_pages_.clear();
		_context = {};
		return {transactionId, _context.state, "COMMIT"};
	}

	TransactionResult TransactionManager::rollback(){
		ensureActive("ROLLBACK");
		const auto transactionId = _context.transactionId;
		for(auto it = undo_actions_.rbegin(); it != undo_actions_.rend(); ++it){
			(*it)();
		}
		undo_actions_.clear();
		if(_wal){
			_wal->appendRollback(transactionId);
		}
		db::storage::BufferPoolManager::getInstance().markDirtyPagesCommitted(touched_pages_);
		db::storage::BufferPoolManager::getInstance().flushCommittedPagestoDisk();
		touched_pages_.clear();
		_context = {};
		return {transactionId, _context.state, "ROLLBACK"};
	}

	TransactionResult TransactionManager::apply(Action action){
		switch(action){
			case Action::Begin:
				return begin();
			case Action::Commit:
				return commit();
			case Action::Rollback:
				return rollback();
		}
		throw std::runtime_error("TRANSACTION ERROR: Unknown transaction action.");
	}

	bool TransactionManager::hasActiveTransaction() const{
		return _context.state == TransactionState::Active;
	}

	const TransactionContext& TransactionManager::context() const{
		return _context;
	}

	void TransactionManager::setWriteAheadLog(db::recovery::WriteAheadLog* wal){
		_wal = wal;
	}

	void TransactionManager::recordInsert(std::shared_ptr<db::memory::TableManager> tableManager, const db::memory::Tuple& tuple){
		ensureActive("INSERT");
		if(_wal){
			_wal->appendInsert(_context.transactionId, tuple);
		}
		auto rid = tuple.rid;
		touched_pages_.insert(rid.page_id);
		undo_actions_.push_back([tableManager, rid]{
			tableManager->deleteTuple(rid);
		});
	}

	void TransactionManager::recordDelete(std::shared_ptr<db::memory::TableManager> tableManager, const db::memory::Tuple& tuple){
		ensureActive("DELETE");
		if(_wal){
			_wal->appendDelete(_context.transactionId, tuple);
		}
		touched_pages_.insert(tuple.rid.page_id);
		undo_actions_.push_back([tableManager, tuple]{
			tableManager->restoreTuple(tuple);
		});
	}

	void TransactionManager::ensureActive(const char* action) const{
		if(_context.state != TransactionState::Active){
			throw std::runtime_error(std::string("TRANSACTION ERROR: ") + action + " called without an active transaction.");
		}
	}

}
