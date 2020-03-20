//
//  TreeNode.cpp
//  TreeNode
//
//  Created by 江玥 on 2019/11/15.
//  Copyright © 2019 江玥. All rights reserved.
//

#include "TreeNode.h"
int line = 0;
int temp_var_seq = 0;
const int FOR_CHILD_COUNT = 4;
unordered_map<string, int> optMap;
unordered_map<string, int> Idmap;
// vector<string> str; // str数组在遍历时进行赋值，同时对每个str常量的temp_var也进行赋值

TreeNode::TreeNode(int nodekind, int kind) {
    for (int i = 0; i < MAXCHILDREN; i++) {
        this->child[i] = NULL;
    }
    this->sibling = NULL;
    this->nodekind = nodekind;
    this->kind = kind;
    this->lineno = line++; // 全局变量
    this->temp_var = 0;
    memset(&(this->label), 0, sizeof(this->label)); // 初始化为0
}

TreeNode * newTreeNode(int nodekind, int kind, ...) {
    TreeNode * t = new TreeNode(nodekind, kind);
    va_list args = {}; //定义一个可变参数列表
	va_start(args, kind); // 初始化args指向强制参数arg的下一个参数
    TreeNode *temp = NULL;
    int i = 0;
    if (nodekind == StmtK && kind == ForK)
    {
        for (int i = 0; i < FOR_CHILD_COUNT; i++)
            t->child[i] = va_arg(args, TreeNode *);
    }
    else
    {
        while ((temp = va_arg(args, TreeNode *)) != NULL)
            t->child[i++] = temp;
    }
	va_end(args);//释放args
    return t;
}

void ConstructMap() {
    optMap.insert(make_pair("+", Plus));
    optMap.insert(make_pair("-", Minus));
    optMap.insert(make_pair("*", Times));
    optMap.insert(make_pair("/", Over));
    optMap.insert(make_pair("%", Mod));
    optMap.insert(make_pair("++", Pplus));
    optMap.insert(make_pair("--", Mminus));
    optMap.insert(make_pair("&", And));
    optMap.insert(make_pair("|", Or));
    optMap.insert(make_pair("^", Xor));
    optMap.insert(make_pair("<<", Shift_left));
    optMap.insert(make_pair(">>", Shift_right));
    optMap.insert(make_pair("=", Assign));
    optMap.insert(make_pair("==", Equ));
    optMap.insert(make_pair(">", Gtr));
    optMap.insert(make_pair("<", Lss));
    optMap.insert(make_pair(">=", Geq));
    optMap.insert(make_pair("<=", Leq));
    optMap.insert(make_pair("!=", Neq));
    optMap.insert(make_pair("&&", Logical_and));
    optMap.insert(make_pair("||", Logical_or));
    optMap.insert(make_pair("!", Logical_not));
}

TreeNode * newVarNode(int kind, char *text) {
    TreeNode *t = new TreeNode(ExpK, kind);
    string str = text;
    if (kind == IdK) {
        t->attr.name = text;
        if (Idmap.count(str)) {
            t->type = Idmap.at(str);
        }
    } else if (kind == OpK || kind == TypeK) {
        t->attr.op = str[0];
        if (kind == TypeK) {
            t->type = str[0];
        }
    } else if (kind == ConstK) {
        if (str[0] == '\'')
        {
            t->attr.val = str[1];
            t->type = Char;
        }
        else
        {
            t->attr.val = stoi(str); // 转为int型变量
            t->type = Integer;
        }
    }
    return t;
}

TreeNode * newExpNode(int op, TreeNode *operand_1, TreeNode *operand_2) {
    TreeNode* opt = newTreeNode(ExpK, OpK, operand_1, operand_2, NULL);
    opt->attr.op = op;
    return opt;
}

TreeNode * newDeclNode(TreeNode *type, TreeNode *idlist) {
    TreeNode *t = newTreeNode(DeclK, VarK, type, idlist, NULL);
    TreeNode *temp = idlist;
    while (temp != NULL) {
        Idmap.insert(make_pair(temp->attr.name, type->attr.op)); // 插入到符号表中
        temp->type = type->attr.op;
        temp = temp->sibling;
    }
    return t;
}

void recursive(TreeNode *p) {
    TreeNode *temp;
    int i = 0;
    for(int i = 0; i < MAXCHILDREN; i++) {
        if(p->child[i] != NULL)
        {
            recursive(p->child[i]);
        }
    }
    check_type(p);
    temp = p->sibling;
    if(temp != NULL) {
        recursive(temp);
    }
    return;
}

