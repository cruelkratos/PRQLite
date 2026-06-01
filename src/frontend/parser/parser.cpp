#include<include/frontend/parser/parser.hpp>
#include<include/frontend/parser/AST.hpp>
#include<include/frontend/lexer.hpp>
#include<memory>
#include<iostream>
#include<stdexcept>
#include<string>

namespace db::parser{

	Parser::Parser() : token_pos(0), statementTree(nullptr){}

	ASTNode::ASTNode() {}

	void Parser::insert(std::string statement){
		try{
			std::unique_ptr<db::lexer::Lexer> _lexer = std::make_unique<db::lexer::Lexer>(statement);
			db::lexer::Token temp = _lexer.get()->next_token();

			while(temp.type!= db::lexer::TokenType::EOF_TOKEN){
				// std::cout << "TOKEN: '" << temp.lexeme << "' type=" << (int)temp.type << "\n";
				tokenStream.push_back(temp);
				temp = _lexer->next_token();
			}
			
		}
		catch (const std::runtime_error& e) {
        std::cerr << "Runtime error in creating LEXER: " << e.what() << std::endl;
		throw;
    	}

		catch (...) {
        std::cerr << "Unknown exception occurred! when creating LEXER" << std::endl;
		throw;
    	}


		return ;
	}

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

