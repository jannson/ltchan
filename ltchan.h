#ifndef _LTCHAN_H_
#define _LTCHAN_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <lthread.h>

#define LTCHAN_NONE -1
#define LTCHAN_CLOSED -2
#define LTCHAN_FULL -3

struct Channel;
typedef struct Channel Channel;

// Create a channel. The channel will be filled with elements of size
// 'elemsize' and will hold at most 'buffersize' elements before blocking.
// (0 means block immediately for processing.)
Channel*	chancreate(size_t elemsize, unsigned int buffersize);

// Close a channel. No more elements will be accepted.
void		chanclose(Channel *c);

// Free a channel. Fails if there are still elements within.
int			chanfree(Channel *c);

// Receive elements.
// Non-blocking. [false for no element, true for has element]
int				channbrecv(Channel *c, void *v);
void*			channbrecvp(Channel *c);
unsigned long	channbrecvul(Channel *c);

// Blocking. Waits for an element.
int				chanrecv(Channel *c, void *v);
void*			chanrecvp(Channel *c);
unsigned long	chanrecvul(Channel *c);

// Send elements.
// Non-blocking. [false if the buffer is full (another lthread is waiting)]
int				channbsend(Channel *c, void *v);
int				channbsendp(Channel *c, void *v);
int				channbsendul(Channel *c, unsigned long v);

// Blocking. If the buffer is full, waits until a slot opens.
int				chansend(Channel *c, void *v);
int				chansendp(Channel *c, void *v);
int				chansendul(Channel *c, unsigned long v);

#ifdef __cplusplus
}
#endif
#endif