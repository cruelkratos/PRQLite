#include "include/frontend/lexer.hpp"
#include <string>
#include <cctype>
#include <iostream>
#include <stdexcept>

namespace db::lexer{

	Lexer::Lexer(std::string inputStream) 
    : _pos(0), 
      _inputStream(std::move(inputStream)),
      _tokenTable({
		//DDL
		  {"CREATE", TokenType::CREATE}, {"create", TokenType::CREATE},
		  {"TABLE", TokenType::TABLE}, {"table", TokenType::TABLE},
		  {"INDEX", TokenType::INDEX}, {"index", TokenType::INDEX},
          // DML
          {"SELECT", TokenType::SELECT}, {"select", TokenType::SELECT},
          {"INSERT", TokenType::INSERT}, {"insert", TokenType::INSERT},
          {"UPDATE", TokenType::UPDATE}, {"update", TokenType::UPDATE},
          {"DELETE", TokenType::DELETE}, {"delete", TokenType::DELETE},

          // Clauses
          {"FROM",   TokenType::FROM},   {"from",   TokenType::FROM},
          {"WHERE",  TokenType::WHERE},  {"where",  TokenType::WHERE},
          {"GROUP",  TokenType::GROUP},  {"group",  TokenType::GROUP},
          {"BY",     TokenType::BY},     {"by",     TokenType::BY},
          {"ORDER",  TokenType::ORDER},  {"order",  TokenType::ORDER},
          {"HAVING", TokenType::HAVING}, {"having", TokenType::HAVING},
          {"LIMIT",  TokenType::LIMIT},  {"limit",  TokenType::LIMIT},
		  {"ON",  TokenType::ON},  {"on",  TokenType::ON},

          // Logical and Misc
          {"AND",    TokenType::AND},    {"and",    TokenType::AND},
          {"OR",     TokenType::OR},     {"or",     TokenType::OR},
          {"NOT",    TokenType::NOT},    {"not",    TokenType::NOT},
		  {"ASC",  TokenType::ASC},  {"asc",  TokenType::ASC},
		  {"DESC", TokenType::DESC}, {"desc", TokenType::DESC},
		  {"TRUE", TokenType::TRUE} , {"true", TokenType::TRUE},
		  {"FALSE", TokenType::FALSE} , {"false", TokenType::FALSE},
		  {"INTO", TokenType::INTO}, {"into", TokenType::INTO},
		  {"VALUES", TokenType::VALUES}, {"values", TokenType::VALUES},
		  {"INT", TokenType::INT}, {"int", TokenType::INT},
		  {"TEXT", TokenType::TEXT}, {"text", TokenType::TEXT},
		  {"BOOL", TokenType::BOOL}, {"bool", TokenType::BOOL},
      }) 
	{
		if(_inputStream.size() == 0){
			throw std::runtime_error("LEXER_ERROR: input stream is empty");	
		}
	}
	void Lexer::skip_whitespace(){
		while(_pos < _inputStream.size() && std::isspace(static_cast<unsigned char>(_inputStream[_pos]))){
			++_pos;
		}
	}

