#include "include/transaction/transaction_manager.hpp"

#include <stdexcept>

namespace db::transaction{

	TransactionManager::TransactionManager() = default;

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
			_wal->flush();
		}
		_context = {};
		return {transactionId, _context.state, "COMMIT"};
	}

	TransactionResult TransactionManager::rollback(){
		ensureActive("ROLLBACK");
		const auto transactionId = _context.transactionId;
		if(_wal){
			_wal->appendRollback(transactionId);
			_wal->flush();
		}
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

	void TransactionManager::ensureActive(const char* action) const{
		if(_context.state != TransactionState::Active){
			throw std::runtime_error(std::string("TRANSACTION ERROR: ") + action + " called without an active transaction.");
		}
	}

}
