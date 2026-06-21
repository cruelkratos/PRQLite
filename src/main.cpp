#include<include/main.hpp>
#include<include/frontend/parser/parser.hpp>
#include<iostream>
#include<string>
namespace db {

	DB::DB(){
		this->_parser = std::make_unique<db::parser::Parser>();
		this->_semantic_analyzer = std::make_unique<db::semantic::SemanticAnalyzer>();
		this->_executor = std::make_unique<db::executor::ExecutorEngine>();
		this->_diskmanager = std::make_unique<db::storage::DiskManager>();
		// this->_lexer = std::make_unique<db::lexer::Lexer>
	}

	int DB::REPL() {
    std::string line;
    std::string accumulated;

    std::cout << "db> ";
    while (std::getline(std::cin, line)) {
        if (line == "exit")
            break;

        // check if this line contains the terminator
        bool terminated = false;
        auto pos = line.find(';');
        if (pos != std::string::npos) {
            line = line.substr(0, pos); // strip everything from ';' onward
            terminated = true;
        }

        accumulated += " " + line;

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

            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
				_parser->reset();
            }
            _parser->reset();
            accumulated.clear();
            std::cout << "db> ";
        } else {
            std::cout << "... ";
        }
    }

    return 0;
}

	DB::~DB(){}

}

int main(int argc, char** argv) {
    db::DB app;
    return app.REPL();
}