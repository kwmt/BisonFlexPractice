
#include <stdio.h>
#include <stdlib.h>
#include "mem.h"



tree_node *create_new_node(int num)
{
  tree_node *tree_new;

  tree_new = (tree_node*)malloc( sizeof(tree_node) );
  if( tree_new == NULL )
  {

  }
  
  tree_new->left  = NULL;
  tree_new->right = NULL;
  tree_new->value = num;

  return tree_new;
}

void insert_tree(int num, tree_node *node)
{
  if(node == NULL)
    {
      tree_root = create_new_node(num);
      return;
    }
  node->left = create_new_node(num);
  return;
}

void add_node_left(tree_node *node, tree_node *add_node)
{
  node->left = add_node;
}


void print_tree(tree_node *node)
{
    printf("node->value=%d\n", node->value);
    if(node->left != NULL)
    {
      print_tree(node->left);
    }
}
