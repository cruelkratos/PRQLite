#include "include/virtual_machine/executor.hpp"
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
		if(!it->hasNext() || *interrupt){
			return std::nullopt;
		}
		return it->nextTuple();
	}

	db::table::TableSchema SelectOperator::getOutputSchema() const{
		auto schema = db::semantic::Catalog::getInstance().getTableSchema(this->node->tableId);
		return *schema;
	}
}