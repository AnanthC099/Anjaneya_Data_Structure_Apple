#include <stdio.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 4
#define MALLOC_FAILED_MACRO -1

typedef struct NaryTreeNode
{
	int val;
	int child_count;
	int child_capacity;
	struct NaryTreeNode **children; 
}NaryTreeNode;


NaryTreeNode *create_node(int val)
{
	NaryTreeNode *temp_node = NULL;
	temp_node = (NaryTreeNode*)malloc( 1 * sizeof(NaryTreeNode));
	if(temp_node == NULL)
	{
		printf("ERROR: malloc failed!\n");
		exit(MALLOC_FAILED_MACRO);
	}
	temp_node->val = val;
	temp_node->child_count = 0;
	temp_node->child_capacity = INITIAL_CAPACITY;
	temp_node->children = (NaryTreeNode**)malloc(INITIAL_CAPACITY * sizeof(NaryTreeNode*));
	if(temp_node->children == NULL)
	{
		free(temp_node);
		printf("ERROR: malloc failed!\n");
		exit(MALLOC_FAILED_MACRO);
	}
	return temp_node; 
}

void add_child(NaryTreeNode *parent, NaryTreeNode *child)
{
	if (parent->child_count >= parent->child_capacity)
	{
		parent->child_capacity = parent->child_capacity * 2;
		parent->children = (NaryTreeNode**)realloc(parent->children, (parent->child_capacity * sizeof(NaryTreeNode*)));
		if(parent->children == NULL)
		{
			printf("ERROR: malloc failed!\n");
			exit(MALLOC_FAILED_MACRO);
		}
	}
	parent->children[parent->child_count++] = child;
}

NaryTreeNode *build_example_tree(void)
{
	NaryTreeNode *n1  = create_node(1);
    NaryTreeNode *n2  = create_node(2);
    NaryTreeNode *n3  = create_node(3);
    NaryTreeNode *n4  = create_node(4);
    NaryTreeNode *n5  = create_node(5);
    NaryTreeNode *n6  = create_node(6);
    NaryTreeNode *n7  = create_node(7);
    NaryTreeNode *n8  = create_node(8);
    NaryTreeNode *n9  = create_node(9);
    NaryTreeNode *n10 = create_node(10);
	
	add_child(n1, n2);
	add_child(n1, n3);
	add_child(n1, n4);
	
    add_child(n2, n5);
	add_child(n2, n6);
	add_child(n2, n7);
    
	add_child(n6, n10);
    add_child(n4, n8);
	add_child(n4, n9);
}

int main()
{
	NaryTreeNode *root = build_example_tree();
	return 0;
}

