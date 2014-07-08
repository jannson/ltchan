#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ltchan.h"

struct LtLock {
	lthread_cond_t	*c;
	int				locked;
	uint64_t		lthread;
};

LtLock*
ltlockcreate()
{
	LtLock *lock = malloc(sizeof(LtLock));

	if (lthread_cond_create(&(lock->c)) == -1) {
		// yikes.
		return NULL;
	}

	lock->locked = 0;

	return lock;
}

void
ltlock(LtLock *l)
{
	while(l->locked)
		lthread_cond_wait(l->c, 1000);
	l->locked = 1;
	l->lthread = lthread_id();
}

int
ltchecklock(LtLock *l)
{
	return !l->locked;
}

void
ltunlock(LtLock *l)
{
	if (!l->locked)
		return;

	if (l->lthread != lthread_id()) {
		fprintf(stderr, "tried to unlock on %lu when thread was locked by %lu\n", lthread_id(), l->lthread);
	}
	l->locked = 0;
	lthread_cond_broadcast(l->c);
}