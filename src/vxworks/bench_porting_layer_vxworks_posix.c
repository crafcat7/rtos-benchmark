/*
 * Copyright (c) 2023 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include "bench_api.h"

static pthread_t       g_bench_threads[CONFIG_RTOS_BENCHMARK_MAXTHREADS];
static sem_t           g_bench_semaphores[CONFIG_RTOS_BENCHMARK_MAXSEMAPHORES];
static pthread_mutex_t g_bench_mutex[CONFIG_RTOS_BENCHMARK_MAXMUTEXES];

void bench_test_init(void (*test_init_function)(void *))
{
	test_init_function(NULL);
}

static int map_prio(int prio)
{
	/*
	 * The test cases assume smaller numbers are higher priorities,
	 * this is same as VxWorks native priority numbering scheme, but not
	 * true for VxWorks POSIX priority numbering scheme.
	 * So use native priority number (rtpMainPri) for test cases, and
	 * convert the priority here for POSIX scheduling.
	 */

	return PX_VX_PRIORITY_CONVERT(prio);
}

void bench_thread_set_priority(int priority)
{
	pthread_setschedprio(pthread_self(), map_prio(priority));
}

int bench_thread_create(int thread_id, const char *thread_name,
	int priority, void (*entry_function)(void *), void *args)
{
	/*
	 * This function need to be implemented, as it is called by several tests.
	 * No thread create without start function, use spawn instead, which does
	 * not impact the logic of the tests.
	 */

	return bench_thread_spawn(thread_id, thread_name,
			priority, entry_function, args);
}

int bench_thread_spawn(int thread_id, const char *thread_name,
	int priority, void (*entry_function)(void *), void *args)
{
	struct sched_param param;
	pthread_attr_t     attr;
	int                policy;
	int                ret;

	pthread_attr_init(&attr);
	pthread_getschedparam(0, &policy, &param);
	param.sched_priority = map_prio(priority);
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setname(&attr, (char *)thread_name);

	pthread_attr_setschedparam(&attr, &param);

	ret = pthread_create(&g_bench_threads[thread_id], &attr,
			(void *(*)(void *))entry_function, args);

	pthread_attr_destroy (&attr);
	return ret;
}

void bench_thread_start(int thread_id)
{
}

void bench_thread_resume(int thread_id)
{
}

void bench_thread_suspend(int thread_id)
{
}

void bench_thread_abort(int thread_id)
{
	pthread_cancel(g_bench_threads[thread_id]);
}

void bench_thread_exit(void)
{
	pthread_exit(NULL);
}

void bench_yield(void)
{
	pthread_yield();
}

int bench_sem_create(int sem_id, int initial_count, int maximum_count)
{
	int ret;

	ret = sem_init(&g_bench_semaphores[sem_id], 0, initial_count);

	return ret;
}

void bench_sem_give(int sem_id)
{
	sem_post(&g_bench_semaphores[sem_id]);
}

void bench_sem_give_from_isr(int sem_id)
{
}

int bench_sem_take(int sem_id)
{
	int ret;

	ret = sem_wait(&g_bench_semaphores[sem_id]);

	return ret;
}

int bench_mutex_create(int mutex_id)
{
	pthread_mutexattr_t attr;
	int                 ret;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
	ret = pthread_mutex_init(&g_bench_mutex[mutex_id], &attr);
	pthread_mutexattr_destroy(&attr);

	return ret;
}

int bench_mutex_lock(int mutex_id)
{
	return pthread_mutex_lock(&g_bench_mutex[mutex_id]);
}

int bench_mutex_unlock(int mutex_id)
{
	return pthread_mutex_unlock(&g_bench_mutex[mutex_id]);
}

void * bench_malloc(size_t size)
{
	return malloc(size);
}

void bench_free(void * ptr)
{
	free(ptr);
}