void check_type(TreeNode *p) {
    get_temp_var(p);
    if (p->nodekind == StmtK || p->nodekind == DeclK)
    {
        p->type = Void;
    }
    
    else if (p->nodekind == ExpK)
    {
        if (p->kind == OpK)
        {
            if (p->attr.op == Logical_not || p->attr.op == Logical_and || p->attr.op == Logical_or)
            {
                p->type = Boolean;
            }
            else if (p->attr.op == Equ || p->attr.op == Gtr || p->attr.op == Lss || p->attr.op == Geq || p->attr.op == Leq || p->attr.op == Neq) 
            {
                p->type = Boolean;
            }
            else
            {
                p->type = p->child[0]->type;
            } 
        }
        else if (p->kind == IdK)
        {
            if (Idmap.count(p->attr.name) != 0)
            {
                p->type = Idmap.at(p->attr.name); 
            }
            if (strcmp(p->attr.name, "c") == 0)
            {
                p->attr.name = "c1";
            }
        }
    }
    return;
}

// 在.data区域里写临时变量！
void get_temp_var(TreeNode *t) { // TODO: temp_var这点搞清楚！！！ 
    if (t->nodekind != ExpK || t->kind != OpK)
        return;
    if (t->attr.op == Logical_and || t->attr.op == Logical_or || t->attr.op == Logical_not)
    {
        return;
    }
    TreeNode *arg1 = t->child[0];
    TreeNode *arg2 = t->child[1];
    if (arg1->kind == OpK) {
        temp_var_seq--;
    }
    if (arg2 && arg2->kind == OpK) {
        temp_var_seq--;
    }
    t->temp_var = temp_var_seq;
    if (t->attr.op < Plus || t->attr.op > Shift_right) 
    {
        switch (t->attr.op)
        {
        case Assign:
        case Equ:
        case Gtr:
        case Lss:
        case Geq:
        case Leq:
        case Neq:
            return;
        default:
            break;
        }
    }
    temp_var_seq++;
}

void ShowNode(TreeNode *p) {
    string type = "";
    string detail = "";
    string child_lineno = "Children: ";
    if (p->nodekind == StmtK) {

        switch (p->kind) //IfK, WhileK, AssignK, ForK, CompK, InputK, PrintK
        {
            case IfK:
                type = "if statement";
                break;
            case DoWhileK:
                type = "do-while statement";
                break;
            case WhileK:
                type = "while statement";
                break;
            case ForK:
                type = "for statement";
                break;
            case CompK:
                type = "compound statement";
                break;
            case InputK:
                type = "input statement";
                break;
            case PrintK:
                type = "print statement";
                break;
            default:
                break;
        }
    } else if (p->nodekind == ExpK) {
        if (p->kind == OpK) {
            type = "Expr";
            detail = "op";
        } else if (p->kind == IdK) {
            type = "ID Declaration";
            detail = "symbol: " + string(p->attr.name);
        } else if (p->kind == ConstK) {
            type = "Const Declaration";
            detail = "value: " + to_string(p->attr.val);
        } else if (p->kind == TypeK) {
            type = "Type Specifier";
            switch (p->attr.op) {
                case Void:
                    detail = "void";
                    break;
                case Char:
                    detail = "char";
                    break;
                case Integer:
                    detail = "integer";
                    break;
                case Double:
                    detail = "double";
                    break;
                case Boolean:
                    detail = "boolean";
                    break;
                default:
                    break;
            }
        }
    } else if (p->nodekind == DeclK) {
        type = "Var Declaration";
        
    }
    cout << p->lineno << setw(20) << type << setw(20) << detail << setw(20) << child_lineno;
    for (int i = 0; i < MAXCHILDREN; ++i) {
        if (p->child[i] != NULL) {
            cout << p->child[i]->lineno << "   ";
            TreeNode *temp = p->child[i];
            while (temp->sibling != NULL) {
                cout << temp->sibling->lineno << "   ";
                temp = temp->sibling;
            }
        }
    }
    cout << endl;
}

void Display(TreeNode *p) {
    TreeNode *temp;
    for(int i = 0; i < MAXCHILDREN; i++) {
        if(p->child[i] != NULL)
        {
            Display(p->child[i]);
        }
    }
    ShowNode(p);
    temp = p->sibling;
    if(temp != NULL) {
        Display(temp);
    }
    return;
}