#include "suffix_tree.h"

char *node_name[3] = { "Internode", "Interleaf", "Leaf" };

/* Allocate a new node 
* Parameter:  type: INTERNODE | INTERLEAF | LEAF
* Return:     the pointer to the new node
*             NULL if fails
* Last modified:  6/25/2002
*/

NODE *stree_alloc_node( int type )
{
	NODE *NewNode;

	if( ( NewNode = ( NODE * ) malloc( sizeof( NODE ) ) ) != NULL ){
		memset( ( void * )NewNode, 0, sizeof( NODE ) );
		NewNode->node_type = type;
		NewNode->stringid_num = 0;
		NewNode->embedding_num = 0;
	}
	else{
		printf( "Error: node allocation fail!" );
	}
	return NewNode;
}

/* Create a new CHILD_STRUCT
* Parameter:  child: the corresponding child NODE
*             next:  the next CHILD_STRUCT
* Return:     pointer to the created CHILD_STRUCT
*             NULL if fails
* Last modified:  6/25/2002
*/  

CHILD_STRUCT *stree_new_child_struct( NODE *child, CHILD_STRUCT *next )
{
	CHILD_STRUCT *NewChild;

	if( ( NewChild = ( CHILD_STRUCT * )malloc( sizeof( CHILD_STRUCT ) ) )!= NULL ){
		NewChild->child = child;
		NewChild->next = next;
	}
	else{
		printf( "Error: child struct allocation fail!" );
	}
	return NewChild;
}

/* Create a new STRINGID 
* Parameter:  str_id:    the ID of the new string
*             str_start: the starting index of the new string
*                        corresponding to the sequence on the 
*                        path from the root to this node
*             next:  the next STRINGID
* Return:     pointer to the created STRINGID
*             NULL if fails
* Last modified:  6/25/2002
*/  

STRINGID *stree_new_string_struct( unsigned int str_id, unsigned int str_start, STRINGID *next)
{
	STRINGID *NewStr;

	if( ( NewStr = ( STRINGID * ) malloc( sizeof( STRINGID ) ) ) != NULL){
		NewStr->str_id = str_id;
		NewStr->str_start = str_start;
		NewStr->next = next;
	}
	else{
		printf( "Error: String struct allocation fail!" );
	}
	return NewStr;
}

/* Create a new leaf or interleaf
* Parameter:  str_id:      the ID of the new string
*             str_start:   the starting index of the new string
*                          corresponding to the sequence on the 
*                          path from the root to this node
*             edgelen:     the number of charaters on the edge
*             char_depth:  the charater depth, which equals to
*                          the length of the sequence on the 
*                          path from the root to this node
*             c:           pointer to the string on the edge
*             isinterleaf: INTERLEAF or LEAF
*             node_count:  the total number of nodes in tree
* Return:     pointer to the created node, NULL if fails
* Last modified:  6/26/2002
*/

NODE * stree_new_leaf( unsigned int str_id, unsigned int str_start,
	unsigned int edgelen, unsigned int char_depth,
	char *c, int isinterleaf, unsigned int *node_count)
{
	NODE *NewLeaf;
	if( ( NewLeaf = stree_alloc_node( isinterleaf ? INTERLEAF : LEAF ) ) != NULL ){
		if( ( NewLeaf->strings = ( STRINGID * ) malloc( sizeof( STRINGID ) ) ) != NULL ){
			/*For INTERLEAF, the string on the edge is NULL */
			NewLeaf->start_char = isinterleaf ? NULL : c;  
			NewLeaf->edgelen = edgelen;
			NewLeaf->char_depth = char_depth;
			NewLeaf->strings->str_id = str_id;
			NewLeaf->strings->str_start = str_start;
			NewLeaf->stringid_num++;
			NewLeaf->strings->next = NULL;
			NewLeaf->node_num = ++( *node_count );
		}
	}
	else{
		printf( "Error: new leaf allocation fail!" );
		free( NewLeaf );
		NewLeaf = NULL;
	}

	return NewLeaf;
}

/* Create a new RAWSTRING
* Parameter: len: the length of the raw string
* Return:    the created RAWSTRING
*            NULL, if fails
* Last modified: 6/25/2002
*/

RAWSTRING *stree_alloc_rawstring( unsigned int len )
{
	RAWSTRING *t;
	if( ( t = ( RAWSTRING * )malloc( sizeof( RAWSTRING ) ) ) != NULL ){
		if( ( t->string = ( char * )malloc( sizeof( char )*len ) ) == NULL ){
			free( t );
			t = NULL;
		}
		else{ 
			t->next = NULL;
		}
	}
	return t;
}


/* Check if the string ID -str_id- already exists
* in the node.
* Parameter:  head:   the pointer to the string ID 
*                     linked list of  the node.
*             str_id: the string ID in search.
*             slot:   the address of the return pointer
*                     (used for return). 
* Return:     0       If str_id in node
*                     slot points to the STRINGID
*                     of str_id in search
*             1       otherwise
*                     slot points to the place to
*                     insert the new string ID
* Last modified: 6/25/2002
*/

