#include<include/backend/table_manager.hpp>
#include<stdexcept>

namespace db::memory{

	TableManager::TableManager(){
		auto page_num = global::page_count.fetch_add(1);
		this->_pageList.push_back(std::make_unique<Page>(page_num));
		this->pageNos.fetch_add(1);
	}

	TableIterator TableManager::begin(){
		if(pageNos.load()==0){
			throw std::runtime_error("MEMORY ERROR: No Pages exist");
		}
		return TableIterator(*this,0,0);
		//after last
	}

	TableIterator TableManager::end(){
		if(pageNos.load()==0){
			throw std::runtime_error("MEMORY ERROR: No Pages exist");
		}
		return TableIterator(*this,pageNos.load(),0);
		//after last
	}


	void TableManager::createTuple(Tuple& t){

		if (pageNos.load()==0) {
            throw std::runtime_error("STORAGE ERROR: Table has no pages.");
        }

		auto& page = this->_pageList.back();
		if (page->insertTuple(t)) {
            t.rid.page_id = pageNos.load() - 1;	
			return;
        }

		_pageList.push_back(std::make_unique<Page>(pageNos.load()));
		pageNos.fetch_add(1);
		auto& newPage = this->_pageList.back();

		if(newPage->insertTuple(t)){
			t.rid.page_id = pageNos.load() - 1;
			return ;
		}
		throw std::runtime_error("DB Error: Can't insert tuple!");
	}
	
	void TableIterator::advanceToNext(){
		while(currentPageIdx < manager.pageNos.load()){
			auto& page = manager._pageList[currentPageIdx];
			while(currentSlotId < page->slotCount){
				if(page->isSlotValid(currentSlotId)){
					return ;
				}
				currentSlotId++;
			}
			currentPageIdx++;
			currentSlotId = 0;	
		}
	}
	bool TableIterator::hasNext() const{
		return currentPageIdx < manager.pageNos.load();
	}

	Tuple TableIterator::nextTuple() {
		if(!hasNext()){
			throw std::runtime_error("STORAGE ERROR: No more tuples to read.");
		}

		auto& page = manager._pageList[currentPageIdx];
		Tuple t = page->getTuple(currentSlotId);

		currentSlotId++;
		advanceToNext();

		return t;
	}

	
}