#include "codes.h"

ofstream out;
vector<int> symtbl;
int out_label = 0;

void gen_header() {
    out << "\t.586" << endl;
    out << "\t.model flat, stdcall" << endl;
    out << "\toption casemap :none" << endl;
    out << endl;
    out << "\tinclude \\masm32\\include\\windows.inc" << endl;
    out << "\tinclude \\masm32\\include\\user32.inc" << endl;
    out << "\tinclude \\masm32\\include\\kernel32.inc" << endl;
    out << "\tinclude \\masm32\\include\\masm32.inc" << endl;
    out << "\tinclude \\masm32\\include\\gdi32.inc" << endl;
    out << "\tinclude \\masm32\\include\\msvcrt.inc" << endl;
    out << "\tinclude \\masm32\\macros\\macros.asm" << endl;
    out << endl;
    out << "\tincludelib \\masm32\\lib\\user32.lib" << endl;
    out << "\tincludelib \\masm32\\lib\\kernel32.lib" << endl;
    out << "\tincludelib \\masm32\\lib\\masm32.lib" << endl;
    out << "\tincludelib \\masm32\\lib\\gdi32.lib" << endl;
    out << "\tincludelib \\masm32\\lib\\msvcrt.lib" << endl;
}

void gen_decl(TreeNode *t) {  // 需要用一个字符数组将char类型存储起来放在data段
    out << endl << endl << "\t.data" << endl;
    for (; t->nodekind == DeclK; t = t->sibling)
    {
        for (TreeNode *p = t->child[1]; p; p = p->sibling) {
            if (p->type == Integer)
            {
                out << "\t\t" << p->attr.name << " dd 0" << endl;
            }
            else if (p->type == Char)
            {
                out << "\t\t" << p->attr.name << " db 0" << endl;
            }
        }
    }
    for (int i = 0; i <= temp_var_seq; i++) {
        out << "\t\tt" << i << " dd 0" << endl;
    }
    out << "\t\tinttype db '%d',0" << endl; // 输出int型变量的占位符
    out << "\t\tchartype db '%c',0" << endl; // 输出char型变量的占位符
}

void recursive_gen_code(TreeNode *t) {
    if (t == NULL)
    {
        return;
    }
    else if (t->nodekind == StmtK)
    {
        stmt_gen_code(t);
    }
    else if (t->nodekind == ExpK && t->kind == OpK)
    {
        expr_gen_code(t); // 没有兄弟节点！
    }
}

void gen_code(TreeNode *root) {
    recursive(root);
    // cout << "root" << endl;
    recursive_get_label(root, 0);
    // cout << "label" << endl;

    out.open("res.asm");
    gen_header();
    TreeNode *p = root->child[0];
    if (p->nodekind == DeclK)
    {
        gen_decl(p); // 声明语句在最前面！
    }
    out << endl << endl;
    out << "\t.code" << endl;
    out << "start:" << endl;
    out << "\tpush eax" << endl; // 把eax压栈 记得在程序末出栈

    recursive_gen_code(root);
    out << "L0:" << endl;
    out << "\tpop eax" << endl;
    out << "\tinvoke crt__getch" << endl;
    out << "\tret" << endl;
    out << "end start" << endl;
}

string cover_label(int label) {
    return "L" + to_string(label);
}

string jmp_code(int label, TreeNode *t) {
    string code = "";
    if (t->sibling )
    {
        code =  "\tjmp " + cover_label(label) + "\n";
    }
    return code;
}

void stmt_gen_code(TreeNode *t) {
    switch (t->kind)
    {
    case CompK:
    {
        recursive_gen_code(t->child[0]); // 复合语句块只有一个孩子！
        for (TreeNode *p = t->child[0]->sibling; p; p = p->sibling)
        {
            recursive_gen_code(p);
        }
    }
    break;
    case WhileK:
    {
        out << cover_label(t->label.begin_label) << ":" << endl;
        recursive_gen_code(t->child[0]);
        recursive_gen_code(t->child[1]);
        out << jmp_code(t->child[1]->label.next_label, t);
    }
        break;
    case ForK:
    {
        recursive_gen_code(t->child[0]); // 初始化
        out << cover_label(t->label.begin_label) << ":" << endl;
        recursive_gen_code(t->child[1]);
        recursive_gen_code(t->child[3]);
        recursive_gen_code(t->child[2]);
        out << jmp_code(t->child[3]->label.next_label, t);
    }
        break;
    case IfK:
    {
        if (t->label.begin_label) 
        {
            out << cover_label(t->label.begin_label) << ":" << endl;
        }
        recursive_gen_code(t->child[0]);
        recursive_gen_code(t->child[1]);
        if (t->child[2]) {
            out << "\tjmp " << cover_label(t->child[1]->label.next_label) << endl;
            recursive_gen_code(t->child[2]);
        }
        out << jmp_code(t->child[1]->label.next_label, t);
    }
        break;
    case PrintK:
    {
        TreeNode *id = t->child[0];
        if (t->label.begin_label) 
        {
            out << cover_label(t->label.begin_label) << ":" << endl;
        }
        if (id->kind == OpK)
            expr_gen_code(id);
        if (id->type == Integer)
            out << "\tinvoke crt_printf, addr inttype, " << expr_child_id(id) << endl;
        else if (id->type == Char)
        {
            if (id->kind == IdK)
                out << "\tinvoke crt_printf, addr " << expr_child_id(id) << endl; 
            else if (id->kind == ConstK)
                out << "\tinvoke crt_printf, addr chartype, addr " << expr_child_id(id) << endl; 
            
        }
    }
        break;
    case InputK:
    {
        if (t->label.begin_label) 
        {
            out << cover_label(t->label.begin_label) << ":" << endl;
        }
        if (t->child[0]->type == Integer)
            out << "\tinvoke crt_scanf, addr inttype, addr " << expr_child_id(t->child[0]) << endl;
        else
            out << "\tinvoke crt_scanf, addr chartype, addr " << expr_child_id(t->child[0]) << endl;
    }
        break;
    default:
        break;
    }
}

