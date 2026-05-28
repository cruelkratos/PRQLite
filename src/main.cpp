#include<include/main.hpp>
#include<include/frontend/parser.hpp>
#include<iostream>
#include<string>
namespace db {

	DB::DB(){
		this->_parser = std::make_unique<db::parser::Parser>();
		this->_lexer = std::make_unique<db::lexer::Lexer>();
	}

	int DB::REPL(){
		int exit_flag = 1;
		while(exit_flag){
			std::string statement;
			std::cin>>statement;
			while(statement.size() && statement.back() != ';' && statement!="exit"){
				//temp logic fix with lexer 
				_parser.get()->insert(statement);
				std::cin>>statement;
			}
			if(statement=="exit"){
				exit_flag = 0;
			}
			//execute and print
		}
		return 0;
	}


}