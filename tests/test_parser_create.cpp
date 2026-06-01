// tests/test_parser_create.cpp

#include <iostream>
#include <cassert>
#include "include/frontend/parser/parser.hpp"
#include "include/frontend/parser/AST.hpp"
#include "include/table.hpp"

using namespace db::parser;

#define TEST(name, expr) \
    do { \
        if (expr) std::cout << "[PASS] " << name << "\n"; \
        else      std::cerr << "[FAIL] " << name << "\n"; \
    } while(0)

#define TEST_THROWS(name, code) \
    do { \
        bool threw = false; \
        try { code } catch (const std::exception&) { threw = true; } \
        TEST(name, threw); \
    } while(0)

void test_create_basic() {
    Parser p;
    p.insert("CREATE TABLE users (id INT, name TEXT, active BOOL)");
    p.parse_();

    auto* node = dynamic_cast<CreateStatement*>(p.getTree());
    TEST("create basic: node is CreateStatement", node != nullptr);
    TEST("create basic: tableName is users",      node->tableName == "users");
    TEST("create basic: tableSchema allocated",   node->tableSchema != nullptr);
    TEST("create basic: has 3 columns",           node->tableSchema->columns.size() == 3);
    
    // Column 1: id INT
    TEST("create basic: col 0 name is id",        node->tableSchema->columns[0].colName == "id");
    TEST("create basic: col 0 type is INT",       node->tableSchema->columns[0].type == db::lexer::TokenType::INT);
    
    // Column 2: name TEXT
    TEST("create basic: col 1 name is name",      node->tableSchema->columns[1].colName == "name");
    TEST("create basic: col 1 type is TEXT",      node->tableSchema->columns[1].type == db::lexer::TokenType::TEXT);
    
    // Column 3: active BOOL
    TEST("create basic: col 2 name is active",    node->tableSchema->columns[2].colName == "active");
    TEST("create basic: col 2 type is BOOL",      node->tableSchema->columns[2].type == db::lexer::TokenType::BOOL);
}

void test_create_single_column() {
    Parser p;
    p.insert("CREATE TABLE data (val INT)");
    p.parse_();

    auto* node = dynamic_cast<CreateStatement*>(p.getTree());
    TEST("create single: node exists",            node != nullptr);
    TEST("create single: tableName is data",      node->tableName == "data");
    TEST("create single: has 1 column",           node->tableSchema->columns.size() == 1);
    TEST("create single: col 0 name is val",      node->tableSchema->columns[0].colName == "val");
}

void test_create_errors() {
    TEST_THROWS("error create: missing TABLE keyword", {
        Parser p;
        p.insert("CREATE users (id INT)");
        p.parse_();
    });

    TEST_THROWS("error create: missing table identifier", {
        Parser p;
        p.insert("CREATE TABLE (id INT)");
        p.parse_();
    });

    TEST_THROWS("error create: empty column definition list", {
        Parser p;
        p.insert("CREATE TABLE users ()");
        p.parse_();
    });

    TEST_THROWS("error create: missing closing parenthesis", {
        Parser p;
        p.insert("CREATE TABLE users (id INT");
        p.parse_();
    });
}

int main() {
    test_create_basic();
    test_create_single_column();
    test_create_errors();
    return 0;
}