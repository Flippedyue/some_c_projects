//  TreeNode.hpp
//  TreeNode
//
//  Created by 江玥 on 2019/11/15.
//  Copyright © 2019 江玥. All rights reserved.
//

#ifndef TreeNode_hpp
#define TreeNode_hpp

#define MAXCHILDREN 4

#include <iostream>
#include <unordered_map>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <vector>
using namespace std;

enum {StmtK, ExpK, DeclK};
enum {IfK, WhileK, DoWhileK, ForK, CompK, InputK, PrintK};
enum {OpK, ConstK, IdK, TypeK}; // exp
enum {VarK}; // decl
enum {Void, Boolean, Double, Integer, Char};
enum {Plus, Minus, Times, Over, Mod, And, Or, Xor, Shift_left, Shift_right, Pplus, Mminus, Assign,
    Equ, Gtr, Lss, Geq, Leq, Neq, Logical_and, Logical_or, Logical_not, Not};
extern int line;
extern int temp_var_seq;
extern unordered_map<string, int> optMap;
extern unordered_map<string, int> Idmap;
// extern vector<string>str;

struct Label {
    int true_label;
    int false_label;
    int begin_label;
    int end_label;
    int next_label;
};
class TreeNode {
public:
    TreeNode* child[MAXCHILDREN];
    TreeNode* sibling;
    int lineno;
    int nodekind;
    int kind;
    union {
        int op; // 操作符
        int val; // 常量
        char *name;
    }attr;
    struct Label label;
    int temp_var;
    int type; // 用于类型检查：此节点的类
    TreeNode(int nodekind, int kind);
};

TreeNode* newTreeNode(int nodekind, int kind, ...);
TreeNode* newExpNode(int op, TreeNode * operand_1, TreeNode * operand_2);
TreeNode* newVarNode(int kind, char *str);
TreeNode * newDeclNode(TreeNode *type, TreeNode *idlist);
void ConstructMap();
void check_type(TreeNode *p);
void recursive(TreeNode *p);
void get_temp_var(TreeNode *t);
void Display(TreeNode *p);
void ShowNode(TreeNode *p);
#endif /* TreeNode_hpp */
