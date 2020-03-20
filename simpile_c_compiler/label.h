#ifndef label_hpp
#define label_hpp
#include "TreeNode.h"
extern int tag;

int newLabel();
void recursive_get_label(TreeNode *t, int next_label);
void stmt_get_label(TreeNode *t, int next_label);
void expr_get_label(TreeNode *t);

#endif