int stree_check_stringid( STRINGID *head, unsigned int str_id, STRINGID **slot)
{
	STRINGID *p,*t;
	t = head;
	if( str_id == 0 ) 
		return 0;
	/* When the ID need not to be checked */
	while( t != NULL && t->str_id < str_id ){
		p = t;
		t = t->next;
	}
	if( t != NULL && t->str_id == str_id ){
		*slot = t;
		return 0;
	}
	*slot = p;
	return 1;
}

/* Get the CHILD_STRUCT of the child node whose
* edge starts with character c
* Parameter:  parent: the parent node
*             c:      the starting character
* Return:     pointer to the CHILD_STRUCT if there
*             is an edge starts with c from parent,
*             NULL otherwise. 
* Last modified: 6/25/2002
*/

CHILD_STRUCT * stree_get_child( NODE *parent, char c )
{
	CHILD_STRUCT *t;
	SKIP_INTERLEAF
		while( t != NULL && t->child->start_char[0] != c )
			t = t->next;
	return t;
}

/* Insert the child node into the parent node
* Parameter:  parent:     the parent node 
*             child:      the node to be inserted
*             node_count: the total number of nodes in the tree 
* Return:     If the node with the string ID already exists,return
*             it and free child.Otherwise, return the new inserted
*             node. If fails, return NULL
* Last modified: 6/27/2002
*/

NODE * stree_insert_child( NODE *parent, NODE *child, unsigned int *node_count)
{
	CHILD_STRUCT *newChild, *p, *t;
	STRINGID *s, *snew;

	if( child->start_char == NULL ){  
		/* the child node is INTERLEAF */
		if( parent->children->child->node_type != INTERLEAF ){ 
			/*No INTEALEAF in the parent node. (A node has only oneINTERLEAF child node) */
			if( ( newChild = stree_new_child_struct( child, parent->children ) ) == NULL )  
				return NULL;
			parent->children = newChild;
			child->parent = parent;
			return child;
		}
		/* INTERLEAF exists, check string ID */
		if( stree_check_stringid( parent->children->child->strings, child->strings->str_id, &s ) ){
			if( ( snew = stree_new_string_struct( child->strings->str_id, child->strings->str_start, s->next ) ) == NULL){
				return NULL;
			}
			s->next = snew;
			parent->stringid_num++;
		}
		free( child );
		( *node_count )--;
		return parent->children->child;
	}

	/* the child node is a normal LEAF, check if it already exists */
	p = NULL;
	SKIP_INTERLEAF
		while( t != NULL && t->child->start_char[0] < child->start_char[0] ){
			p = t;
			t = t->next;
		}
		if( t != NULL && t->child->start_char[0] == child->start_char[0] && 
			stree_check_stringid( t->child->strings, child->strings->str_id, &s ) ){
				if( ( snew = stree_new_string_struct( child->strings->str_id, child->strings->str_start, s->next ) ) == NULL){
					return NULL;
				}
				s->next = snew;
				parent->stringid_num++;
				free( child );
				( *node_count )--;
				return t->child;
		}
		/* Does not exist, insert into parent node */
		if( ( newChild = stree_new_child_struct( child, t ) ) == NULL ){
			return NULL;
		}
		if( p == NULL ){
			parent->children = newChild;
		}
		else{
			p->next = newChild;
		}
		child->parent = parent;
		return child;
}

/* Split the edge of child and link node onto it
* Parameter:  node:       the node to be linked
*             child:      the node whose edge is to be split
*             edgeindex:  the index when the last charater
*                         check failed 
*             node_count: the total number of nodes in tree
* Return:     0   if successful, 1 otherwise
* Last modified: 6/27/2002
*/ 

int stree_link_node( NODE *node, NODE *child, unsigned int edgeindex, unsigned int *node_count)
{
	CHILD_STRUCT *t;
	NODE *parent;
	assert( ( parent = child->parent ) );
	if( stree_insert_child( node, child, node_count ) == NULL )   
		return 1;

	/*find the pointer in the parent that points to the
	child and modify it */
	t = stree_get_child( parent, child->start_char[0] );
	assert( t );
	//  if(stree_add_stringid(&node->strings,child->strings)) return 1;
	t->child = node;
	node->start_char = child->start_char;
	node->edgelen = edgeindex + 1;
	node->char_depth = child->char_depth - child->edgelen + node->edgelen;
	node->parent = parent;
	node->node_num = ++( *node_count );
	//node->stringid_num = child->stringid_num + 1; 
 
	child->start_char = &( child->start_char[edgeindex+1] );
	child->edgelen -= node->edgelen;
	//  if (child->edgelen==0) child->node_type=INTERLEAF;
	return 0;
}

/* Check if character c exists on the edge of
* the parent node when the current index is in
* the middle of the edge, or if c exists on any
* path under parent node when the current index
* is the last character of the edge.
* Parameter: parent:    the node to be checked
*            newnode:   the node where c exists
*            edgeindex: the current index
*            newindex:  the index of c
*            str_id:    the ID of the string in search
*            c:         the character in search
* Return:    0   c is found with the correct string id
*            1   c is found, but with wrong string id
*            2   c is not found
* Last modified: 6/27/2002
*/

