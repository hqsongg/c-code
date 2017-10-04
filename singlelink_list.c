/*
 * simple single link list implementation
 * author: huangqs 
 * time: 2017.10.3
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 5

typedef struct  _link_node_ {
	int data;
	struct _link_node_ *next;
}lnode_t;

typedef struct _link_list_ {
	lnode_t *head;
	int len;
	int max_size;
} slist_t;

slist_t *slist_init(int max_size);
lnode_t *link_node_create(int data);
int slist_insert(slist_t *plist, lnode_t *pnode);
int slist_delete(slist_t *plist, int target);
void slist_show(slist_t *plist);
void slist_reverse(slist_t *plist);
lnode_t *slist_get_entry_form_head(slist_t *plist);

int main()
{
	slist_t *list;
	int i;
	
	list = slist_init(MAX_SIZE);
	
	for(i=0; i<6; i++)
	{
		if(slist_insert(list, link_node_create(i)) != 0)
		{
			printf("Error: call slist_insert \n");
			//return -1;
		}
	}
	slist_show(list);
//	slist_delete(list, 0);
//	slist_show(list);
	slist_reverse(list);
	slist_show(list);
	
	return 0;
}

slist_t *slist_init(int max)
{
	slist_t *plist;

	plist = malloc(sizeof(slist_t));
	plist->len = 0;
	plist->max_size = max;
	plist->head = NULL;

	return plist;
}

lnode_t *link_node_create(int data)
{
	lnode_t *p;

	p = malloc(sizeof(lnode_t));
	p->data = data;
	p->next = NULL;

	return p;
}

int slist_insert(slist_t *plist, lnode_t *pnode)
{
	if((plist == NULL) || (plist->len >= plist->max_size))
	{
		printf("%s:%d: slink is full !\n",__FUNCTION__,__LINE__);
		return -1;
	}

	pnode->next = plist->head;
	plist->head = pnode;
	plist->len++;

	return 0;
}

int slist_delete(slist_t *plist, int target)
{
	lnode_t *p = plist->head;
	lnode_t *t;

	if(plist->len == 0)
	{
		return -1;
	}

	if(p->data == target)
	{
		plist->head = plist->head->next;
		plist->len--;
		free(p);

		return 0;
	}
	while(p->next != NULL)
	{
		if(p->next->data == target)
		 {
			t = p->next;
			p->next = t->next;
			plist->len--;
			printf("%s:%d:delete data:%d \n",__FUNCTION__,__LINE__,t->data);
			free(t);

			return 0;
		 } 
		p = p->next;
	}

	return -1;
}

lnode_t *slist_get_entry_from_head(slist_t *plist)
{
	lnode_t *p = NULL;
	
	p = plist->head;
	if(p != NULL)
	{
		//printf("p->data:%d \n", p->data);
		plist->head = p->next;
	} 

	return p;
}

void slist_show(slist_t *plist)
{
	int i = 0;
	lnode_t *p = plist->head;

	printf("slist len:%d, max_size:%d\n", plist->len, plist->max_size);
	while(p != NULL)
	{
		printf("data[%d]:%d \n",i++, p->data);
		p = p->next;
	}

	return;
}

#if 0
void slist_reverse(slist_t *plist)
{
	lnode_t *p, *q;
	
	q = malloc(sizeof(lnode_t)); 
	q->next = NULL;

	p = slist_get_entry_from_head(plist);
	while(p != NULL)
	{
		p->next = q->next;
		q->next = p;
		p = slist_get_entry_from_head(plist);
	}

	plist->head = q->next;
	free(q);

	return;
}
#endif 


void slist_reverse(slist_t *plist)
{
	lnode_t *head, *p, *q;

	if(plist == NULL || plist->head == NULL)
	{
		printf("[%s:%d] involid arguments !\n",__FUNCTION__,__LINE__);
	}

	head = plist->head;
	p = head->next;
	head->next = NULL;

	while(p != NULL)
	{
		printf("data:%d\n",head->data);
		q = p->next;
		p->next = head;
		head = p;
		p = q;
	}
	plist->head = head;

	return;
}

