// tests/test_parser_insert.cpp

#include <iostream>
#include <cassert>
#include "include/frontend/parser/parser.hpp"
#include "include/frontend/parser/AST.hpp"

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

void test_insert_basic() {
    Parser p;
    p.insert("INSERT INTO users VALUES (42, \"Alice\", TRUE)");
    p.parse_();

    auto* node = dynamic_cast<InsertStatement*>(p.getTree());
    TEST("insert basic: node is InsertStatement", node != nullptr);
    TEST("insert basic: tableName is users",      node->tableName == "users");
    TEST("insert basic: has 3 values",            node->values.size() == 3);

    // Tokens are pulled directly out of the flat values vector
    TEST("insert basic: val 0 lexeme is 42",      node->values[0].lexeme == "42");
    TEST("insert basic: val 1 lexeme is 'Alice'",  node->values[1].lexeme == "'Alice'");
    TEST("insert basic: val 2 lexeme is TRUE",     node->values[2].lexeme == "TRUE");
}

void test_insert_errors() {
    TEST_THROWS("error insert: missing INTO", {
        Parser p;
        p.insert("INSERT users VALUES (1)");
        p.parse_();
    });

    TEST_THROWS("error insert: missing VALUES keyword", {
        Parser p;
        p.insert("INSERT INTO users (1)");
        p.parse_();
    });

    TEST_THROWS("error insert: empty value list", {
        Parser p;
        p.insert("INSERT INTO users VALUES ()");
        p.parse_();
    });

    TEST_THROWS("error insert: missing matching parenthesis", {
        Parser p;
        p.insert("INSERT INTO users VALUES (1, 2");
        p.parse_();
    });
}

int main() {
    test_insert_basic();
    test_insert_errors();
    return 0;
}