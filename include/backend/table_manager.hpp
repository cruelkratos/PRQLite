#pragma once
#include<include/backend/memory_manager.hpp>
#include<vector>
#include<memory>

namespace db::memory{

	class TableManager;

	class TableIterator{
		private:
		const TableManager& manager;
		size_t currentPageIdx=0;
        std::uint16_t currentSlotId=0;

		public:
		bool hasNext() const;
		Tuple nextTuple();
		void advanceToNext();
		TableIterator(const TableManager& m) : manager(m) {advanceToNext();}
		TableIterator(const TableManager& m , size_t cPageIdx, std::uint16_t curr_slotId) : manager(m) , currentPageIdx(cPageIdx) , currentSlotId(curr_slotId){advanceToNext();}

	};

	class TableManager{
		private:
		friend class TableIterator;
		std::vector<std::unique_ptr<Page>> _pageList;
		
		public:
		TableManager();
		TableIterator begin();
		TableIterator end();
		void createTuple(Tuple &t);

	};

}