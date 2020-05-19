#include "threads.h"

#include "stdio.h"

#include "sx/allocator.h"
#include "sx/array.h"
#include "sx/threads.h"

#include "node_api.h"

int thread_cb(void *user_data1, void *user_data2)
{
    struct thread_note *thread_note = user_data1;

    while (true)
    {
        // sx_signal_wait(thread_note->signal, -1);
        sx_mutex_lock(thread_note->mutex_in);

        // sx_os_sleep(100);
        thread_note->func_to_run(thread_note->node_to_run);
        // puts("asd");

        thread_note->node_to_run = NULL;
        // sx_signal_raise(thread_note->signal_done);
        sx_mutex_unlock(thread_note->mutex_out);
        // sx_thread_yield();
    }
}

#if SX_PLATFORM_POSIX
// Redefine sx_mutex_init with PTHREAD_MUTEX_TIMED_NP
// to enable cross thread lock unlock
#include <pthread.h>

typedef struct sx__mutex_s
{
    pthread_mutex_t handle;
} sx__mutex;

void sx_mutex_init_(sx_mutex *mutex)
{
    sx__mutex *_m = (sx__mutex *)mutex->data;

    pthread_mutexattr_t attr;
    int r = pthread_mutexattr_init(&attr);
    sx_assert(r == 0);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_TIMED_NP);
    r = pthread_mutex_init(&_m->handle, &attr);
    sx_assert(r == 0 && "pthread_mutex_init failed");
    sx_unused(r);
}
#endif // SX_PLATFORM_POSIX

void init_threads_subsystem()
{
    // TODO: default_thread_note will be main thread...
    default_thread_note.mutex_in = malloc(sizeof(sx_mutex));
    default_thread_note.mutex_out = malloc(sizeof(sx_mutex));
    default_thread_note.signal = malloc(sizeof(sx_signal));
    default_thread_note.signal_done = malloc(sizeof(sx_signal));

#if SX_PLATFORM_POSIX
    sx_mutex_init_(default_thread_note.mutex_in);
    sx_mutex_init_(default_thread_note.mutex_out);
#else
    sx_mutex_init(default_thread_note.mutex_in);
    sx_mutex_init(default_thread_note.mutex_out);
#endif
    sx_signal_init(default_thread_note.signal);
    sx_signal_init(default_thread_note.signal_done);
    sx_mutex_lock(default_thread_note.mutex_in);
    default_thread_note.thread = sx_thread_create(
        sx_alloc_malloc(),
        thread_cb,
        &default_thread_note,
        0,
        "user-thread-c",
        NULL);

    sx_array_push(sx_alloc_malloc(),
                  thread_notes, &default_thread_note);
}

struct thread_note *make_thread_note()
{
    struct thread_note *thread_note = malloc(sizeof(struct thread_note));

    thread_note->mutex_in = malloc(sizeof(sx_mutex));
    thread_note->mutex_out = malloc(sizeof(sx_mutex));
    thread_note->signal = malloc(sizeof(sx_signal));
    thread_note->signal_done = malloc(sizeof(sx_signal));

#if SX_PLATFORM_POSIX
    sx_mutex_init_(thread_note->mutex_in);
    sx_mutex_init_(thread_note->mutex_out);
#else
    sx_mutex_init(thread_note->mutex_in);
    sx_mutex_init(thread_note->mutex_out);
#endif
    sx_signal_init(thread_note->signal);
    sx_signal_init(thread_note->signal_done);
    sx_mutex_lock(thread_note->mutex_in);
    thread_note->thread = sx_thread_create(
        sx_alloc_malloc(),
        thread_cb,
        thread_note,
        0,
        "user-thread-c",
        NULL);

    sx_array_push(sx_alloc_malloc(), thread_notes, thread_note);

    return thread_note;
}

void free_thread_note(struct thread_note *thread_note)
{
    sx_thread_destroy(thread_note->thread, sx_alloc_malloc());
    sx_mutex_release(thread_note->mutex_in);
    sx_mutex_release(thread_note->mutex_out);
    sx_signal_release(thread_note->signal);
    sx_signal_release(thread_note->signal_done);
    free(thread_note->signal);
    free(thread_note);
}

void send_func_to_thread(func_for_node *func_to_run, struct node *node)
{
    // sx_mutex_lock(node->thread_note->mutex);

    node->thread_note->func_to_run = func_to_run;
    node->thread_note->node_to_run = node;
    // sx_signal_raise(node->thread_note->signal);

    // puts("send >>>");
    // WARNING: works only with sx/threads.c:296 patch     pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    sx_mutex_lock(node->thread_note->mutex_out);
    sx_mutex_unlock(node->thread_note->mutex_in);

    // sx_thread_yield();
    sx_mutex_lock(node->thread_note->mutex_out);
    sx_mutex_unlock(node->thread_note->mutex_out);
    // puts("received <<<");
}