int stree_check_next(NODE *parent, NODE **newnode,
	int edgeindex, int *newindex, unsigned int str_len,
	unsigned int str_id, char c)
{
	CHILD_STRUCT *t;
	STRINGID *temp;

	if( edgeindex >= ( int )parent->edgelen - 1 ){ 
		/* the current index is the last character */
		if( parent->node_type == LEAF && c == 0 ) 
			return 1;
		t = parent->children;
		if( t != NULL && c == 0 ){ 
			/* the character in search is the null ending symbol */
			if( t->child->node_type == INTERLEAF || ( t->child->node_type == LEAF && t->child->edgelen == 0 ) ){
				return stree_check_stringid( t->child->strings, str_id, &temp );
			}
		}
		t = stree_get_child( parent, c );
		/* For INTERNODE, don't check string ID. The string ID 
		consistency will be fixed after the whole string is added */
		if( t == NULL || ( t->child->node_type == INTERLEAF && stree_check_stringid( t->child->strings, str_id, &temp ) ) )  
			return 2;
		/* the new index is the first character on the corresponding edge */
		*newnode = t->child;
		*newindex = 0;
		return 0;
	}
	/* the current index is in the middle of the edge,
	check the next character on the edge */
	if( parent->start_char[edgeindex+1] != c )   
		return 2;
	*newindex = edgeindex + 1;
	*newnode = parent;
	return 0;
}

/* walk down the path from the start node according to the string 
* Parameter:  start:  the node under which the walk starts
*             string: the query string
*             len:    the number of characters to walk
*             str_id: the ID of the string
* Return:     the node on whose edge the walk ends,
*             NULL if the string is empty or len less than 1
* Last modified: 6/27/2002
*/

NODE * stree_walk_down( NODE *start, char *string, unsigned int len, unsigned int str_id )
{
	unsigned int i;
	int n;
	NODE *p,*t;
	if( string == NULL && len < 1 ) 
		return NULL;
	for( i = 0, n = start->edgelen - 1, p = start; i < len; i++, p = t ){
		if( stree_check_next( p, &t, n, &n, len, str_id, string[i] ) )
			return NULL;
	}
	return p;
}

NODE * stree_walk_and_fill( NODE *start, char *string, unsigned int len, int *s, int size )
{
	unsigned int i;
	int n;
	NODE *p, *t;
	if( string == NULL && len < 1 ) 
		return NULL;
	for( i = 0 , n = start->edgelen - 1, p = start; i < len; i++, p = t ){
		if( stree_check_next( p, &t, n, &n, len, 0, string[i] ) )
			return NULL;
		if( n == t->edgelen - 1 ){
			if( t->node_type == LEAF || t->children->child->node_type == INTERLEAF ){
				s[size-i-1] = TRUE;
			}
		}		  
	}
	if( n == t->edgelen - 1 && t->node_type == LEAF )
		return NULL;
	return p;
}	

NODE * stree_walk_and_report( NODE *start, char *string,
	unsigned int len, int *s, int size, int nid)
{
	unsigned int i;
	int n;
	NODE *p, *t;
	if( string == NULL && len < 1 ) 
		return NULL;
	for( i = 0, n = start->edgelen - 1, p = start; i < len; i++, p = t ){
		if( stree_check_next( p, &t, n, &n, len, 0, string[i] ) )
			return NULL;
		if( n == t->edgelen - 1 ){
			if( t->node_type == LEAF || t->children->child->node_type == INTERLEAF ){
				if( s[i+1] == TRUE ){
					printf( "The pattern occurs at index %d in node %d\n", size - i, nid );
				}
			}
		}		  
	}
	if( n == t->edgelen - 1 && t->node_type == LEAF )
		return NULL;
	return p;
}	

/* the skip-count trick used in the tree construction.
* similar to stree_walk_down, but no string ID check
* and matching on a edge base
* Parameter:  start:     the node under which the match starts
*             string:    the query string
*             edgeindex: pointer to new edge index (for return)
*             len:       the number of characters to match
* Return:     the node on whose edge the match ends and set
*             the corresponding edge index
* Last modified: 6/27/2002
*/

NODE * stree_skip_count( NODE *start, char *string, int *edgeindex, int len )
{
	int n, i;
	NODE *p, *t;
	assert( string != NULL );
	if( len <= 0 ){
		*edgeindex = start->edgelen - 1;
		return start;
	}
	for( i = 0, n = start->edgelen - 1, p = start;	i < len; i += t->edgelen, p = t, n = t->edgelen - 1 ){
		if( stree_check_next( p, &t, n, &n, len, 0, string[i] ) == 2 ) 
			break;
	}
	*edgeindex = p->edgelen - ( i - len ) - 1;
	return p;
}

/* the follow suffix link trick used in the tree construction
* Parameter: last:      the new leaf or new internode
*            edgeindex: the current edge index 
* Return:    the node where the next query should start with
*            and set the corresponding current edge index
* Last modified: 6/27/2002
*/

NODE * stree_follow_suffix( NODE *last, int *edgeindex )
{
	NODE *s, *child, *parent;
	parent = last->parent;
	s = parent->suffix_link;
	if( last->node_type != INTERNODE ){ 
		/* if a new leaf or interleaf, always query from the 
		internode to which th suffix link points */ 
		child = s;
		*edgeindex = s->edgelen - 1;
	}
	else{
		if( parent->char_depth != s->char_depth ){
			/* if parent is not the root */
			child = stree_skip_count( s, last->start_char, edgeindex, last->edgelen );
		}
		else{
			child = stree_skip_count( s, last->start_char + 1, edgeindex, last->edgelen - 1 );
		}
	}
	return child;
}

