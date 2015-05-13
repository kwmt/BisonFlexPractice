#ifndef MEM_H
#define MEM_H

typedef struct _tag_tree_node {
  int value;
  char *Name;
  struct _tag_tree_node *left;
  struct _tag_tree_node *right;
} tree_node;

tree_node *tree_root;

tree_node *create_new_node(int num);
void insert_tree(int num, tree_node *node);
void print_tree(tree_node *node);
void add_node_left(tree_node *node, tree_node *add_node);

#endif /* MEM_H */
