#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ltchan.h"

struct _lt_ch_list;
typedef struct _lt_ch_list _lt_ch_list;

struct _lt_ch_list {
	_lt_ch_list		*prev;
	void*			elem;
	_lt_ch_list		*next;
};

struct Channel
{
	lthread_cond_t	*rcond;
	lthread_cond_t	*wcond;

	int				closed;

	size_t			elemsize;

	unsigned int	bufsize;
	unsigned int	cursize;

	_lt_ch_list		*head;
	_lt_ch_list		*tail;
};

Channel*
chancreate(size_t elemsize, unsigned int buffersize)
{
	Channel* chan = calloc(1, sizeof(Channel));
	if (lthread_cond_create(&(chan->rcond)) == -1) {
		// failed to allocate cond create, abort
		fprintf(stderr, "failed to allocate rcond\n");
		abort();
	}
	if (lthread_cond_create(&(chan->wcond)) == -1) {
		fprintf(stderr, "failed to allocate wcond\n");
		abort();
	}

	chan->elemsize = elemsize;
	chan->bufsize = buffersize;

	return chan;
}

void
chanclose(Channel *c)
{
	c->closed = 1;
	lthread_cond_broadcast(c->rcond);
	lthread_cond_broadcast(c->wcond);
	// ^ anything waiting on a channel must now realise it's closed
}

int
chanfree(Channel *c)
{
	if (!c->closed)
		return 0;
	if (c->head != NULL)
		return 0;
	free(c->rcond);
	free(c->wcond);
	free(c);

	return 1;
}

int
channbrecv(Channel *c, void *v)
{
	if (c->closed)
		return LTCHAN_CLOSED;
	if (c->head == NULL)
		return LTCHAN_NONE;

	_lt_ch_list *head = c->head;

	memmove(v, head->elem, c->elemsize);

	if (c->head == c->tail) { // one element left
		c->head = NULL;
		c->tail = NULL;
	} else {
		c->head = head->next;
		c->head->prev = NULL;
	}

	c->cursize--;
	free(head->elem);
	free(head);

	// signal to wcond that we just freed an element
	lthread_cond_broadcast(c->wcond);

	return 0;
}

void*
channbrecvp(Channel *c)
{
	void* pointer = NULL;
	channbrecv(c, &pointer);
	return pointer;
}

unsigned long
channbrecvul(Channel *c)
{
	unsigned long value;
	channbrecv(c, &value);
	return value;
}

int
chanrecv(Channel *c, void *v)
{
	while (1)
	{
		int result = channbrecv(c, v);
		if (result != LTCHAN_NONE)
			return result;

		lthread_cond_wait(c->rcond, 0);
	}
}

void*
chanrecvp(Channel *c)
{
	void* pointer = NULL;
	chanrecv(c, &pointer);
	return pointer;
}

unsigned long
chanrecvul(Channel *c)
{
	unsigned long value;
	channbrecv(c, &value);
	return value;
}

int
_chansend(Channel *c, void *v, int block)
{
	if (c->closed)
		return LTCHAN_CLOSED;

	// special condition for 0
	// if the buffer size is 0 (synchronous), then we will go over the 
	// the buffer limit regardless... (and block later until chan receieves it)
	if (c->bufsize != 0)
	{
		while (c->cursize >= c->bufsize)
		{
			if (block) {
				lthread_cond_wait(c->wcond, 0);
			} else {
				return LTCHAN_FULL;
			}
		}
	}
	
	// check again in case we close the channel
	if (c->closed)
		return LTCHAN_CLOSED;

	_lt_ch_list *elem = malloc(sizeof(_lt_ch_list));
	elem->prev = c->tail;
	elem->elem = malloc(c->elemsize);
	memmove(elem->elem, v, c->elemsize);
	elem->next = NULL;

	if (c->head == NULL) { // no elements
		c->head = elem;
		c->tail = elem;
	} else {
		c->tail->next = elem;
		c->tail = elem;
	}

	c->cursize++;

	// let readers know that we've just inserted an element
	lthread_cond_broadcast(c->rcond);

	// if buffer is overfull and we're blocking, block
	// (special condition for 0)
	if (block)
	{
		while (c->cursize > c->bufsize)
		{
			lthread_cond_wait(c->wcond, 0);
		}
	}

	return 0;
}

int
channbsend(Channel *c, void *v)
{
	return _chansend(c, v, 0);
}

int
channbsendp(Channel *c, void *v)
{
	return channbsendp(c, &v);
}

int
channbsendul(Channel *c, unsigned long v)
{
	return channbsend(c, &v);
}

int
chansend(Channel *c, void *v)
{
	return _chansend(c, v, 1);
}

int
chansendp(Channel *c, void *v)
{
	return chansendp(c, &v);
}

int
chansendul(Channel *c, unsigned long v)
{
	return chansend(c, &v);
}