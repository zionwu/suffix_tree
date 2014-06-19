#pragma once

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <algorithm> 
#include <list>

using namespace std;

#define INTERNODE  0
#define INTERLEAF  1
#define LEAF       2


#define TRUE  1
#define FALSE 0

#define STRNUM 20000
#define CLASSNUM 2
#define CLASSSIZE 5

#define NAMEMAX 10
#define SEQMAX  1000
#define CLASSMAX 150

#define HIGHLEVL 0.5
#define LOWLEVL  0.3

typedef struct rawstring{
	char *string;
	struct rawstring *next;
}RAWSTRING;

typedef struct stringid{
	unsigned int str_id;
	unsigned int str_start;
	struct stringid *next;
}STRINGID;

typedef struct child_struct{
	struct child_struct *next;
	struct node *child;
}CHILD_STRUCT;

typedef struct node{
	unsigned int node_num;
	int node_type;          /*Internode; Interleaf; Leaf*/
	char *start_char;
	unsigned int edgelen;
	unsigned int char_depth;
	unsigned int stringid_num;
	unsigned int embedding_num;
	STRINGID *strings;
	struct node *suffix_link;
	CHILD_STRUCT *children;
	struct node *parent;
}NODE;

typedef struct suffixtree{
	unsigned int strnum;
	RAWSTRING *raw;
	NODE *root;
	unsigned int node_count;
}SUFFIXTREE;

typedef struct classstats{
	float occr;
	int classid;
}CLASSSTATS;
static float fuzzyclass[STRNUM][CLASSMAX];

#define SKIP_INTERLEAF t = parent->children != NULL && parent->children->child->node_type == INTERLEAF ? \
                         parent->children->next : parent->children;

int stree_insert_string( SUFFIXTREE *tree, char *string );
char *stree_find_string( SUFFIXTREE *t, unsigned int str_id );
void stree_print_leaf( NODE *node, SUFFIXTREE *tree );
int stree_print_tree( SUFFIXTREE *t );
int stree_print_subtree( NODE *node, SUFFIXTREE *t );
int stree_graph_node( NODE *node, SUFFIXTREE *tree );
int stree_print_suffix_node( NODE *node, unsigned int str_id, char *string );
int stree_add_stringid( STRINGID **p, STRINGID *t, NODE *parent );
int stree_query_string( SUFFIXTREE *t, char *string );
int stree_print_path( NODE *t );
int stree_sigstring_report(NODE *t,int *idclass);
int check_stringid_integrity( NODE *t );
int stree_fix_subtree_id( NODE *t, int *idclass );

int add_stringid( STRINGID **p, STRINGID *t, NODE *parent );
int get_stringid_num( NODE* t );
int fix_stringid( NODE *t );
int find_substring(  int min_sup, NODE *node, NODE *output[], int *output_size );
int get_closed_string(  NODE *input[], int input_size, NODE *output[], int* output_size );
int get_closed_string( NODE *input[], int input_size, list<NODE*> &node_list );
int stree_print_path_file( NODE *t, FILE *fp );
string get_substring( NODE *s );

