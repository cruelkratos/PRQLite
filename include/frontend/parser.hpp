#pragma once
#include<memory>
#include<include/frontend/lexer.hpp>

namespace db::parser{
	class Parser{
		public:
		void insert(std::string statement);
		
	};
}