/*
 * Atomic ticketlock-based spinlock for lolz, inspired by Linux kernel's
 * own fancier ticket lock
 *
 * Works on x86 or x64 architecture but use gcc, okay? (b/c gcc inline
 * assembly for atomic operations)
 *
 * Wat is ticket lock?
 * You own a store. Every customer gets a unique ticket, a number. When
 * their number is called, they come to you and do their business things
 * with you. When they're done, you call out the next number. All
 * customers not currently being served by you spin around in circles to
 * cure their boredom.
 *
 * Locks prevent race conditions. Ticket lock is a fair, first-come
 * first-serve lock. A process trying to acquire the lock is a customer
 * trying to get a hold of you. Analogy follows.
 */

/*
 * Usage notes:
 *
 * Use "ticketlock_t mylock;" to declare your spinlock
 *
 * Initialize your spinlock like "mylock = TICKETLOCK_UNLOCKED;"
 * or "mylock = TICKETLOCK_LOCKED;" accordingly
 *
 * Call "ticketlock_lock(&mylock);" to try to acquire the lock
 * ticketlock_lock will spin (infinite loop) and will not return until
 * lock is acquired
 *
 * Call "ticketlock_unlock(&mylock);" to release the lock
 */

#ifndef _TICKETLOCK_H_
#define _TICKETLOCK_H_

typedef struct ticketlock_struct {
	int queue;
	int dequeue;
} ticketlock_t;
#define TICKETLOCK_LOCKED (ticketlock_t){.queue = 1, .dequeue = 0}
#define TICKETLOCK_UNLOCKED (ticketlock_t){.queue = 0, .dequeue = 0}

/* try to acquire lock, critical part is atomic */
static inline void ticketlock_lock(ticketlock_t *t)
{
	__asm__ __volatile__("movl $1, %%eax\n\t"
			"lock xaddl %%eax, %[q]\n\t"
			"1:\n\t"
			"cmpl %%eax, %[d]\n\t"
			"jne 1b"
			: [q] "+m" (t->queue)
			: [d] "m" (t->dequeue)
			: "cc", "eax");
}

/* release lock atomically */
static inline void ticketlock_unlock(ticketlock_t *t)
{
	__asm__ __volatile__("lock incl %[d]"
			: [d] "+m" (t->dequeue)
			:
			: "cc");
}

#endif /* !_TICKETLOCK_H_ */