	Token Lexer::next_token(){
		skip_whitespace();
		if (_pos >= _inputStream.size()) {
        return {"",TokenType::EOF_TOKEN};
    	}
		Token t;
		char c = _inputStream[_pos];

		// Punctuation and operators (handle two-char operators)
		switch(c){
			case ',': ++_pos; return {",", TokenType::COMMA};
			case ';': ++_pos; return {";", TokenType::SEMICOLON};
			case '*': ++_pos; return {"*", TokenType::STAR};
			case '(' : ++_pos; return {"(", TokenType::LPAREN};
			case ')' : ++_pos; return {")", TokenType::RPAREN};
			case '=' : ++_pos; return {"=", TokenType::EQUAL};
			case '<': {
				// <, <=, <>
				if(_pos+1 < _inputStream.size()){
					char n = _inputStream[_pos+1];
					if(n == '=') { _pos += 2; return {"<=", TokenType::LESS_EQUAL}; }
					if(n == '>') { _pos += 2; return {"<>", TokenType::NOT_EQUAL}; }
				}
				++_pos; return {"<", TokenType::LESS};
			}
			case '>': {
				// >, >=
				if(_pos+1 < _inputStream.size() && _inputStream[_pos+1] == '='){
					_pos += 2; return {">=", TokenType::GREATER_EQUAL};
				}
				++_pos; return {">", TokenType::GREATER};
			}
			case '!': {
				// != only
				if(_pos+1 < _inputStream.size() && _inputStream[_pos+1] == '='){
					_pos += 2; return {"!=", TokenType::NOT_EQUAL};
				}
				throw std::runtime_error("LEXER_ERROR: unexpected '!' without '=' at position " + std::to_string(_pos));
			}
			case '\'': {
				// single-quoted string
				char quote = '\'';
				++_pos; // consume opening quote
				std::string val;
				while(_pos < _inputStream.size() && _inputStream[_pos] != quote){
					// simple escape handling for \\ and \'
					if(_inputStream[_pos] == '\\' && _pos+1 < _inputStream.size()){
						val.push_back(_inputStream[_pos+1]);
						_pos += 2;
						continue;
					}
					val.push_back(_inputStream[_pos]);
					++_pos;
				}
				if(_pos >= _inputStream.size()) throw std::runtime_error("LEXER_ERROR: unterminated string literal");
				++_pos; // consume closing quote
				return {val, TokenType::STRING};
			}
			case '"': {
				// double-quoted string
				char quote = '"';
				++_pos;
				std::string val;
				while(_pos < _inputStream.size() && _inputStream[_pos] != quote){
					if(_inputStream[_pos] == '\\' && _pos+1 < _inputStream.size()){
						val.push_back(_inputStream[_pos+1]);
						_pos += 2;
						continue;
					}
					val.push_back(_inputStream[_pos]);
					++_pos;
				}
				if(_pos >= _inputStream.size()) throw std::runtime_error("LEXER_ERROR: unterminated string literal");
				++_pos;
				return {val, TokenType::STRING};
			}
		}

		// Identifiers or keywords: starts with letter or underscore
		if(std::isalpha(static_cast<unsigned char>(c)) || c == '_'){
			std::string ident;
			while(_pos < _inputStream.size() && (std::isalnum(static_cast<unsigned char>(_inputStream[_pos])) || _inputStream[_pos] == '_')){
				ident.push_back(_inputStream[_pos]);
				++_pos;
			}
			// check token table (case-sensitive entries for both cases may exist)
			if(_tokenTable.find(ident) != _tokenTable.end()){
				return {ident, _tokenTable[ident]};
			}
			return {ident, TokenType::IDENTIFIER};
		}

		// Numbers
		if(std::isdigit(static_cast<unsigned char>(c))){
			std::string num;
			while(_pos < _inputStream.size() && std::isdigit(static_cast<unsigned char>(_inputStream[_pos]))){
				num.push_back(_inputStream[_pos]);
				++_pos;
			}
			// optional fractional part
			if(_pos < _inputStream.size() && _inputStream[_pos] == '.'){
				num.push_back('.');
				++_pos;
				if(_pos >= _inputStream.size() || !std::isdigit(static_cast<unsigned char>(_inputStream[_pos])))
					throw std::runtime_error("LEXER_ERROR: malformed number literal");
				while(_pos < _inputStream.size() && std::isdigit(static_cast<unsigned char>(_inputStream[_pos]))){
					num.push_back(_inputStream[_pos]);
					++_pos;
				}
			}
			return {num, TokenType::NUMBER};
		}

		// If we reach here, character is not recognized
		throw std::runtime_error(std::string("LEXER_ERROR: unexpected character '") + c + "' at position " + std::to_string(_pos));
	}

	Lexer::~Lexer(){
		_inputStream.clear();
		_tokenTable.clear();
	}


}


// int main(){
// 	db::lexer::Lexer l = db::lexer::Lexer("select * from employees where name = \"garv\" ;");

// 	auto a = l.next_token();
// 	while(a.type != db::lexer::TokenType::EOF_TOKEN) {
// 		std::cout<<a.lexeme<<" "<<static_cast<int>(a.type)<<std::endl;
// 		a = l.next_token();
// 	}
// 	std::cout<<a.lexeme<<" "<<static_cast<int>(a.type)<<std::endl;
// }