/* Find the last slot to insert a new raw string
* Parameter:  head: the head of the linked list
* Return:     pointer to the last RAWSTRING 
* Last modified: 6/25/2002
*/

RAWSTRING * stree_find_last( RAWSTRING *head )
{
	RAWSTRING *t;
	for( t = head; t->next != NULL; t = t->next );
	return t;
}  

/* Insert a new string into the suffix tree.
* Parameter: tree:   the SUFFIXTREE 
*            string: the new string
* Return:    If successful, return 0.
*            Otherwise, return 1.
* Last modified: 6/27/2002
*/

int stree_insert_string( SUFFIXTREE *tree, char *string )
{
	int i, j, len, tag, p;
	int edgeindex, lastindex, depth, newindex;
	NODE *lastnode, *suffix_update, *childnode, *newleaf, *newnode;
	RAWSTRING *last;
	STRINGID *temp;

	if( tree->strnum == 0 ){/* no string exists, create new tree */
		if( ( tree->root = stree_alloc_node( INTERNODE ) ) == NULL ) 
			return 1; 
		tree->root->suffix_link = tree->root->parent = tree->root;
		tree->node_count = tree->root->node_num = 1;
		if( ( last = tree->raw = stree_alloc_rawstring( strlen( string ) + 1 ) ) == NULL ){ 
			free( tree->root );
			return 1;
		}
	}
	else{
		last = stree_find_last( tree->raw );
		if( ( last->next = stree_alloc_rawstring( strlen( string ) + 1 ) ) == NULL ) 
			return 1;
		last = last->next;
	}
	strncpy( last->string, string, strlen( string ) );
	last->string[strlen( string )] = 0;
	tree->strnum++;

	len = strlen( last->string );
	lastnode = tree->root;
	lastindex = 0;
	edgeindex = -1;
	suffix_update = NULL;
	for( i = 0; i <= len; i++ ){
		if( i == len && lastnode == tree->root && edgeindex == -1 ){
			break;
		}
		tag = 0;
		for( j = lastindex; j <= i; j++ ){
			if( j == len ){
				if(	tag && suffix_update != NULL ){
					suffix_update->suffix_link = lastnode;
				}
				break;
			}

			if( ( p = stree_check_next( lastnode, &childnode, edgeindex, &newindex,
				len, tree->strnum, last->string[i] ) ) == 0 ){ /*in tree */
					if( tag && suffix_update != NULL ){
						suffix_update->suffix_link = lastnode;
						tag = 0;
					}
					edgeindex = newindex;
					lastnode = childnode;
					break;
			}
			else{ 
				if( p == 1 ){
					if( lastnode->node_type == LEAF ){
						assert( stree_check_stringid( lastnode->strings, tree->strnum, &temp ) == 1 );
						lastnode->stringid_num++;
					}
					else{
						assert( stree_check_stringid( lastnode->children->child->strings,
							tree->strnum, &temp ) == 1 );
						lastnode->children->child->stringid_num++; 
					}
					temp->next = (STRINGID *)malloc( sizeof( STRINGID ) );
					temp->next->str_id = tree->strnum;
					temp->next->str_start = len - lastnode->char_depth;
					temp->next->next = NULL;
					if( tag && suffix_update != NULL ){
						suffix_update->suffix_link = lastnode;
						tag = 0;
					}
				}
				else{
					if( edgeindex >= ( int )lastnode->edgelen - 1 ){
						depth = len - j - lastnode->char_depth;
					}
					else{
						depth = len - j - lastnode->char_depth + lastnode->edgelen - edgeindex - 1;
					}

					if( ( newleaf = stree_new_leaf( tree->strnum, j, i == len ? 0 : depth, len-j,
						&last->string[len - depth], i == len, &tree->node_count ) ) == NULL ){
							return 1;
					}

					if( edgeindex < ( int ) lastnode->edgelen - 1 || lastnode->node_type == LEAF ){
						if( ( newnode = stree_alloc_node( INTERNODE ) ) == NULL ) 
							return 1;
						stree_link_node( newnode, lastnode, edgeindex, &tree->node_count );
						if( tag && suffix_update != NULL ){
							suffix_update->suffix_link = newnode;
						}
						tag = 1;
						suffix_update = newnode;
						if( stree_insert_child( newnode, newleaf, &tree->node_count ) == NULL ) 
							return 1;
						lastnode = newnode;
					}
					else{
						if( tag && suffix_update != NULL ){
							suffix_update -> suffix_link = lastnode;
							tag = 0;
						}
						lastnode = stree_insert_child( lastnode, newleaf, &tree->node_count );
					}
				}
				lastindex = j + 1;
				lastnode = stree_follow_suffix( lastnode, &edgeindex );
			}
		}
	}
	//  stree_fix_stringid_suffixlink(tree,tree->strnum);
	return 0;
}

char *stree_find_string( SUFFIXTREE *t, unsigned int str_id )
{
	RAWSTRING *s;
	unsigned int i;
	if( str_id>t->strnum ) 
		return NULL;
	for( s = t->raw, i = 0; i < str_id - 1; i++, s = s->next );
	return s->string;
}

