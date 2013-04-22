#include "stdafx.h"

#include "process.h"
#include "util.h"


CProcess::CProcess() : hProcess(INVALID_HANDLE_VALUE)
{
	rbuf.base = nullptr;
}
CProcess::~CProcess()
{
	Detach();
}

bool CProcess::Attach( HANDLE hproc )
{
	if ( hProcess = hproc )
	{
		// We need some temporary memory to work with
		if ( rbuf.base = ::VirtualAllocEx( hProcess, nullptr, 4096, MEM_COMMIT, PAGE_EXECUTE_READWRITE ) )
		{
			rbuf.free = (char*)rbuf.base + 0x100;
			return true;
		}
	}
	return false;
}
bool CProcess::Attach( int pid )
{
	this->pid = pid;
	DWORD rights = PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_SUSPEND_RESUME|SYNCHRONIZE;
	return Attach( ::OpenProcess( rights, FALSE, pid ) );
}
bool CProcess::Attach( const _TCHAR* exe, PROCESSENTRY32* lppe )
{
	bool b = false;
	HANDLE hSnap = ::CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if ( hSnap!=INVALID_HANDLE_VALUE )
	{
		PROCESSENTRY32 pe;
		if ( !lppe ) lppe = &pe;

		lppe->dwSize = sizeof(PROCESSENTRY32);
		if ( ::Process32First( hSnap, lppe ) )
		{
			do
			{
				if ( !_tcsicmp( lppe->szExeFile, exe ) )
				{
					// Try to attach, keep looking on failure
					if ( Attach( lppe->th32ProcessID ) )
					{
						b = true;
						break;
					}
				}
			}
			while ( ::Process32Next( hSnap, lppe ) );
		}
		::CloseHandle( hSnap );
	}
	return b;
}
bool CProcess::Attach( const _TCHAR* window )
{
	if ( HWND hwnd = ::FindWindow( NULL, window ) )
	{
		DWORD pid;
		DWORD tid = ::GetWindowThreadProcessId( hwnd, &pid );
		return Attach( pid );
	}
	return false;
}
void CProcess::Detach()
{
	::VirtualFreeEx( hProcess, rbuf.base, 0, MEM_RELEASE );
	::CloseHandle( hProcess );
	hProcess = INVALID_HANDLE_VALUE;
	rbuf.free = rbuf.base = nullptr;
}

bool CProcess::GetRemoteModule( const _TCHAR* filename, MODULEENTRY32* lpme )
{
	bool b = false;
	HANDLE hSnap = ::CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, pid );
	if ( hSnap!=INVALID_HANDLE_VALUE )
	{
		MODULEENTRY32 me;
		if ( !lpme ) lpme = &me;

		lpme->dwSize = sizeof(MODULEENTRY32);
		if ( ::Module32First( hSnap, lpme ) )
		{
			do
			{
				if ( !_tcsicmp( lpme->szModule, filename ) )
				{
					b = true;
					break;
				}
			}
			while ( ::Module32Next( hSnap, lpme ) );
		}
		::CloseHandle( hSnap );
	}
	return b;
}


bool CProcess::ReadData( void* dest, const void* src, unsigned int bytes )
{
	SIZE_T n;
	return ::ReadProcessMemory( hProcess, src, dest, bytes, &n )!=FALSE;
}
bool CProcess::WriteData( void* dest, const void* src, unsigned int bytes )
{
	SIZE_T n;
	return ::WriteProcessMemory( hProcess, dest, src, bytes, &n )!=FALSE;
}


void* CProcess::AddResource( const void* res, unsigned int bytes )
{
	void* rs = nullptr;

	if ( ::WriteProcessMemory( hProcess, rbuf.free, res, bytes, nullptr ) )
	{
		rs = rbuf.free;
		((unsigned char*&)rbuf.free) += ((bytes-1)&~3)+4;
	}

	return rs;
}
//int CProcess::Invoke( unsigned int ms )
//{
//}
int CProcess::Execute( void* addr, void* param )
{
	// Temporary way to invoke this API...
	if ( HANDLE hThread = ::CreateRemoteThread( hProcess, nullptr, 0,
		(LPTHREAD_START_ROUTINE)addr, param, 0, nullptr ) )
	{
		// Wait for thread to finish (check errors!)
		::WaitForSingleObject( hThread, 5000 );

		DWORD code;
		::GetExitCodeThread( hThread, &code );

		// Cleanup
		::CloseHandle( hThread );

		return code;
	}
	return -1;
}

CModule::CModule() : data(nullptr)
{
}
CModule::~CModule()
{
	free( data );
}
bool CModule::Init( CProcess& process, const _TCHAR* dllname )
{
	free( data );
	data = nullptr;

	if ( process.GetRemoteModule( dllname, &me ) )
	{
		if ( void* dll = malloc( me.modBaseSize ) )
		{
			if ( process.ReadData( dll, me.modBaseAddr, me.modBaseSize ) )
			{
				data = dll;
				return true;
			}
			free( dll );
		}
	}
	return false;
}
bool CModule::SigMatch( unsigned char* p, const char* sig ) const
{
	unsigned char* data = (unsigned char*)this->data;
	const char* s = sig;
	char c;
	unsigned char* stack[8];
	unsigned int stacki = 0;
	while ( c = *s++ )
	{
		unsigned char b;
		if ( (b = parsehex(c)) || c=='0' )
		{
			b = (b<<4)|parsehex_unsafe(*s++);
			// Mismatch
			if ( *p++!=b )
				return false;
		}
		else if ( c=='?' )
		{
			// Ignore this byte
			++p;
		}
		else if ( c=='+' )
		{
			// Save this position on the stack
			assert( stacki<(sizeof(stack)/sizeof(stack[0])) );
			stack[stacki++] = p;
		}
		else if ( c=='-' )
		{
			// Pop from stack
			assert( stacki>0 );
			p = stack[--stacki];
		}
		else if ( c=='r' || c=='R' )
		{
			// Figure out the offset
			int offset;
			c = *s++;
			if ( c=='1' ) offset = *(char*)p;
			else if ( c=='2' ) offset = *(short*)p;
			else if ( c=='4' ) offset = *(int*)p;
			else return false;

			// Compute new address as (address)+(skipoffset)+(offset)
			p = p + c-'0' + offset;
		}
		else if ( c=='j' || c=='J' )
		{
			// Compute offset for this absolute address
			int off = *(unsigned char**)p - me.modBaseAddr;
			// New search pointer
			p = (unsigned char*)data + off;
		}
		else if ( c=='\'' || c=='\"' )
		{
			// Raw scan
			for ( ; *s!=c; ++s )
			{
				unsigned char b = static_cast<unsigned char>( *s );
				if ( !b || *p++!=b )
					return false;
			}
			++s;
		}
		
		// Check if we're still within valid bounds
		if ( p<data || p>=(data+me.modBaseSize) )
			return false;
	}
	return true;
}
int CModule::SigScan( const char* sig ) const
{
	// Shitty guess for the signature length :3
	int len = strlen(sig)/2;

	for ( unsigned i = 0; i<(me.modBaseSize-len); ++i )
	{
		if ( SigMatch( (unsigned char*)data + i, sig ) )
			return i;
	}

	return -1;
}
