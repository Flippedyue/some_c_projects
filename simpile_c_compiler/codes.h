#ifndef codes_hpp
#define codes_hpp

#include "TreeNode.h"
#include "label.h"
#include <vector>

extern ofstream out;

extern vector<int> symtbl;

void gen_header();
void gen_decl(TreeNode *p);
void gen_code(TreeNode *root);
void recursive_gen_code(TreeNode *root);
void stmt_gen_code(TreeNode *t);

void expr_gen_code(TreeNode *t);
void expr_child_gen_code(TreeNode *expr_child);
void base_expr_gen_code(TreeNode *t);
void shift_expr_gen_code(TreeNode *t);
void cmp_expr_gen_code(TreeNode *t);
void print_jmp(int label, TreeNode *t);

string cover_label(int label);
string expr_child_id(TreeNode *expr_child);

#endif