	bool Parser::match(const db::lexer::Token &t){
		const auto& current = peak();
		if (current.lexeme == t.lexeme && current.type == t.type) {
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

	void Parser::parseStatement(){
		// implement according to grammar.
		db::lexer::Token t = peak();
		if(t.type == db::lexer::TokenType::SELECT){
			statementTree = parseSelect();
		}
		else if(t.type == db::lexer::TokenType::INSERT){
			statementTree = parseInsert();
		}

		else if(t.type == db::lexer::TokenType::DELETE){
			statementTree = parseDelete();
		}

		else if(t.type == db::lexer::TokenType::CREATE){
			statementTree = parseCreate();
		}
		else throw std::runtime_error("SYNTAX ERROR: Statement must be CRE/SEL/INS/DEL");
	}

	SelectStatement* Parser::parseSelect(){
		auto* node = new SelectStatement();
		advance();
		if(!isAtEnd() && peak().type == db::lexer::TokenType::STAR){
			node->selectStar = true;
			advance();
		} else{
			node->columns.push_back(parseIdentifier());
			while(!isAtEnd() && peak().type == db::lexer::TokenType::COMMA){
				advance();
				node->columns.push_back(parseIdentifier());
			}
		}

		if (isAtEnd() || peak().type != db::lexer::TokenType::FROM)
    	throw std::runtime_error("SYNTAX ERROR: expected FROM");
		advance();
		node->tableName = parseIdentifier();

		if (!isAtEnd() && peak().type == db::lexer::TokenType::WHERE) {
        advance();
        node->whereClause = parseExpression();
    	}

    
		if (!isAtEnd() && peak().type == db::lexer::TokenType::ORDER) {
			advance();
			if (isAtEnd() || peak().type != db::lexer::TokenType::BY)
    		throw std::runtime_error("SYNTAX ERROR: expected BY after ORDER");
			advance();
			node->orderBy = parseIdentifier();
			if (!isAtEnd() && peak().type == db::lexer::TokenType::ASC) {
				node->orderDir = "ASC"; advance();
			} else if (!isAtEnd() && peak().type == db::lexer::TokenType::DESC) {
				node->orderDir = "DESC"; advance();
			}
		}

    
		if (!isAtEnd() && peak().type == db::lexer::TokenType::LIMIT) {
			advance();
			if (isAtEnd() || peak().type != db::lexer::TokenType::NUMBER)
    		throw std::runtime_error("SYNTAX ERROR: expected number after LIMIT");
			node->limitVal = std::stoi(peak().lexeme);
			advance();
		}

    	return node;
	}

	// expression → or_expr
	ASTNode* Parser::parseExpression() {
    return parseOrExpr();
	}

	// or_expr → and_expr ("OR" and_expr)*
	ASTNode* Parser::parseOrExpr() {
    ASTNode* left = parseAndExpr();
    while (!isAtEnd() && peak().type == db::lexer::TokenType::OR) {
        advance();
        auto* node  = new BinaryExpr();
        node->op    = "OR";
        node->left  = left;
        node->right = parseAndExpr();
        left = node;
    }
    return left;
   }

   // and_expr → equality_expr ("AND" equality_expr)*
   ASTNode* Parser::parseAndExpr() {
    ASTNode* left = parseEqualityExpr();
    while (!isAtEnd() && peak().type == db::lexer::TokenType::AND) {
        advance();
        auto* node  = new BinaryExpr();
        node->op    = "AND";
        node->left  = left;
        node->right = parseEqualityExpr();
        left = node;
    }
    return left;
}

ASTNode* Parser::parseEqualityExpr() {
    ASTNode* left = parseComparisonExpr();
    while (!isAtEnd() && (peak().type == db::lexer::TokenType::EQUAL ||
		peak().type == db::lexer::TokenType::NOT_EQUAL)) {
        std::string op = peak().lexeme;
        advance();
        auto* node  = new BinaryExpr();
        node->op    = op;
        node->left  = left;
        node->right = parseComparisonExpr();
        left = node;
    }
    return left;
}

// comparison_expr → primary ((">" | ">=" | "<" | "<=") primary)*
	ASTNode* Parser::parseComparisonExpr() {
		ASTNode* left = parsePrimary();
		while (!isAtEnd() && (peak().type == db::lexer::TokenType::GREATER       ||
		peak().type == db::lexer::TokenType::GREATER_EQUAL ||
		peak().type == db::lexer::TokenType::LESS          ||
		peak().type == db::lexer::TokenType::LESS_EQUAL))  {
			std::string op = peak().lexeme;
			advance();
			auto* node  = new BinaryExpr();
			node->op    = op;
			node->left  = left;
			node->right = parsePrimary();
			left = node;
		}
		return left;
	}

	// primary → identifier | literal | "(" expression ")"
	ASTNode* Parser::parsePrimary() {
		db::lexer::Token t = peak();

		if (t.type == db::lexer::TokenType::LPAREN) {
			advance();
			ASTNode* inner = parseExpression();
			if (isAtEnd() || peak().type != db::lexer::TokenType::RPAREN)
				throw std::runtime_error("SYNTAX ERROR: expected ')'");
			advance();
			return inner;
		}
		if (t.type == db::lexer::TokenType::NUMBER ||
			t.type == db::lexer::TokenType::STRING ||
			t.type == db::lexer::TokenType::TRUE   ||
			t.type == db::lexer::TokenType::FALSE) {
			auto* node = new LiteralExpr();
			node->value = t;
			advance();
			return node;
		}
		if (t.type == db::lexer::TokenType::IDENTIFIER) {
			auto* node = new IdentifierExpr();
			node->name = t.lexeme;
			advance();
			return node;
		}
		throw std::runtime_error("SYNTAX ERROR: unexpected token '" + t.lexeme + "'");
	}

	
	std::string Parser::parseIdentifier() {
		if (isAtEnd() || peak().type != db::lexer::TokenType::IDENTIFIER)
			throw std::runtime_error("SYNTAX ERROR: expected identifier, got '" + peak().lexeme + "'");
		std::string name = peak().lexeme;
		advance();
		return name;
	}

	InsertStatement* Parser::parseInsert() {
    auto* node = new InsertStatement();
    // TODO: fill node->tableName, node->values by consuming tokens
	advance();
	if(isAtEnd() || peak().type != db::lexer::TokenType::INTO){	
		throw std::runtime_error("SYNTAX ERROR: expected INTO after INSERT.");
	} else advance();
	node->tableName = parseIdentifier();

	if(isAtEnd() || peak().type != db::lexer::TokenType::VALUES){
		throw std::runtime_error("SYNTAX ERROR: expected VALUES after tableName.");
	} else advance();
	

	if (isAtEnd() || peak().type != db::lexer::TokenType::LPAREN)
        throw std::runtime_error("SYNTAX ERROR: expected '(' after VALUES");
    advance(); 

    node->values = parseValues();

    if (isAtEnd() || peak().type != db::lexer::TokenType::RPAREN)
        throw std::runtime_error("SYNTAX ERROR: expected ')' after value list");
    advance(); 


    return node;
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

		while(!isAtEnd() && peak().type == db::lexer::TokenType::COMMA){
			advance();
			auto temp = peak();
			if(isAtEnd() || !isLiteral(temp.type))
				throw std::runtime_error("SYNTAX Error: expected literal after ,");
			values.emplace_back(temp);
			advance();
		}

		return values;
	}

	DeleteStatement* Parser::parseDelete() {
		auto* node = new DeleteStatement();
		// TODO: fill node->tableName and node->whereClause by consuming tokens
		advance();
		if(isAtEnd() || peak().type != db::lexer::TokenType::FROM){	
			throw std::runtime_error("SYNTAX ERROR: expected FROM after DELETE.");
		} else advance();
		node->tableName = parseIdentifier();

		if (!isAtEnd() && peak().type == db::lexer::TokenType::WHERE) {
        advance(); // consume WHERE
        node->whereClause = parseExpression();
    	}
		return node;
	}

	CreateStatement* Parser::parseCreate() {
		// auto* node = new CreateStatement();
		advance();
		if(isAtEnd() || peak().type!= db::lexer::TokenType::TABLE){
			throw std::runtime_error("SYNTAX ERROR: expected table after create.");
		} else advance();

		auto tableName = parseIdentifier();

		if (isAtEnd() || peak().type != db::lexer::TokenType::LPAREN)
        	throw std::runtime_error("SYNTAX ERROR: expected '(' after table name");
   		advance();

		std::vector<db::table::Column> columns = parseColumnDefs();

		if (isAtEnd() || peak().type != db::lexer::TokenType::RPAREN)
        	throw std::runtime_error("SYNTAX ERROR: expected ')' after column definitions");
    	advance();

		auto* node = new CreateStatement(tableName, columns);
		node->tableName = tableName;
		return node;
	}

	std::vector<db::table::Column> Parser::parseColumnDefs() {
    std::vector<db::table::Column> columns;

    columns.push_back(parseColumnDef());

    while (!isAtEnd() && peak().type == db::lexer::TokenType::COMMA) {
        advance(); // consume ','
        columns.push_back(parseColumnDef());
    }

    return columns;
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
		col.type    = colType;
		return col;
	}

}