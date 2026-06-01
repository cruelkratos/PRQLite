// tests/test_parser_delete.cpp

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

void test_delete_all() {
    Parser p;
    p.insert("DELETE FROM logs");
    p.parse_();

    auto* node = dynamic_cast<DeleteStatement*>(p.getTree());
    TEST("delete all: node is DeleteStatement", node != nullptr);
    TEST("delete all: tableName is logs",       node->tableName == "logs");
    TEST("delete all: whereClause is nullptr",  node->whereClause == nullptr);
}

void test_delete_where() {
    Parser p;
    p.insert("DELETE FROM users WHERE id = 10");
    p.parse_();

    auto* node = dynamic_cast<DeleteStatement*>(p.getTree());
    TEST("delete where: node exists",          node != nullptr);
    TEST("delete where: tableName is users",   node->tableName == "users");
    TEST("delete where: has where clause",     node->whereClause != nullptr);

    // Verify expression tree tracking through base ASTNode left/right properties
    auto* expr = dynamic_cast<BinaryExpr*>(node->whereClause);
    TEST("delete where: root is BinaryExpr",   expr != nullptr);
    TEST("delete where: operator is =",        expr->op == "=");

    auto* lhs = dynamic_cast<IdentifierExpr*>(expr->left);
    TEST("delete where: lhs is identifier",    lhs != nullptr);
    TEST("delete where: lhs matches id",       lhs->name == "id");

    auto* rhs = dynamic_cast<LiteralExpr*>(expr->right);
    TEST("delete where: rhs is literal",       rhs != nullptr);
    TEST("delete where: rhs matches 10",       rhs->value.lexeme == "10");
}

void test_delete_errors() {
    TEST_THROWS("error delete: missing FROM keyword", {
        Parser p;
        p.insert("DELETE users");
        p.parse_();
    });

    TEST_THROWS("error delete: missing identifier", {
        Parser p;
        p.insert("DELETE FROM");
        p.parse_();
    });

    TEST_THROWS("error delete: missing expression after WHERE", {
        Parser p;
        p.insert("DELETE FROM users WHERE");
        p.parse_();
    });
}

int main() {
    test_delete_all();
    test_delete_where();
    test_delete_errors();
    return 0;
}