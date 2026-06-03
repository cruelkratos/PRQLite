#include<include/frontend/parser/parser.hpp>

namespace db::parser{

	Parser::Parser() : token_pos(0), statementTree(nullptr){}
	ASTNode::ASTNode() {}

	ASTNode* Parser::getTree(){
		return statementTree;
	}

	void Parser::reset(){
		statementTree = nullptr;
		token_pos = 0;
		tokenStream.clear();
	}
	
	db::lexer::Token Parser::peak() {
		if(tokenStream.size()<1){
			throw std::runtime_error("Token Stream is Empty");
		}
		if(static_cast<size_t>(token_pos) >= tokenStream.size()){
			throw std::runtime_error("At End of token stream");
		}
		return tokenStream[token_pos];
	}

	bool Parser::isAtEnd() {
    return static_cast<size_t>(token_pos) >= tokenStream.size();
	}

	void Parser::advance(){
		++token_pos;
	}

	bool Parser::match(const db::lexer::TokenType t){
		if(isAtEnd()) return false;
		const auto& current = peak();
		if (!isAtEnd() && current.type == t) {
			this->advance();
			return true;
		} 
		return false;
	}
	bool Parser::expect(const db::lexer::Token &t){
		const auto& current = peak();
		if (current.lexeme == t.lexeme && current.type == t.type) {
			this->advance();
			return true;
		} 
		throw std::runtime_error("SYNTAX ERROR: expected " + t.lexeme);
	}
	
	std::string Parser::parseIdentifier() {
		if (isAtEnd() || peak().type != db::lexer::TokenType::IDENTIFIER)
			throw std::runtime_error("SYNTAX ERROR: expected identifier, got '" + peak().lexeme + "'");
		std::string name = peak().lexeme;
		advance();
		return name;
	}

	// column_def → identifier type
	db::table::Column Parser::parseColumnDef() {
		std::string colName = parseIdentifier();

		if (isAtEnd())
			throw std::runtime_error("SYNTAX ERROR: expected type after column name '" + colName + "'");

		db::lexer::TokenType colType = peak().type;

		if (colType != db::lexer::TokenType::INT  &&
			colType != db::lexer::TokenType::TEXT &&
			colType != db::lexer::TokenType::BOOL)
			throw std::runtime_error("SYNTAX ERROR: expected INT, TEXT, or BOOL for column '" + colName + "'");

		advance(); 

		db::table::Column col;
		col.colName = colName;
		col.type    = db::table::tokenToColumnType(colType);
		return col;
	}

	std::vector<db::lexer::Token> Parser::parseValues(){
		std::vector<db::lexer::Token> values;
		if (isAtEnd())
			throw std::runtime_error("SYNTAX ERROR: expected value, got end of input");
		auto isLiteral = [](db::lexer::TokenType t) {
        return t == db::lexer::TokenType::NUMBER ||
               t == db::lexer::TokenType::STRING ||
               t == db::lexer::TokenType::TRUE   ||
               t == db::lexer::TokenType::FALSE;
    	};
		auto temp = peak();
		if(!isLiteral(temp.type))
			throw std::runtime_error("SYNTAX ERROR: expected literal got: " + temp.lexeme);
		values.emplace_back(temp);
		advance();

		while(match(db::lexer::TokenType::COMMA)){
			// advance();
			auto temp = peak();
			if(isAtEnd() || !isLiteral(temp.type))
				throw std::runtime_error("SYNTAX Error: expected literal after ,");
			values.emplace_back(temp);
			advance();
		}

		return values;
	}

	std::vector<db::table::Column> Parser::parseColumnDefs() {
    std::vector<db::table::Column> columns;

    columns.push_back(parseColumnDef());

    while (match(db::lexer::TokenType::COMMA)) {
        // advance(); // consume ','
        columns.push_back(parseColumnDef());
    }

    return columns;
}
};