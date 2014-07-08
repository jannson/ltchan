# ltchan

A library that provides channels for lthread coroutines. Like libtask has channels for its coroutines.

I don't know what I was thinking, really. The only reason this exists is because I need a coroutine library that can have multiple schedulers on multiple pthreads and channels to send stuff between. None existed. So I made this.

Requires [lthread](https://github.com/halayli/lthread) to be installed on your system.

Then, run `make` and `make install`.

### How to use:

```c
// no element in channel
#define LTCHAN_NONE -1
// channel has closed
#define LTCHAN_CLOSED -2
// channel buffer is full
#define LTCHAN_FULL -3

// Create a channel. The channel will be filled with elements of size
// 'elemsize' and will hold at most 'buffersize' elements before blocking.
// (0 means block immediately for processing.)
Channel*	chancreate(size_t elemsize, int buffersize);

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
```