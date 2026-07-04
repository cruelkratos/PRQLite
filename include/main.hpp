#pragma once 
#include "include/backend/buffer_pool.hpp"
#include <csignal>

namespace db {

class DB{
public:
	
	DB();
	int connect();
	~DB();

private:
	inline static volatile sig_atomic_t interrupt{0};

	static void handleSignal(int);
	int REPL();

};

}
