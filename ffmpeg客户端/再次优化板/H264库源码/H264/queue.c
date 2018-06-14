/*******************************************************************************
 *                  Cross Platform Queue Code
 *			    Copyright GoDB Tech Private Limited.
 *
 *******************************************************************************/

#include <stdio.h>
#ifdef WIN32
	#include <windows.h>
#else
	#include <string.h>
	#include <pthread.h>
	#include <time.h>
	#include <sys/time.h>
#endif


typedef struct gq_elem{
	void*		data;
	int			length;
	struct gq_elem*	next;
}gq_elem;

typedef struct gq_{
	void*		mutex;
	void*		cond;
	long		size;
	long		stop;
	long		avg_wait_time;
	struct gq_elem* first;
	struct gq_elem* last;
}gq;


void* gvp_create_mutex()
{
#ifdef WIN32
CRITICAL_SECTION* pcs = (CRITICAL_SECTION*)malloc( sizeof(CRITICAL_SECTION) );
	memset(pcs, 0, sizeof(CRITICAL_SECTION));
	InitializeCriticalSection(pcs);
	return (void*)pcs;
#else
	pthread_mutex_t *mutx = (pthread_mutex_t *) malloc (sizeof (pthread_mutex_t));
	pthread_mutex_init(mutx, NULL);
	return mutx;
#endif


}
void* gvp_create_condition()
{
#ifdef WIN32
	return CreateEvent( 
		NULL,         // no security attributes
		TRUE,         // manual-reset event
		FALSE,         // initial state is signaled
		"queue"  // object name
		);
#else
	pthread_cond_t *cond = (pthread_cond_t *) malloc (sizeof (pthread_cond_t));
	pthread_cond_init(cond, NULL);
	return cond;
#endif
}



void* gvp_create_new_queue()
{
gq* pq = malloc( sizeof(gq) );
	memset( pq, 0, sizeof(gq) );
	pq->mutex = gvp_create_mutex();
	pq->cond = gvp_create_condition();
	return pq;
}

void gvp_delete_queue(void* pq)
{
gq* p = (gq*)pq;
	if( p->mutex )
	{
		DeleteCriticalSection(p->mutex);
		free( p->mutex );
		p->mutex = 0;
	}
	if( p->cond )
	{
		CloseHandle( p->cond );
		p->cond = 0;
	}

	free( pq );	//?
}

void gvp_lock_queue(void* pq)
{
	if( pq==0 )
		return;
#ifdef WIN32
	EnterCriticalSection( (CRITICAL_SECTION*)((gq*)pq)->mutex );
#else
	pthread_mutex_lock( (pthread_mutex_t*)((gq*)pq)->mutex );
#endif
}
void gvp_unlock_queue(void* pq)
{
	if( !pq )
		return;
#ifdef WIN32
	LeaveCriticalSection( (CRITICAL_SECTION*)((gq*)pq)->mutex );
#else
	pthread_mutex_unlock( (pthread_mutex_t*)((gq*)pq)->mutex );
#endif
}
void gvp_signal_queue(void* pq)
{
#ifdef WIN32
	SetEvent((HANDLE)((gq*)pq)->cond);
#else
	pthread_cond_signal( (pthread_cond_t*)((gq*)pq)->cond );
#endif
}
void gvp_wait_signal_queue(void* pq)
{
#ifdef WIN32
	gvp_unlock_queue( pq );
	WaitForSingleObject((HANDLE)((gq*)pq)->cond,1000);
	//WaitForSingleObject((HANDLE)((gq*)pq)->cond,INFINITE);
	ResetEvent((HANDLE)((gq*)pq)->cond);
	gvp_lock_queue( pq );
#else
	struct timeval	tv;
	struct timespec abstime;
	gettimeofday(&tv, NULL);
	abstime.tv_sec = tv.tv_sec + 2;			//two seconds from now
	abstime.tv_nsec = tv.tv_usec*1000;
	pthread_cond_timedwait( (pthread_cond_t*)((gq*)pq)->cond, (pthread_mutex_t*)((gq*)pq)->mutex, &abstime );
#endif
	
}

