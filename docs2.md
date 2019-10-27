EE 415 Project2 Design docs
=============

## Group
- Sunghwan Kim <sunghwankim@kaist.ac.kr>
- Yunseok Choi <dbstjr@kaist.ac.kr>

## Preliminaries
### Extra point 
> We implement advanced scheduling in multiple queue.
### Referenced source
- https://web.stanford.edu/class/cs140/projects/pintos/pintos.pdf

# Alarm Clock
## DATA STRUCTURE
<pre>src/threads/thread.h @@ struct thread 
<code>int 64_t timer ticks; </code></pre>
- Define timer_ticks in struct thread to count the number of ticks

<pre>src/devices/timer.c 
<code>static struct list sleep_list;</code></pre>
- Add sleep_list to insert thread state sleep
## ALGORITHMS 
<pre>src/devices/timer.c  
@@ void timer_sleep (int64_t ticks) 
<code>
ASSERT (intr_get_level () == INTR_ON);
enum intr_level old_level = intr_disable ();

int64_t start = timer_ticks ();
struct thread *t = thread_current();
t->timer_ticks = start + ticks;

list_insert_ordered(&sleep_list, &(t->elem) , less_ticks_func, NULL);
thread_block();
intr_set_level(old_level);</code></pre>

- In **timer_sleep** function, first have to disable interrupt. Then, we have to count the number of ticks of current thread, and save it to **timer_ticks** element in struct.  Next, insert sleep in sleep_list using list_insert_ordered function. We implemented **less_ticks_func** to compare the number of ticks. Then call thread_block function and able interrupt

<pre>src/devices/timer.c  
@@ void timer_interrupt(struct intr_frame *args UNUSED)
<code>
struct thread *t;
ticks++;
  
struct list_elem *e = list_begin(&sleep_list);
while (e != list_end(&sleep_list)){
    t = list_entry(e, struct thread, elem);      
    if (ticks < t->timer_ticks) break;

    list_remove(e); 
    thread_unblock(t); 
    e = list_begin(&sleep_list);
}</code></pre>

- In **timer_interrupt** fuction, do the following opertion for every element in sleep_list. If the number of time ticks is greater or equal than global ticks, remove the element and unblock the thread.

## RATIONALE 
### Advantages
 - All tests passed
### Disadvantages
 - Chance to make a wake up function for refactoring

# Priority Scheduling
## DATA STRUCTURE
<pre>src/thread/thread.h
@@ struct thread

<code>
    /* pj1 */
    struct list holding_locks;          /* locks that this thread holds. */
    int original_priority;              /* priority with no donations */
    struct lock *waiting_lock;          /* lock that this thread waits */
</code></pre>
- we added some variables in thread structure.
- we should keep own donation for recover this.

<pre>src/thread/synch.h
@@ struct semaphore
<code>
struct semaphore 
  {
    unsigned value;             /* Current value. */
    struct list waiters;        /* List of waiting threads. */
    int priority;               /* pj1: Semaphore priority for cond */
  };
</code>
@@ struct lock
<code>
struct lock 
  {
    struct thread *holder;      /* Thread holding lock (for debugging). */
    struct semaphore semaphore; /* Binary semaphore controlling access. */
    
    /* pj1 */
    struct list_elem elem;      /* lock could be a member of list */
    int priority;               /* priority of holder */
  };
</code></pre>
- we also add variable in lock and semaphore to synchronize priority with the thread.

## ALGORITHMS 
<pre>src/thread/thread.c
<code>
void
thread_unblock (struct thread *t) 
{
  enum intr_level old_level;

  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);

  /* not just push back the thread to ready_list but insert with priority order */
  // list_push_back (&ready_list, &t->elem);
  if (thread_mlfqs)
    list_push_back(&mlfqs_list[t->priority], &t->elem);
  else
    list_insert_ordered (&ready_list, &t->elem, cmp_priority, NULL);

  t->status = THREAD_READY;
  intr_set_level (old_level);
}
</code></pre>
- we should run thread for priority order so when we push thread into the ready_list we used *list_insert_ordered* to make list ordered.
- So as in *thread_yield()*
- not in multiple queue case.

<pre>src/thread/thread.c
<code>tid_t
thread_create (const char *name, int priority,
               thread_func *function, void *aux) 
{
  ...

  /* if creating thread has higher priority than current's than yield */
  /* we should go to yield and modify! */
  if (priority > thread_current ()->priority)
    thread_yield ();

  return tid;
}
</code></pre>
- When we create new thread, we also should compare priority with current thread.
- If it has more priority then current one, yield CPU.
- So as in sema_up case 
- Also when we call set_priority compare with biggest priority in ready_list.
### Here we finished for priority based ready_list. But we should synchronize with lock too!
<pre>src/thread/synch.c
<code>
void
sema_down (struct semaphore *sema) 
{
  enum intr_level old_level;

  ASSERT (sema != NULL);
  ASSERT (!intr_context ());

  old_level = intr_disable ();
  while (sema->value == 0) 
    {
      list_insert_ordered (&sema->waiters, &thread_current ()->elem, 
                      cmp_priority, NULL);
      thread_block ();
    }
  sema->value--;
  intr_set_level (old_level);
}
</code></pre>
- similar with ready_list case, sema_waiters should be ordered with priority.

