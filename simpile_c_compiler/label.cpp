#include "label.h"

int tag = 1;

int newLabel() {
    return tag++;
}

void recursive_get_label(TreeNode *t, int next_label) {
    if (t == NULL)
    {
        return;
    } 
    switch (t->nodekind) {
    case StmtK:
        stmt_get_label(t, next_label);
        break;
    case ExpK:
        expr_get_label(t);
        break;
    default:
        break;
    }
}

void stmt_get_label(TreeNode *t, int next_label) {
    switch (t->kind) {
        case CompK: {
            TreeNode *stmt = t->child[0];
            stmt->label.begin_label = t->label.begin_label;
            for ( ; stmt; stmt = stmt->sibling)
            {
                recursive_get_label(stmt, next_label);
            }
        }
            break;
        case WhileK: {
            TreeNode *expr = t->child[0];
            TreeNode *stmt = t->child[1];
            if (t->label.begin_label == 0) {
                t->label.begin_label = newLabel();
            }
            stmt->label.next_label = t->label.begin_label; // 继续循环
            stmt->label.begin_label = expr->label.true_label = newLabel(); // 真值 =》循环开始
            
            if (t->sibling) { // 若while语句有兄弟 
                t->label.next_label = newLabel();
                t->sibling->label.begin_label = expr->label.false_label = t->label.next_label;
            }
            else {
                expr->label.false_label = t->label.next_label = next_label;
            }
            recursive_get_label(expr, 0);
            recursive_get_label(stmt, t->label.begin_label);
        }
            break;

        case ForK: {
            TreeNode *expr_1 = t->child[0];
            TreeNode *expr_2 = t->child[1];
            TreeNode *expr_3 = t->child[2];
            TreeNode *stmt = t->child[3];
            if (t->label.begin_label != 0)
            {
                expr_1->label.begin_label = t->label.begin_label;
            }
            t->label.begin_label = newLabel();
            stmt->label.next_label = t->label.begin_label;
            stmt->label.begin_label = expr_2->label.true_label = newLabel();

            if (t->sibling) { // 若for语句有兄弟
                t->label.next_label = newLabel();
                t->sibling->label.begin_label = expr_2->label.false_label = t->label.next_label;
            }
            else {
                expr_2->label.false_label = t->label.next_label = next_label;
            }
            recursive_get_label(expr_2, 0);
            recursive_get_label(stmt, t->label.begin_label);
        }
            break;
        case IfK: {
            TreeNode *expr = t->child[0];
            TreeNode *stmt = t->child[1];
            TreeNode *else_stmt = t->child[2];
            expr->label.true_label = stmt->label.begin_label = newLabel();

            if (t->sibling) {
                t->label.next_label = newLabel();
                t->sibling->label.begin_label = expr->label.false_label = stmt->label.next_label = t->label.next_label;
            }
            else
                expr->label.false_label = stmt->label.next_label = t->label.next_label = next_label;

            if (else_stmt) {
                expr->label.false_label = else_stmt->label.begin_label = newLabel();
                else_stmt->label.next_label = t->label.next_label;
            }
            
            recursive_get_label(expr, 0);
            recursive_get_label(stmt, t->label.next_label);
            recursive_get_label(else_stmt, t->label.next_label);
        }
            break;
        default:
            break;
    }
}

void expr_get_label(TreeNode *t) {
    if (t->type != Boolean)
        return;
    TreeNode *expr_1 = t->child[0];
    TreeNode *expr_2 = t->child[1];
    switch (t->attr.op)
    {
    case Logical_and:
        expr_1->label.true_label = newLabel();
        expr_2->label.true_label = t->label.true_label;
        expr_1->label.false_label = expr_2->label.false_label = t->label.false_label;
        break;
    case Logical_or:
        expr_1->label.false_label = newLabel();
        expr_2->label.false_label = t->label.false_label;
        expr_1->label.true_label = expr_2->label.true_label = t->label.true_label;
        break;
    case Logical_not:
        expr_1->label.true_label = t->label.false_label;
        expr_1->label.false_label = t->label.true_label;
        break;
    default:
        break;
    }
    recursive_get_label(expr_1, 0);
    recursive_get_label(expr_2, 0);
}