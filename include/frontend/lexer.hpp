#pragma once

#include<unordered_map>
#include<string>

namespace db::lexer{

	enum class TokenType {
        // DML
        SELECT, INSERT, UPDATE, DELETE,
        
        // Clauses
        FROM, WHERE, GROUP, BY, ORDER, HAVING, LIMIT,
        
        // Logical
        AND, OR, NOT,
        
        // Literals & Identifiers
        IDENTIFIER, NUMBER, STRING,

        // Punctuation
        COMMA, SEMICOLON, STAR,
        LPAREN, RPAREN,

        // Operators
        EQUAL, 
        LESS, GREATER,
        LESS_EQUAL,    // <=
        GREATER_EQUAL, // >=
        NOT_EQUAL,     // != or <>

        EOF_TOKEN
    };

	struct Token{
		std::string lexeme;
		TokenType type;

	};

	class Lexer{
	private:
		std::unordered_map<std::string,TokenType> _tokenTable;
		size_t _pos; 
		std::string _inputStream;
		void skip_whitespace();
	public:
		Lexer(std::string _inputStream);
		Token next_token();
		~Lexer();
	};
}