void stree_print_leaf( NODE *node, SUFFIXTREE *tree )
{
	CHILD_STRUCT *c;
	if( node->node_type != INTERNODE )
		stree_graph_node( node, tree );
	else{
		for( c = node->children; c != NULL; c = c->next ){
			stree_print_leaf( c->child, tree );
		}
	}
}

void stree_print_suffix( SUFFIXTREE *t )
{
	printf( "\n\n\n\t\t\t\t\t|************************************|\n" );
	printf( "\t\t\t\t\t Printing all the suffix in the tree:\n" );
	printf( "\t\t\t\t\t|************************************|\n\n\n" );
	stree_print_leaf( t->root, t );
}

void stree_print_suffix_withid( SUFFIXTREE *t, unsigned int str_id )
{
	char *c = stree_find_string( t, str_id );
	printf( "\n\nPrinting the suffix of string %d:\n", str_id );
	stree_print_suffix_node( t->root,str_id, c);
}

int stree_print_tree( SUFFIXTREE *t )
{
	stree_print_subtree( t->root, t );
	return 0;
}

int stree_print_subtree( NODE *node, SUFFIXTREE *t )
{
	CHILD_STRUCT *c;
	stree_graph_node( node, t );
	printf( "the path is  " );
	stree_print_path( node );
	printf( "\n" );
	for( c = node->children; c != NULL; c = c->next ){
		stree_print_subtree( c->child, t );
	}
	return 0;
}

int stree_graph_node( NODE *node, SUFFIXTREE *tree )
{
	STRINGID *t;
	CHILD_STRUCT *c;
	unsigned int i;
	printf( "|" );
	for( i = 0 ; i < 80; i++ )
		printf( "*" );
	printf( "|" );
	printf( "\nNode # %d:\n", node->node_num );
	printf( "Node Type: %s\n", node_name[node->node_type] );
	printf( "String Num: %d\n", node->stringid_num );
	if( node->start_char != NULL ){
		printf( "Character(s) on the edge:\t\t " );
		for( i = 0; i < node->edgelen; i++ )
			printf( "%c", node->start_char[i] );
		printf( "\n" );
	}
	else{
		printf( "There are no characters on this edge\n" );
	}
	printf( "Edge length is %d\n", node->edgelen );
	printf( "the depth of character is %d\n", node->char_depth );
	t = node->strings;
	while( t != NULL ){
		if( node->node_type == INTERNODE )
			printf( "the first appearance is %d in string %d\n", t->str_start, t->str_id );
		else
			printf( "The corresponding suffix is\t\t %s\n Starts from index %d in string %d\n",
			stree_find_string( tree, t->str_id ) + t->str_start, t->str_start, t->str_id );
		t = t->next;
	}
	if( node->node_type == INTERNODE )
		printf( "The suffix-link points to node %d\n", node->suffix_link->node_num );
	if( node->node_type == INTERNODE ){
		printf( "\n" );
		for( i = 0, c = node->children; c != NULL; c = c->next, i++ ){
			printf( "\t\tthe %d child is node %d\n", i, c->child->node_num);
		}
		printf( "\n" );
	}
	printf( "The parent node is %d\n", node->parent->node_num );
	return 0;
}  

int stree_print_suffix_node( NODE *node, unsigned int str_id, char *string )
{
	CHILD_STRUCT *t;
	STRINGID *s;
	if( node->node_type != INTERNODE){
		for( s = node->strings; s != NULL && s->str_id != str_id; s = s->next );
		if(s != NULL){
			printf( "Starting position %4ld:\t%s\n", s->str_start, &string[s->str_start] );
		}
	}
	else{
		t = node->children;
		while( t != NULL ){
			stree_print_suffix_node( t->child, str_id, string );
			t = t->next;
		}
	}
	return 0;
}

int stree_add_stringid( STRINGID **p, STRINGID *t, NODE *parent )
{
	STRINGID *pa, *a, *b, *NewStr;
	b = t;
	if( *p == NULL && b != NULL ){
		if( ( NewStr = ( STRINGID * )malloc( sizeof( STRINGID ) ) ) == NULL )
			return 1;
		NewStr->str_id = b->str_id;
		NewStr->str_start = b->str_start;
		NewStr->next = NULL;
		*p = NewStr;
		parent->stringid_num++;
		b = b->next;
	}
	pa = NULL;
	a = *p;
	while( a != NULL && b != NULL ){
		if( a->str_id <= b->str_id ){
			if( a->str_id == b->str_id ){
				if( a->str_start > b->str_start ){
					a->str_start = b->str_start;
				}
				b = b->next;
			}
			pa = a;
			a = a->next;
		}
		else{
			if( ( NewStr = ( STRINGID * )malloc( sizeof( STRINGID ) ) ) == NULL )
				return 1;
			NewStr->str_id = b->str_id;
			NewStr->str_start = b->str_start;
			if( pa == NULL ){
				( *p ) = NewStr;
			}
			else{
				pa->next = NewStr;
			}
			parent->stringid_num++;
			pa = NewStr;
			NewStr->next = a;
			b = b->next;
		}
	}
	while( b != NULL ){
		if( ( NewStr = ( STRINGID* )malloc( sizeof( STRINGID ) ) ) == NULL )
			return 1;
		NewStr->str_id = b->str_id;
		NewStr->str_start = b->str_start;
		NewStr->next = NULL;
		parent->stringid_num++;
		pa->next = NewStr;
		pa = NewStr;
		b = b->next;
	}
	return 0;
}

