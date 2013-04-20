#pragma once

// Abstracts a process and makes a few things easy

class CProcess
{
public:
	CProcess();
	~CProcess();

	// Various ways to attach to a process
	bool Attach( HANDLE hProcess );
	bool Attach( int pid );
	bool Attach( const _TCHAR* exe, PROCESSENTRY32* lppe );
	bool Attach( const _TCHAR* window );
	
	// Find module information in the specified process
	bool GetRemoteModule( const _TCHAR* filename, MODULEENTRY32* lpme );

	// Read some information
	bool ReadData( void* dest, const void* src, unsigned int bytes );
	template< typename T >
	inline bool ReadData( T& dest, const void* src )
	{
		return ReadData( &dest, src, sizeof(T) );
	}

	// Write some data back
	bool WriteData( void* dest, const void* src, unsigned int bytes );
	template< typename T >
	inline bool WriteData( void* dest, const T& src )
	{
		return WriteData( dest, &src, sizeof(T) );
	}

private:
	int pid;
	HANDLE hProcess;
};

// Wrapper for a module

class CModule
{
public:
	CModule();
	~CModule();

	bool Init( CProcess& process, const _TCHAR* dllname );

	int SigScan( const char* sig );

	template< typename T >
	inline T& ReadData( int offset )
	{
		return *(T*)( (char*)data + offset );
	}

	inline void* GetAddress( int offset )
	{
		return me.modBaseAddr + offset;
	}

protected:

private:
	MODULEENTRY32 me;
	void* data;
};
