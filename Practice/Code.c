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

int main()
{
	NaryTreeNode *n1 = create_node(1);
	NaryTreeNode *n2 = create_node(2);
	NaryTreeNode *n3 = create_node(3);
	return 0;
}

