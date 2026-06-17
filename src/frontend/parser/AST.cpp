#include<include/frontend/parser/AST.hpp>
#include<frontend/semantic_analyzer.hpp>
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
		tableName   = name;
		// tableSchema = db::semantic::Catalog::getInstance().createTable(name,c);
		tableSchema = std::make_shared<db::table::TableSchema>(1,name, c);
	}

		void BinaryExpr::accept(db::parser::ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	void LiteralExpr::accept(db::parser::ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	void IdentifierExpr::accept(db::parser::ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	void SelectStatement::accept(db::parser::ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	void InsertStatement::accept(db::parser::ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	void DeleteStatement::accept(db::parser::ASTVisitor& visitor) {
		visitor.visit(*this);
	}

	void CreateStatement::accept(db::parser::ASTVisitor& visitor) {
		visitor.visit(*this);
	}


}