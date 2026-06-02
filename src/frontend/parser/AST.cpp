#include<include/frontend/parser/AST.hpp>
#include<include/catalog.hpp>

namespace db::parser{
//implement destructors.


	ASTNode::~ASTNode(){
		delete left;
		delete right;
	}

	DeleteStatement::~DeleteStatement(){
		delete whereClause;
		tableName.clear();
	}

	SelectStatement::~SelectStatement(){
		delete whereClause;
		columns.clear();
	}

	CreateStatement::CreateStatement(std::string name, std::vector<db::table::Column> &c){
		tableSchema = db::semantic::Catalog::getInstance().createTable(name,c);
	}


}