NODE * stree_next_suffix( NODE *node, unsigned int str_id )
{
	NODE *parent, *t;
	if( node->node_type == INTERNODE ){
		t = node->suffix_link;
	}
	else{
		parent = node->parent;
		if( node->node_type == INTERLEAF ){
			t = parent->suffix_link;
		}
		else{
			if( parent->char_depth != parent->suffix_link->char_depth ){
				t = stree_walk_down( parent->suffix_link, node->start_char,	node->edgelen,str_id );
			}
			else{
				if( node->edgelen > 1 ){
					t = stree_walk_down( parent->suffix_link, node->start_char + 1, node->edgelen - 1, str_id );
				}
				else{
					return parent->suffix_link;
				}
			}
		}
	}
	if( t->node_type == INTERNODE && t->node_num != 1 )
		t = t->children->child;
	assert( t );
	return t;  
}

int stree_test( SUFFIXTREE *tree )
{
	NODE *t;
	char *c;
	c = stree_find_string( tree, 2 );
	t = stree_walk_down( tree->root, c, strlen( c ), 2 );
	while( t != tree->root ){
		printf( "Node #:\t %d\n", t->node_num );
		t = stree_next_suffix( t, 2 );
	}
	return 0;
}

int stree_query_string( SUFFIXTREE *t, char *string )
{
	NODE *start;
	if( ( start = stree_walk_down( t->root, string, strlen( string ), 0 ) ) == NULL )
		return 1;
	stree_print_leaf( start, t );
	return 0;
}

void stree_fill_leaf( NODE *node, int *s )
{
}

int stree_fill_pattern( SUFFIXTREE *p, char *string, int *s, int size)
{
	NODE *start;
	if( ( start = stree_walk_and_fill( p->root, string, strlen( string ), s, size ) ) == NULL )
		return 1;
	stree_fill_leaf( start, s );
	return 0;
}

int stree_report_occurence( SUFFIXTREE *p, char *string, int *s, int size, int nid )
{
	NODE *start;
	if( ( start = stree_walk_and_report( p->root, string, strlen( string ), s, size,nid ) ) == NULL )
		return 1;
	stree_print_leaf( start, p );
	return 0;
}

int stree_print_path( NODE *t )
{
	NODE *p = t;
	char *ch;
	int i;

	if ( p->parent != p ){
		stree_print_path( p->parent );
		for( i = 0, ch = t->start_char; i < p->edgelen; i++, ch++ )
			printf( "%c", *ch );
	}
	return 0;
}

int whichclass( int a, int *b )
{
	int i;
	for( i = 0; a > b[i]; a -= b[i++] );
	return i;
}

int stree_sigstring_report(NODE *t,int *idclass)
{
	int i, count[CLASSMAX] = {0}, max_index, secmax_index;
	double max, secmax;
	STRINGID *p;
	CHILD_STRUCT * child;
	for( i = 0 , p = t->strings; p != NULL && i < t->stringid_num - 1; p = p->next, i++ ){
		count[whichclass( ( p->str_id ), idclass )]++;
	}
	max = ( double )count[0] / ( double )idclass[0];
	max_index = 0;
	secmax = -1;
	for( i = 1 ;i<CLASSMAX; i++ ){
		if( ( double ) count[i] / ( double )idclass[i] > max ){
			secmax = max;
			secmax_index = max_index;
			max = ( double )count[i] / ( double )idclass[i];
			max_index = i;
		}
		else{
			if( ( double ) count[i] / ( double )idclass[i] > secmax){
				secmax = ( double ) count[i] / ( double )idclass[i];
				secmax_index = i;
			}
		}
	}
	if (/*max>=HIGHLEVL&&secmax<=LOWLEVL&&*/max * ( double ) idclass[max_index] >= 75 ){
		printf( "\nThe signature substring for class %d is ", max_index + 1 );
		stree_print_path( t );
		printf(" with %.0f instances.\n", max*(double)idclass[max_index]);
	}

	for( child = t->children; child != NULL; child = child->next ){
		if( child->child->node_type == INTERNODE ||
			( child->child->node_type == LEAF && child->child->edgelen != 0 ) ){
				stree_sigstring_report( child->child, idclass );
		}
	}
	return 0;
}

int check_stringid_integrity( NODE *t )
{
	int i = 0;
	STRINGID *p;
	p = t->strings;
	while(p != NULL ){
		i++;
		p = p->next;
	}
	return i == t->stringid_num ? 1 : 0;
}

