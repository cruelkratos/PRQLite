#include "include/main.hpp"
#include "include/frontend/parser/parser.hpp"
#include "include/frontend/semantic_analyzer.hpp"
#include "include/virtual_machine/executor.hpp"

#include <iostream>
#include <string>
namespace db {

	DB::DB() = default;

	int DB::REPL() {
	thread_local db::parser::Parser parser;
	thread_local db::semantic::SemanticAnalyzer semantic_analyzer;
	thread_local db::executor::ExecutorEngine executor(&DB::interrupt);

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
                parser.insert(accumulated);
                parser.parse_();
                db::parser::ASTNode* tree = parser.getTree();
                // TODO: pass to executor
				if(tree == nullptr){
					std::cout<<"bruh"<<std::endl;
				}
				semantic_analyzer.analyze(tree);
				executor.execute(tree);

            } catch (const std::runtime_error &e) {
                std::cerr << "Error: " << e.what() << "\n";
				parser.reset();
            }
			catch(const std::exception &e){
				std::cerr << "Error: " << e.what() << "\n";
				parser.reset();
			}
			catch(...){
				std::cerr<<"Unknown Error Caught"<<"\n";
				parser.reset();
			}
            parser.reset();
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
