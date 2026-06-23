// tests/test_parser.cpp

//AI WRITTEN TESTS


#include "include/frontend/parser/AST.hpp"
#include "include/frontend/parser/parser.hpp"
#include <iostream>
#include <cassert>

using namespace db::parser;

// helper — prints pass/fail with a label
#define TEST(name, expr) \
    do { \
        if (expr) std::cout << "[PASS] " << name << "\n"; \
        else      std::cerr << "[FAIL] " << name << "\n"; \
    } while(0)

// helper — expects a throw
#define TEST_THROWS(name, code) \
    do { \
        bool threw = false; \
        try { code } catch (const std::exception&) { threw = true; } \
        TEST(name, threw); \
    } while(0)

void test_select_star() {
    Parser p;
    p.insert("SELECT * FROM users");
    p.parse_();  // make this public temporarily, or add a parse() wrapper

    auto* node = dynamic_cast<SelectStatement*>(p.getTree());
    TEST("select star: node is SelectStatement",  node != nullptr);
    TEST("select star: selectStar is true",       node->selectStar == true);
    TEST("select star: tableName is users",       node->tableName == "users");
    TEST("select star: no where clause",          node->whereClause == nullptr);
    TEST("select star: no order by",              node->orderBy.empty());
    TEST("select star: no limit",                 node->limitVal == -1);
}

void test_select_columns() {
    Parser p;
    p.insert("SELECT id, name, email FROM users");
    p.parse_();

    auto* node = dynamic_cast<SelectStatement*>(p.getTree());
    TEST("select cols: node exists",          node != nullptr);
    TEST("select cols: selectStar is false",  node->selectStar == false);
    TEST("select cols: 3 columns",            node->columns.size() == 3);
    TEST("select cols: first col is id",      node->columns[0] == "id");
    TEST("select cols: last col is email",    node->columns[2] == "email");
    TEST("select cols: tableName is users",   node->tableName == "users");
}

void test_select_where() {
    Parser p;
    p.insert("SELECT * FROM orders WHERE status = 1");
    p.parse_();

    auto* node = dynamic_cast<SelectStatement*>(p.getTree());
    TEST("where: node exists",          node != nullptr);
    TEST("where: has where clause",     node->whereClause != nullptr);

    // drill into the expression tree
    auto* expr = dynamic_cast<BinaryExpr*>(node->whereClause);
    TEST("where: root is BinaryExpr",   expr != nullptr);
    TEST("where: op is =",              expr->op == "=");

    auto* lhs = dynamic_cast<IdentifierExpr*>(expr->left);
    TEST("where: lhs is identifier",    lhs != nullptr);
    TEST("where: lhs name is status",   lhs->name == "status");

    auto* rhs = dynamic_cast<LiteralExpr*>(expr->right);
    TEST("where: rhs is literal",       rhs != nullptr);
    TEST("where: rhs value is 1",       rhs->value.lexeme == "1");
}

void test_select_order_limit() {
    Parser p;
    p.insert("SELECT * FROM products ORDER BY price DESC LIMIT 10");
    p.parse_();

    auto* node = dynamic_cast<SelectStatement*>(p.getTree());
    TEST("order+limit: node exists",        node != nullptr);
    TEST("order+limit: orderBy is price",   node->orderBy == "price");
    TEST("order+limit: orderDir is DESC",   node->orderDir == "DESC");
    TEST("order+limit: limitVal is 10",     node->limitVal == 10);
}

void test_select_errors() {
    TEST_THROWS("error: missing FROM", {
        Parser p;
        p.insert("SELECT * users");
        p.parse_();
    });

    TEST_THROWS("error: empty input", {
        Parser p;
        p.insert("");
        p.parse_();
    });

    TEST_THROWS("error: missing table name", {
        Parser p;
        p.insert("SELECT * FROM");
        p.parse_();
    });
}

int main() {
    test_select_star();
    test_select_columns();
    test_select_where();
    test_select_order_limit();
    test_select_errors();
    return 0;
}