int stree_fix_subtree_id( NODE *t, int *idclass )
{
	int i, count[CLASSMAX] = {0}, max_index, secmax_index, ptag;
	double max, secmax;
	STRINGID *p;
	CHILD_STRUCT * child;
	ptag = 0;

	for( child = t->children; child != NULL; child = child->next ){
		if( child->child->node_type == INTERNODE ){
			ptag += stree_fix_subtree_id( child->child, idclass );
		}
		stree_add_stringid( &t->strings, child->child->strings, t );
		if( !check_stringid_integrity ( t ) ){
			printf( "Stringid number incorrect in node %d!\n", t->node_num );
		}
	}
	if( ptag == 0 ){
		for( i = 0, p = t->strings; p != NULL && i < t->stringid_num; p = p->next, i++){
			count[whichclass( ( p->str_id ), idclass )]++;
		}

		max = ( double )count[0] / ( double )idclass[0];
		max_index = 0;
		secmax = -1;

		for( i = 1; i < CLASSMAX; i++ ){
			if( ( double )count[i] / ( double )idclass[i] > max ){
				secmax = max;
				secmax_index = max_index;
				max = ( double )count[i] / ( double )idclass[i];
				max_index = i;
			}
			else{
				if( ( double )count[i] / ( double )idclass[i] > secmax){
					secmax = ( double )count[i] / ( double )idclass[i];
					secmax_index = i;
				}
			}
		}
		if (/*max>=HIGHLEVL&&secmax<=LOWLEVL&&*/ max * ( double ) idclass[max_index] >= 500 ){

			printf( "\n%d\t", max_index + 1 );
			stree_print_path( t );
			printf( "\t%.0f\t %.2f\t %.2f\n", max * ( double )idclass[max_index], max, secmax );
			ptag = 1;  
		}										
	}
	return ptag;
}

/*add all the string ids to its parent, including all the occurences of the same substring

*/
int add_stringid( STRINGID **p, STRINGID *t, NODE *parent )
{
	STRINGID *pa, *a, *b, *NewStr;
	b = t;
	if( *p == NULL && b != NULL ){
		if( ( NewStr = ( STRINGID * )malloc( sizeof( STRINGID ) ) ) == NULL )
			return 1;
		NewStr->str_id = b->str_id;
		NewStr->str_start = b->str_start;
		NewStr->next = NULL;
		*p = NewStr;
		//parent->stringid_num++;
		b = b->next;
	}
	pa = NULL;
	a = *p;
	while( a != NULL && b != NULL ){
		if( a->str_id < b->str_id ){
			pa = a;
			a = a->next;
		}
		else if( a->str_id == b->str_id ){
			if( ( NewStr = ( STRINGID * )malloc( sizeof( STRINGID ) ) ) == NULL )
				return 1;
			NewStr->str_id = b->str_id;
			NewStr->str_start = b->str_start;
			if( a->str_start < b->str_start ) {
				NewStr->next = a->next;
				a->next = NewStr;
				pa = a;
				a = NewStr;
			}
			else{
				if( pa == NULL ){
					( *p ) = NewStr;
				}
				else{
					pa->next = NewStr;
				}
				pa = NewStr;
				NewStr->next = a;
			}
			b = b->next;
		}
		else{
			if( ( NewStr = ( STRINGID * )malloc( sizeof( STRINGID ) ) ) == NULL )
				return 1;
			NewStr->str_id = b->str_id;
			NewStr->str_start = b->str_start;
			if( pa == NULL ){
				( *p ) = NewStr;
			}
			else{
				pa->next = NewStr;
			}
			//parent->stringid_num++;
			pa = NewStr;
			NewStr->next = a;
			b = b->next;
		}
	}
	while( b != NULL ){
		if( ( NewStr = ( STRINGID* )malloc( sizeof( STRINGID ) ) ) == NULL )
			return 1;
		NewStr->str_id = b->str_id;
		NewStr->str_start = b->str_start;
		NewStr->next = NULL;
		//parent->stringid_num++;
		pa->next = NewStr;
		pa = NewStr;
		b = b->next;
	}
	return 0;
}

int get_stringid_num( NODE* t )
{
	int num = 0;
	STRINGID *p, *pre;
	p = t->strings;
	while( p != NULL ){
		pre = p;
		p = p->next;
		if( p == NULL)
			num++;
		else if( pre->str_id != p->str_id )
			num++;
	}

	return num ;
}

int get_embedding_num( NODE* t )
{
	int num = 0;
	STRINGID *p;
	p = t->strings;
	if( p == NULL )
		return 0;
	num = 1;
	while( p->next != NULL ){
		num++;
		p = p->next;
	}
	return num ;
}

// fix the string id of each node of the tree when multiple occurences of substring are taken into consideration.
int fix_stringid( NODE *t )
{
	int i;
	STRINGID *p;
	CHILD_STRUCT * child;

	for( child = t->children; child != NULL; child = child->next ){
		if( child->child->node_type == INTERNODE ){
			 fix_stringid( child->child );
		}
		// bottom-up method, add the children's ids to the parent'.
		add_stringid( &t->strings, child->child->strings, t );
		t->stringid_num =  get_stringid_num( t );
		t->embedding_num = get_embedding_num( t );
	}
	return 0;
}
// travse the tree to find substring given a threshold sup
int find_substring(  int min_sup, NODE *node, NODE *output[], int *output_size )
{
	CHILD_STRUCT *c;

	if( node->stringid_num >= min_sup && node->char_depth != 0 ){
		output[*output_size] = node;
		(*output_size)++;
	}
	for( c = node->children; c != NULL; c = c->next ){
		find_substring( min_sup, c->child, output, output_size );
	}
	return 0;
}

string get_substring( NODE *s )
{
	NODE *p = s;
	string str ;
	if( p->parent != p ){
		str = get_substring( p->parent );
		string t = string( p->start_char, p->start_char + p->edgelen );
		str +=  t;
		return str;
	}
	str = "";
	return str;
}	