### But we also consider the Priority Inversion!!, To solve that, we should make new function, donation and, donation backward. They are shown below.
<pre>src/thread/synch.c
<code>
void
priority_donation (struct lock *lock)
{
  struct thread *curr = thread_current ();
  struct thread *lock_holder = lock->holder;
  if (curr->priority > lock_holder->priority)
  {
    thread_donate_priority (lock_holder, curr->priority);
    lock->priority = lock_holder->priority;
    
    if (lock_holder->waiting_lock != NULL)
    {
      priority_donation (lock_holder->waiting_lock);
    }
  }
}
</code>
src/thread/thread.c
<code>
void
thread_donate_priority (struct thread *t, int new_priority)
{
  t->priority = new_priority;

  /* if new < max_prior in ready_list, yield CPU */
  if (!list_empty (&ready_list))
  {
    list_sort (&ready_list, cmp_priority, NULL);
    struct thread *max_thread = list_entry (list_front (&ready_list), 
                            struct thread, elem);
    if (max_thread->priority > t->priority) {
      thread_yield();
    }
  }
}
</code>
</pre>
- *poriority_donation* donate the priority to the lock holder that the thread wants.
- to make nest priority possible we donate with while loop so that donate until the lock holder have bigger priority then him (donating thread).

<pre>src/thread/synch.c
<code>
void
donation_backward (struct lock *lock)
{
  struct thread *curr = thread_current ();

  list_remove (&lock->elem);

  if (list_empty (&curr->holding_locks))
  {
    thread_donate_priority(curr, curr->original_priority);
  }
  else
  {
    list_sort (&curr->holding_locks, cmp_priority_lock, NULL);
    struct lock *max_lock = list_entry (list_front (&curr->holding_locks),
                          struct lock, elem);
    thread_donate_priority (curr, max_lock->priority);
  }
}
</code></pre>
- to go backward with donations. we make this function!
- if holding_lock is empty, set priority with original_priority.
- or set priority with the maximum priority that donate to this lock hold.

<pre>src/thread/synch.c
<code>
void
cond_wait (struct condition *cond, struct lock *lock) 
{
  struct semaphore_elem waiter;

  ASSERT (cond != NULL);
  ASSERT (lock != NULL);
  ASSERT (!intr_context ());
  ASSERT (lock_held_by_current_thread (lock));
  
  sema_init (&waiter.semaphore, 0);
  waiter.semaphore.priority = thread_current ()->priority; // pj1: for cond
  list_insert_ordered (&cond->waiters, &waiter.elem, cmp_priority_sema, NULL); // pj1
  lock_release (lock);
  sema_down (&waiter.semaphore);
  lock_acquire (lock);
}
</code></pre>
- also for cond_priority, we insert waiters in order of priority of sema. that we declared at *semaphore* structure.
## RATIONALE
### Advantages
 - All tests passed
### Disadvantages
 -  NULL


# Advanced Scheduling
## DATA STRUCTURE
<pre>src/threads/thread.h @@ struct thread
<code>
int recent_cpu;
int nice;</code></pre>
- Define recent_cpu, nice in sttruct thread 
<pre>src/threads/thread.c 
<code>
static struct list mlfqs_list[PRI_MAX+1];
</code></pre>
- We use one ready list for each priority.
## ALGORITHMS
<pre>src/threads/fixed-point.h
<code>
#define F (1 << 14)

#define ADD_FP(X, Y) ((X) + (Y))
#define ADD_INT(X, N) ((X) + (N) * (F))
#define SUB_FP(X, Y) ((X) - (Y))
#define SUB_INT(X, N) ((N) * (F) - (X))
#define MUL_FP(X, Y) ((int64_t)(X) * (Y) / (F))
#define MUL_INT(X, N) ((X) * (N))
#define DIV_FP(X, Y) ((int64_t)(X)  * (F) / (Y))
#define DIV_INT(X, N) ((X) / (N))

#endif
</code></pre>
- We add fixed point header file to use in load average, recent cpu, priority calculation. We use macro for the purpose.

<pre>src/threads/thread.c @@void thread_init(void)
<code>
if (thread_mlfqs){
    int i;
    for (i = PRI_MIN; i <= PRI_MAX; i++)
    list_init(&mlfqs_list[i]);
}else 
    list_init(&ready_list);

</code></pre>
- Since, we use mlfqs_list instead of ready_list in advanced scheduling, we have to use if-else statement in functions using ready_list. For example, in thread_init function, we the list initialzie seperately. We did same thing in function thread_unblock, thread_yield, next_thread_to_run.

<pre>src/threads/thread.c @@void update_mlfqs(void)
<code>
load_avg = DIV_INT(ADD_INT(MUL_INT(load_avg, 59), ready_threads), 60);
</code></pre>
<pre>src/threads/thread.c @@void update_mlfqs(void)
<code>
t->recent_cpu = ADD_INT(MUL_FP(DIV_FP(MUL_INT(load_avg, 2), 
        ADD_INT(MUL_INT(load_avg, 2), 1)), t->recent_cpu), t->nice);
</code></pre>
- In update_mlfqs function we calculate load_avg and recent_cpu of each thread. 


<pre>src/threads/thread.c @@void update_priority(void)
<code>
    t->priority = SUB_FP(SUB_FP(ADD_INT(0, PRI_MAX),
      DIV_INT(t->recent_cpu, 4)), MUL_INT(ADD_INT(0, t->nice),2)) / F;
</code></pre>
- In update_priority function we calculate priority by nice and recent_cpu of thread.

<pre>src/devices/timer.c @@static void timer_interrupt (struct intr_frame *args UNUSED)
    t->recent_cpu = ADD_INT(t->recent_cpu, 1);
    if (timer_ticks() % TIMER_FREQ == 0) update_mlfqs();
    if (timer_ticks() % 4 == 0) update_priority();
</code></pre>
- In timer_interrupt, we add 1 in recent_cpu of thread. Then, every TIMER_FREQ ticks we call update_mlfqs, every 4 ticks we call update_priority.

## RATIONALE 
### Advantages
 - All tests passed
 - We use multiple priority queue to implement advanced scheduling. Thus, operation time is faster.
### Disadvantages
 - Not considering sema_up and sema_down when implementing advanced scheduling.