int gvp_enqueue(void* pq, void* data, int length)
{
gq_elem* pqelem = malloc( sizeof(gq_elem) );
	memset( pqelem, 0, sizeof(gq_elem) );
	pqelem->data = data;
	pqelem->length = length;
	
	gvp_lock_queue( pq );
	((gq*)pq)->size++;
	if( ((gq*)pq)->first == 0 )
		((gq*)pq)->first = pqelem;
	else
		((gq*)pq)->last->next = pqelem;
	((gq*)pq)->last = pqelem;
	gvp_signal_queue( pq );
	gvp_unlock_queue( pq );
	return 1;
}



int gvp_enqueue_alloc(void* pq, void* data, int length)
{
char* p = malloc( length+1 );
	memcpy(p, data, length);
	return gvp_enqueue( pq, p, length);
}

int gvp_dequeue(void* pq, void** data, int* length, int block)
{
	*data = 0;
	*length= 0;
	if( pq==0 )return 0;
	gvp_lock_queue( pq );
	for( ;; )
	{
		if( ((gq*)pq)->first != 0 )
		{
			void* p = ((gq*)pq)->first;
			((gq*)pq)->size--;
			*data = ((gq*)pq)->first->data;
			*length=((gq*)pq)->first->length;
			((gq*)pq)->first = ((gq*)pq)->first->next;
			free( p );
			break;
		}
		else
		if( block==0 )
		{
			break;
		}
		else
		{
			int wait = GetTickCount();
			gvp_wait_signal_queue( pq );
			((gq*)pq)->avg_wait_time = ((((gq*)pq)->avg_wait_time) + (GetTickCount()-wait))/2;
		}
		if( ((gq*)pq)->stop )
		{
			printf("stopping queue....\n");
			break;
		}
	}
	gvp_unlock_queue( pq );
	return (*length);
}

int gvp_flush_queue(void* pq)
{
void* p;
	if( pq==0 )return 0;
	gvp_lock_queue( pq );
	while( ((gq*)pq)->first != 0 )
	{
		p = ((gq*)pq)->first->next;
		free( ((gq*)pq)->first );
		((gq*)pq)->first = p;
	}
	((gq*)pq)->last = 0;
	((gq*)pq)->size = 0;
	gvp_unlock_queue( pq );
	return 1;
}

int gvp_queue_size(void* pq)
{
	if(pq==0)return 0;
	return ((gq*)pq)->size;
}
int gvp_queue_wait_time(void* pq)
{
	if(pq==0)return 0;
	return ((gq*)pq)->avg_wait_time;
}
int gvp_queue_abort(void* pq)
{
	if(pq)
	{
		//OutputDebugStringA("gvp_queue_abort %d\n", (int)pq);
		//OutputDebugStringA("gvp_queue_abort\n");
		((gq*)pq)->stop = 1;
	}
	return 1;
}
int gvp_is_abort_set(void* pq)
{
	if(pq)return ((gq*)pq)->stop;
	return 1;
}
#ifdef WIN32
int strcasecmp(const char *s1, const char *s2)
{
	while ((*s1 != '\0')
	&& (tolower(*(unsigned char *) s1) ==
	tolower(*(unsigned char *) s2))) 
	{
		s1++;
		s2++;
	}

	return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}

int strncasecmp(const char *s1, const char *s2, unsigned int n)
{
    if (n == 0)
        return 0;

    while ((n-- != 0)
           && (tolower(*(unsigned char *) s1) ==
               tolower(*(unsigned char *) s2))) {
        if (n == 0 || *s1 == '\0' || *s2 == '\0')
            return 0;
        s1++;
        s2++;
    }

    return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}
#endif