int get_closed_string( NODE *input[], int input_size, NODE *output[], int* output_size )
{
	list<NODE*> node_list;
	for( int i = 0; i < input_size; i ++ ){
		node_list.push_front( input[i] );
	}
	
	bool is_delete; 
	list<NODE*>::iterator it1, it2, end;
	string str1, str2;
	it1 = node_list.begin();
	end =  node_list.end();

	while( it1 != end ){
		is_delete = false;
		it2 = it1;
		++it2;
		while( it2 != end ){
			if( (*it1)->char_depth > (*it2)->char_depth ){ 
				str1 = get_substring( *it1 );
				str2 = get_substring( *it2 );
				char *ptr= strstr( const_cast<char*>(str1.c_str()) , str2.c_str() );
				if( ptr != NULL ){
					if( (*it1)->stringid_num == (*it2)->stringid_num ){
						NODE* t = *(it2);
						++it2;
						node_list.remove( t );
						continue;
					}
				}
			}
			else if( (*it1)->char_depth < (*it2)->char_depth ){
				str1 = get_substring( *it1 );
				str2 = get_substring( *it2 );
				char *ptr= strstr( const_cast<char*>(str2.c_str()) , str1.c_str() );
				if( ptr != NULL ){
					if( (*it1)->stringid_num == (*it2)->stringid_num ){
						is_delete = true;
						NODE* t = *(it1);
						++it1;
						node_list.remove( t );
						break;
					}
				}
			}
			++it2;
		}
		if( is_delete == true )
			continue;
		++it1;
	}

	*output_size = 0;
	for( it1 = node_list.begin(); it1 != node_list.end(); it1++ )
	{
		output[ (*output_size)++ ] = *it1;
	}

	return 0;
}

int get_closed_string( NODE *input[], int input_size, list<NODE*> &node_list )
{
	for( int i = 0; i < input_size; i ++ ){
		node_list.push_front( input[i] );
	}
	
	bool is_delete; 
	list<NODE*>::iterator it1, it2, end;
	string str1, str2;
	it1 = node_list.begin();
	end =  node_list.end();

	while( it1 != end ){
		is_delete = false;
		it2 = it1;
		++it2;
		while( it2 != end ){
			if( (*it1)->char_depth > (*it2)->char_depth ){ 
				str1 = get_substring( *it1 );
				str2 = get_substring( *it2 );
				char *ptr= strstr( const_cast<char*>(str1.c_str()) , str2.c_str() );
				if( ptr != NULL ){
					if( (*it1)->stringid_num == (*it2)->stringid_num ){
						NODE* t = *(it2);
						++it2;
						node_list.remove( t );
						continue;
					}
				}
			}
			else if( (*it1)->char_depth < (*it2)->char_depth ){
				str1 = get_substring( *it1 );
				str2 = get_substring( *it2 );
				char *ptr= strstr( const_cast<char*>(str2.c_str()) , str1.c_str() );
				if( ptr != NULL ){
					if( (*it1)->stringid_num == (*it2)->stringid_num ){
						is_delete = true;
						NODE* t = *(it1);
						++it1;
						node_list.remove( t );
						break;
					}
				}
			}
			++it2;
		}
		if( is_delete == true )
			continue;
		++it1;
	}

	return 0;
}

/*
int get_closed_string( int sup, NODE *tar[], int num, NODE *init_can[], int* count )
{
	int i, j, flag;
	NODE *p, *q;
	STRINGID *pstr, *qstr, *pre;; 
	for( i = 0; i < num; i++ ){
		for( j = i+1; j < num; j++){
			if( tar[i]->char_depth <= tar[j]->char_depth ){
				p = tar[i];
				q = tar[j];
			}
			else{
				p = tar[j];
				q = tar[i];
			}
			
			pre = NULL;
			pstr = p->strings;	
			while( pstr != NULL ){
				qstr = q->strings;
				flag = 0;
				while( qstr != NULL){
					if( pstr->str_id < qstr->str_id )
						break;
					else if( pstr->str_id > qstr->str_id )
						qstr = qstr->next;
					else{
						if( (pstr->str_start >= qstr->str_start) && ( pstr->str_start+p->char_depth <= qstr->str_start+q->char_depth ) ){
							if( pre == NULL ){
								pre = pstr;
								p->strings = pstr->next;
								pstr = p->strings;
								free( pre );
								pre = NULL;	
							}
							else{
								pre->next = pstr->next;
								free( pstr );
								pstr = pre->next;
							}
							flag = 1;
							break;
						}
						qstr = qstr->next;
					}
				}
				if( flag != 1 ){
					pre = pstr;
					pstr = pstr->next;
				}
			}
		}
		if( get_stringid_num( tar[i] ) >= sup )
			init_can[(*count)++] = tar[i];
	}
	return 0;
}
*/


int stree_print_path_file( NODE *t, FILE *fp )
{
	NODE *p = t;
	char *ch;
	int i;

	if ( p->parent != p ){
		stree_print_path_file( p->parent, fp );
		for( i = 0, ch = t->start_char; i < p->edgelen; i++, ch++ )
			fprintf( fp, "%c", *ch );
	}
	return 0;
}




