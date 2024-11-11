/* CSE 536: User-Level Threading Library */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/ulthread.h"

/* Standard definitions */
#include <stdbool.h>
#include <stddef.h>

struct ulthread ulthreads[MAXULTHREADS];
struct ulthread *current_thread;
struct ulthread *scheduler_thread;

enum ulthread_scheduling_algorithm scheduling_algorithm;
int last_yieled_thread_id = -1;

/* Get thread ID */
int get_current_tid(void)
{
    return current_thread->thread_id;
}

int find_next_available_thread(void)
{
    // starting from 1 because, 0 is reserved for scheduler thread
    // refer init function for more details
    for (int i = 1; i < MAXULTHREADS; i++)
    {
        if (ulthreads[i].state == FREE)
        {
            return i;
        }
    }
    return -1;
}

int round_robin_scheduling(void)
{
    int thread_id = get_current_tid() + 1;
    int threads_processed = 0;

    while (threads_processed < MAXULTHREADS)
    {
        if (thread_id != 0 && ulthreads[thread_id].state == RUNNABLE)
        {
            return thread_id;
        }
        thread_id = (thread_id + 1) % MAXULTHREADS;
        threads_processed++;
    }
    return 0; // no thread found so returning back 0 - scheduler thread to run
}

int priority_based_scheduling(void)
{
    int selected_thread_id = 0;
    int max_priority = -1;

    for (int i = 1; i < MAXULTHREADS; i++)
    {
        if (ulthreads[i].state == RUNNABLE)
        {
            if (ulthreads[i].priority > max_priority)
            {
                selected_thread_id = i;
                max_priority = ulthreads[i].priority;
            }
        }
    }
    return selected_thread_id; // if no thread again would return 0 by default (scheduler thread)
}

int first_come_first_serve_scheduling(void)
{
    int selected_thread_id = 0;
    int min_last_time_shceduled = ctime(); // intializing with current cpu time

    for (int thread_id = 1; thread_id < MAXULTHREADS; thread_id++)
    {
        if (ulthreads[thread_id].state == RUNNABLE)
        {
            if (ulthreads[thread_id].last_time_shceduled < min_last_time_shceduled)
            {
                selected_thread_id = thread_id;
                min_last_time_shceduled = ulthreads[thread_id].last_time_shceduled;
            }
        }
    }

    return selected_thread_id; // if no thread again would return 0 by default (scheduler thread)
}

/* Thread initialization */
void ulthread_init(int schedalgo)
{
    scheduler_thread = &ulthreads[0]; // will use the first thread with ID 0 as the scheduler thread
    scheduler_thread->thread_id = 0;
    scheduler_thread->state = RUNNABLE;
    scheduler_thread->stackPointer = malloc(USTACKSIZE);
    memset(&scheduler_thread->context, 0, sizeof(scheduler_thread->context));
    scheduler_thread->context.sp = scheduler_thread->stackPointer + USTACKSIZE;
    scheduler_thread->context.ra = (uint64)ulthread_schedule;
    current_thread = scheduler_thread;
    scheduling_algorithm = schedalgo;
}

/* Thread creation */
bool ulthread_create(uint64 start, uint64 stack, uint64 args[], int priority)
{

    int next_available_thread = find_next_available_thread();

    if (next_available_thread == -1)
    {
        printf("No threads available for creation");
        return false;
    }
    /* Please add thread-id instead of '0' here. */
    printf("[*] ultcreate(tid: %d, ra: %p, sp: %p)\n", next_available_thread, start, stack);

    struct ulthread *ul_thread = &ulthreads[next_available_thread];
    ul_thread->thread_id = next_available_thread;
    ul_thread->state = RUNNABLE;
    ul_thread->stackPointer = stack;
    ul_thread->priority = priority;
    ul_thread->last_time_shceduled = ctime();
    memset(&ul_thread->context, 0, sizeof(ul_thread->context));

    // setting return address and stack pointer for the thread
    ul_thread->context.sp = stack;
    ul_thread->context.ra = start;

    // copying the arguments intot the args registers a0-a7
    ul_thread->context.a0 = args[0];
    ul_thread->context.a1 = args[1];
    ul_thread->context.a2 = args[2];
    ul_thread->context.a3 = args[3];
    ul_thread->context.a4 = args[4];
    ul_thread->context.a5 = args[5];

    // ignoring these registers as they are out of scope as we currently have only atmax 6 arguments
    // ul_thread->context.a6 = args[6];
    // ul_thread->context.a7 = args[7];
    return true;
}

/* Thread scheduler */
void ulthread_schedule(void)
{

    // loops forever until it finds the next available thread
    for (;;)
    {
        int selected_thread_id;
        switch (scheduling_algorithm)
        {
        case ROUNDROBIN:
            selected_thread_id = round_robin_select();
        case PRIORITY:
            selected_thread_id = priority_select();
        default:
            selected_thread_id = fcfs_select();
        }

        if (selected_thread_id == 0)
        {
            if (last_yieled_thread_id != -1 && ulthreads[last_yieled_thread_id].state == YIELD)
            {
                selected_thread_id = last_yieled_thread_id;
            }
            else
            {
                printf("No thread active or RUNNABLE to be scheduled. Exiting...\n");
                return;
            }
        }

        if (selected_thread_id == last_yieled_thread_id)
        {
            ulthreads[last_yieled_thread_id].state = RUNNABLE; // marking the last yielded thread as RUNNABLE
        }

        /* Add this statement to denote which thread-id is being scheduled next */
        printf("[*] ultschedule (next tid: %d)\n", selected_thread_id);
        current_thread = &ulthreads[selected_thread_id];

        // Switch between thread contexts
        ulthread_context_switch(&scheduler_thread->context, &current_thread->context);
    }
}

/* Yield CPU time to some other thread. */
void ulthread_yield(void)
{

    /* Please add thread-id instead of '0' here. */
    printf("[*] ultyield(tid: %d)\n", get_current_tid());
    struct ulthread *current_thread = &ulthreads[get_current_tid()];
    current_thread->state = YIELD;
    last_yieled_thread_id = get_current_tid();
    ulthread_context_switch(&current_thread->context, &scheduler_thread->context);
}

/* Destroy thread */
void ulthread_destroy(void) {
    printf("[*] ultdestroy(tid: %d)\n", get_current_tid());
    struct ulthread *current_thread = &ulthreads[get_current_tid()];
    current_thread->state = FREE;
    memset(&current_thread->context, 0, sizeof(current_thread->context));
    ulthread_context_switch(&current_thread->context, &scheduler_thread->context);
}