string expr_child_id(TreeNode *expr_child) {
    string id;
    if (expr_child->kind == IdK)
        id = string(expr_child->attr.name);
    else if (expr_child->kind == ConstK) {
       id = to_string(expr_child->attr.val);
    }
    else 
        id = "t" + to_string(expr_child->temp_var);
    return id;
}

void base_expr_gen_code(TreeNode *t) {
    TreeNode *expr_1 = t->child[0];
    TreeNode *expr_2 = t->child[1];
    recursive_gen_code(expr_1);
    recursive_gen_code(expr_2);
    out << "\tmov eax, " << expr_child_id(expr_1) << endl;
    switch (t->attr.op)
    {
    case Plus:
        out << "\tadd eax, " << expr_child_id(expr_2) << endl;
        break;
    case Minus:
        out << "\tsub eax, " << expr_child_id(expr_2) << endl;
        break;
    case Times:
        out << "\timul eax, " << expr_child_id(expr_2) << endl;
        break;
    case Over:
        out << "\tcdq" << endl;
        out << "\tidiv eax, " << expr_child_id(expr_2) << endl;
    case Mod:
        out << "\tmov t" << t->temp_var << ", edx";
        return;
    case And:
        out << "\tand eax, " << expr_child_id(expr_2) << endl;
        break;
    case Or:
        out << "\tor eax, " << expr_child_id(expr_2) << endl;
        break;
    case Xor:
        out << "\txor eax, " << expr_child_id(expr_2) << endl;
        break;
    case Pplus:
        out << "\tinc eax" << endl;
        out << "\tmov eax, " << expr_child_id(expr_1) << endl;
        return;
    case Mminus:
        out << "\tdec eax" << endl;
        out << "\tmov eax, " << expr_child_id(expr_1) << endl;
        return;
    default:
        break;
    }
    out << "\tmov t" << t->temp_var << ", eax" << endl;
}

void shift_expr_gen_code(TreeNode *t) {
    recursive_gen_code(t->child[0]);
    recursive_gen_code(t->child[1]);
    out << "\tmov eax, " << expr_child_id(t->child[0]) << endl;
    out << "\tmov ecx, " << expr_child_id(t->child[1]) << endl;
    if (t->attr.op == Shift_left) {
        out << "\tshl eax, cl" << endl;
    }
    else {
        out << "\tsar eax, cl" << endl;
    }
    out << "\tmov t" << t->temp_var << ", eax" << endl;
    return;
}

void cmp_expr_gen_code(TreeNode *t) {
    recursive_gen_code(t->child[0]);
    recursive_gen_code(t->child[1]);
    out << "\tmov eax, " << expr_child_id(t->child[0]) << endl;
    out << "\tcmp eax, " << expr_child_id(t->child[1]) << endl;
    switch (t->attr.op) {
    case Equ:
        out << "\tje ";
        break;
    case Gtr:
        out << "\tjg ";
        break;
    case Lss:
        out << "\tjl ";
        break;
    case Geq:
        out << "\tjge ";
        break;
    case Leq:
        out << "\tjbe ";
        break;
    case Neq:
        out << "\tjne ";
    default:
        break;
    }
    out << cover_label(t->label.true_label) << endl;
    out << "\tjmp " << cover_label(t->label.false_label) << endl;
}

void logical_expr_gen_code(TreeNode *t) {
    TreeNode *expr_1 = t->child[0];
    TreeNode *expr_2 = t->child[1];
    switch (t->attr.op)
    {
    case Logical_and:
        recursive_gen_code(expr_1);
        out << cover_label(expr_1->label.true_label) << ":" << endl;
        recursive_gen_code(expr_2);
        break;
    case Logical_not:
        recursive_gen_code(expr_1);
        break;
    case Logical_or:
        recursive_gen_code(expr_1);
        out << cover_label(expr_1->label.false_label) << ":" << endl;
        recursive_gen_code(expr_2);
        break;
    default:
        break;
    }
}
// enum {Plus, Minus, Times, Over, Mod, Pplus, Mminus, 
// And, Or, Xor, Shift_left, Shift_right, Assign,
// Equ, Gtr, Lss, Geq, Leq, Neq, 
// Logical_and, Logical_or, Logical_not, Not}; // 逻辑运算貌似不用做任何操作
void expr_gen_code(TreeNode *t) {
    if (t->label.begin_label != 0)
    {
        out << cover_label(t->label.begin_label) << ":" << endl;
    }
    switch (t->attr.op)
    {
    case Assign:
        if (t->child[1]->kind == ConstK)
        {
            out << "\tmov " << expr_child_id(t->child[0]) << ", " << expr_child_id(t->child[1]) << endl;
        }
        else
        {
            string reg = "";
            expr_gen_code(t->child[1]);
            if (t->child[0]->type == Char)
                reg = "al";
            else
                reg = "eax";
            out << "\tmov " << reg << ", "<< expr_child_id(t->child[1]) << endl;
            out << "\tmov " << expr_child_id(t->child[0]) << ", " << reg << endl;
        }
        break;
    case Equ: 
    case Gtr: 
    case Lss: 
    case Geq:
    case Leq: 
    case Neq:
        cmp_expr_gen_code(t);
        break;
    case Shift_left:
    case Shift_right:
        shift_expr_gen_code(t);
        break;
    case Logical_and:
    case Logical_not:
    case Logical_or:
        logical_expr_gen_code(t);
        break;
    default:
        base_expr_gen_code(t);
        break;
    }
}