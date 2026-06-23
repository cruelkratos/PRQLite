#include "include/backend/executor.hpp"
#include <stdexcept>


namespace db::executor{
	void SelectOperator::init(){
		//initialize table iterator;
		it.emplace(table_manager->begin());
	}

	// SelectOperator::~SelectOperator(){
	// 	delete it;
	// }

	std::optional<db::memory::Tuple> SelectOperator::next(){
		if(!it->hasNext()){
			return std::nullopt;
		}
		return it->nextTuple();
	}
}