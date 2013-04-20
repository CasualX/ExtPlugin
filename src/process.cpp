#include "stdafx.h"

#include "process.h"
#include "util.h"


CProcess::CProcess() : hProcess(INVALID_HANDLE_VALUE)
{
}
CProcess::~CProcess()
{
	::CloseHandle( hProcess );
}

bool CProcess::Attach( HANDLE hproc )
{
	if ( hProcess = hproc )
	{
		return true;
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
int CModule::SigScan( const char* sig )
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
