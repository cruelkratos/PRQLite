#include "include/frontend/parser/AST.hpp"
#include "include/frontend/lexer.hpp"
#include "include/frontend/parser/parser.hpp"
#include <memory>
#include <iostream>
#include <stdexcept>
#include <string>

namespace db::parser{

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
			//can be create table or create idx.
			advance();
			db::lexer::Token t2 = peak();
			if(t2.type == db::lexer::TokenType::INDEX){
				statementTree = parseIndex();
				return;
			}
			if(t2.type == db::lexer::TokenType::TABLE){
				statementTree = parseCreate();
				return;
			}
			throw std::runtime_error("SYNTAX ERROR: Invalid CREATE Syntax.");
		}
		else throw std::runtime_error("SYNTAX ERROR: Statement must be CRE/SEL/INS/DEL");
	}

	SelectStatement* Parser::parseSelect(){
		auto* node = new SelectStatement();
		advance();
		if(match(db::lexer::TokenType::STAR)){
			node->selectStar = true;
			// advance();
		} else{
			node->columns.push_back(parseIdentifier());
			while(match(db::lexer::TokenType::COMMA)){
				// advance();
				node->columns.push_back(parseIdentifier());
			}
		}

		if (isAtEnd() || peak().type != db::lexer::TokenType::FROM)
    	throw std::runtime_error("SYNTAX ERROR: expected FROM");
		advance();
		node->tableName = parseIdentifier();

		if (match(db::lexer::TokenType::WHERE)) {
        // advance();
        node->whereClause = parseExpression();
    	}

    
		if (match(db::lexer::TokenType::ORDER)) {
			// advance();
			if (isAtEnd() || peak().type != db::lexer::TokenType::BY)
    		throw std::runtime_error("SYNTAX ERROR: expected BY after ORDER");
			advance();
			node->orderBy = parseIdentifier();
			if (match(db::lexer::TokenType::ASC)) {
				node->orderDir = "ASC"; 
				// advance();
			} else if (match(db::lexer::TokenType::DESC)) {
				node->orderDir = "DESC"; 
				// advance();
			}
		}

    
		if (match(db::lexer::TokenType::LIMIT)) {
			// advance();
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
    while (match(db::lexer::TokenType::OR)) {
        // advance();
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
    while (match(db::lexer::TokenType::AND)) {
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

	DeleteStatement* Parser::parseDelete() {
		auto* node = new DeleteStatement();
		// TODO: fill node->tableName and node->whereClause by consuming tokens
		advance();
		if(isAtEnd() || peak().type != db::lexer::TokenType::FROM){	
			throw std::runtime_error("SYNTAX ERROR: expected FROM after DELETE.");
		} else advance();
		node->tableName = parseIdentifier();

		if (match(db::lexer::TokenType::WHERE)) {
        // advance(); // consume WHERE
        node->whereClause = parseExpression();
    	}
		return node;
	}

	CreateStatement* Parser::parseCreate() {
		// auto* node = new CreateStatement();
		// advance();
		if(isAtEnd() || peak().type!= db::lexer::TokenType::TABLE){
			throw std::runtime_error("SYNTAX ERROR: expected table/index after create.");
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
		return node;
	}

	CreateIdxStatement* Parser::parseIndex(){
		if(isAtEnd() || peak().type!= db::lexer::TokenType::INDEX){
			throw std::runtime_error("SYNTAX ERROR: expected index/table after create.");
		} else advance();

		auto idx_name = parseIdentifier();

		if(isAtEnd() || peak().type!= db::lexer::TokenType::ON){
			throw std::runtime_error("SYNTAX ERROR: expected ON after index name.");
		} else advance();

		auto tableName = parseIdentifier();

		if (isAtEnd() || peak().type != db::lexer::TokenType::LPAREN)
        	throw std::runtime_error("SYNTAX ERROR: expected '(' after table name");
   		advance();

		std::vector<std::string> columns;
		columns.push_back(parseIdentifier());
		while(match(db::lexer::TokenType::COMMA)){
			columns.push_back(parseIdentifier());
		}

		if (isAtEnd() || peak().type != db::lexer::TokenType::RPAREN)
        	throw std::runtime_error("SYNTAX ERROR: expected ')' after column definitions");
    	advance();

		auto* node = new CreateIdxStatement(tableName,idx_name,columns);
		return node;
	}


}