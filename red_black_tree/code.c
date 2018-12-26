#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>  /* error codes */
#include <linux/sched.h> /* task states (TASK_INTERRUPTIBLE and friends ) */
#include <linux/mm.h> /* find_vma */
#include <linux/mm_types.h> /* vm_area_struct */
#include <linux/workqueue.h>
#include <linux/delay.h> /* msleep() */
#include <linux/time.h> 
#include <linux/wait.h> /* wait_event */
#include <linux/rbtree.h> /* rbtree */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Red Black Tree");
MODULE_VERSION("110");
MODULE_AUTHOR("Arshad Hussain <arshad.super@gmail.com>");

#define element 14

/*
 * ########################################
 * TODO: Still incomplete. Work in progress
 * ########################################
 *
 * Red Black Tree
 *
 * Red Black Tree or rbtree is a binary search tree where:-
 * 	Rule 1: Every node(rb_node) is either Red or Black
 * 	Rule 2: The root(rb_root) node is always black.
 * 	Rule 3: Children of Red node is Black
 * 	Rule 4: Every path from root to leaf node has the same
 * 		number of black nodes.
 *
 * Recoloring
 * 	Helps solve problem of double red or Rule 3 is broken.
 * 
 * Rebalancing
 * 	Is done when red child's red parent sibling is NULL or black
 *
 * Lookup
 * 	They are same as BST. Colours do not matter here.
 *
 * Insertion
 * 	Insertion is done using the same law as BST
 * 	Inserted node is always red.
 * 	Therefore, if the parent is red, we face the double red
 * 	problem (Rule 3) and this is corrected with rebalancing 
 * 	and then followed by recoloring if necessary
 *
 * 
 * Linux kernel implementation of Red black tree.
 *
 * 01.	First, to turn any data structure into red black tree, 
 * 	embed 'struct rb_node' into it. See example struct TREE below.
 * 02.	There is not search and insert function provided by the API.
 * 	Search and insert must be coded by the user.
 * 03.	struct rb_root defines the root of red black tree.
 * 04.	All nodes are represented as struct rb_node
 * 05.	rb_first() returns the first entry in the tree.
 * 06.	rb_next() subsequently returns the next element until all nodes
 * 	are traversed.
 * 07.	rb_entry() - Helps find struct rb_node
 * 08.	rb_link_node() - TODO
 * 09.	rb_insert_color() - TODO
 * 10.	rb_erase() - TODO
 * 11.	All these red black calls are included in linux/rbtree.h
 *
 * Sample Data structures used in rbtree is below:-
 *
 * 		(rb_root) -> [ROOT]
 * 	                    /	   \
 * 		           /	    \
 *   (rb_node)---> [left child]	  [right child] <---(rb_node)
 * 	                /    |	      /       \
 * 	               /     |	   [NULL]      \
 *                    /      |              [right child] <---(rb_node)
 *                   /    [NULL]
 *                  /
 *   (rb_node)---> [left child]	       
 *
 */

struct TREE
{
	int data;
	/*
	 * Embed "struct rb_node" to turn it into 
	 * Red Black Tree
	 */
	struct rb_node node;
};

int insert(struct TREE* , struct rb_root* root);
struct TREE* search(struct rb_root* root, int value);
void print(struct rb_root *root);

/*
 * rb_root points to the root of red black tree.
 * And is initialized to NULL/Empty using RB_ROOT
 */
struct rb_root root = RB_ROOT;

static int __init start(void)
{
	struct TREE tree[element];
	struct TREE *t;
	int i, search_data;


	pr_info("Start...!\n");

	for(i=0; i<element; i++) {
		tree[i].data = i * 100;	
		tree[i].node.rb_left = NULL;
		tree[i].node.rb_right = NULL;
	}
	
	for(i=0; i<element; i++) {
		insert(&tree[i], &root);
	}

	search_data = 200;
	t = search(&root, search_data);
	if(!t) {
		pr_info("Data %d Not found \n", search_data);
	} else {
		pr_info("Data %d found \n", t->data);
	}

	/* print */
	print(&root);

	return 0;
}

static void __exit stops(void) {
	printk(KERN_INFO "Goodbye!\n");
}

void print(struct rb_root *root)
{
	struct rb_node *tmp;

	tmp = rb_first(root);

	while (tmp) {
		/*
		 * rb_entry() - Uses container_of() to find
		 * rb_node within the structure it is embedded
		 * into.
		 * Eg, rb_entry() will find "struct rb_node"
		 * from within "struct TREE"
		 */
		struct TREE *t = rb_entry(tmp, struct TREE, node);
		pr_info("Data = %d\n", t->data);
		tmp = rb_next(tmp);
	}

}

struct TREE* search(struct rb_root* root, int value)
{
	struct rb_node *n = root->rb_node;

	while(n)
	{
		struct TREE *tmp_tree = rb_entry(n, struct TREE, node);

		if (tmp_tree->data > value)
			n = n->rb_left;
		else if (tmp_tree->data < value)
			n = n->rb_right;
		else
			return tmp_tree;
	}

	return NULL;
}

int insert(struct TREE *t, struct rb_root *root)
{
	struct rb_node *parent;
	struct rb_node *tmp;
	struct TREE *tmp_tree;

	if(!root->rb_node) {
		root->rb_node = &t->node;
		return 0;
	}

	tmp = root->rb_node;
	while(tmp) {
		parent = tmp;
		tmp_tree = rb_entry(tmp, struct TREE, node);

		if(tmp_tree->data > t->data)
			tmp = parent->rb_left;
		else if (tmp_tree->data < t->data)
			tmp = parent->rb_right;
		else
			break;
	}

	/* find duplicate data */
	if (tmp)
	{
		pr_info("Data Already present in tree\n");
		return 1;
	}

	/* now insert */
	if (tmp_tree->data > t->data)
		parent->rb_left = &t->node;
	else
		parent->rb_right = &t->node;

	/* Re-balance TODO*/
	//rb_insert_color(&t->node, root);
	
	return 0;
}

module_init(start);
module_exit(stops);

