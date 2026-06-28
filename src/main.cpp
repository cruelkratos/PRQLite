#include "include/main.hpp"
#include "include/frontend/parser/parser.hpp"

#include <iostream>
#include <string>
namespace db {

	DB::DB(){
		this->_parser = std::make_unique<db::parser::Parser>();
		this->_semantic_analyzer = std::make_unique<db::semantic::SemanticAnalyzer>();
		this->_executor = std::make_unique<db::executor::ExecutorEngine>(&DB::interrupt);
		// this->_lexer = std::make_unique<db::lexer::Lexer>
	}

	int DB::REPL() {
    std::string line;
    std::string accumulated;

	
	
    while (!interrupt) {
		std::cout << (accumulated.empty() ? "db> " : "... ");

		if(!std::getline(std::cin, line)){
			std::cin.clear();
			break;
		}

        if (line == "exit")
            break;

        // check if this line contains the terminator	
        bool terminated = false;
        auto pos = line.find(';');
        if (pos != std::string::npos) {
            line = line.substr(0, pos); // strip everything from ';' onward
            terminated = true;
        }

        accumulated += (accumulated.empty() ? "" : " ") + line;

        if (terminated) {
            try {
                _parser->insert(accumulated);
                _parser->parse_();
                db::parser::ASTNode* tree = _parser->getTree();
                // TODO: pass to executor
				if(tree == nullptr){
					std::cout<<"bruh"<<std::endl;
				}
				_semantic_analyzer->analyze(tree);
				_executor->execute(tree);

            } catch (const std::runtime_error &e) {
                std::cerr << "Error: " << e.what() << "\n";
				_parser->reset();
            }
			catch(const std::exception &e){
				std::cerr << "Error: " << e.what() << "\n";
				_parser->reset();
			}
			catch(...){
				std::cerr<<"Unknown Error Caught"<<"\n";
				_parser->reset();
			}
            _parser->reset();
            accumulated.clear();
            // std::cout << "db> ";
        } else {
            // std::cout << "... ";
        }
    }

    return 0;
}

	DB::~DB(){}

	int DB::connect(){
		std::signal(SIGINT,DB::handleSignal);
		auto success =  REPL();
		std::cout<<"\nClosing DB Connection..."<<std::endl;
		db::semantic::Catalog::getInstance().flush();
		db::storage::BufferPoolManager::getInstance().flushPagestoDisk();
		return success;
	}

	void DB::handleSignal(int){
		DB::interrupt = 1;
	}
}

int main(int argc, char** argv) {
    db::DB app;
    return app.connect();
}