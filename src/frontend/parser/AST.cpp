#include<include/frontend/parser/AST.hpp>

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
		tableSchema = std::make_shared<db::table::TableSchema>(name,c);
	}


}