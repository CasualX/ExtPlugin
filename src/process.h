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
	void Detach();
	
	// Find module information in the specified process
	bool GetRemoteModule( const _TCHAR* filename, MODULEENTRY32* lpme );

	// Read some information
	bool ReadData( void* dest, const void* src, unsigned int bytes );
	template< typename T >
	inline bool ReadData( T& dest, const void* src )
	{
		return ReadData( &dest, src, sizeof(T) );
	}
	template< typename T >
	inline T ReadData( const void* src )
	{
		T t;
		ReadData( &t, src, sizeof(T) ); // Not checking for failure...
		return t;
	}

	// Write some data back
	bool WriteData( void* dest, const void* src, unsigned int bytes );
	template< typename T >
	inline bool WriteData( void* dest, const T& src )
	{
		return WriteData( dest, &src, sizeof(T) );
	}

	// Remote buffer management
	void* AddResource( const void* res, unsigned int bytes );
	//int Invoke( unsigned ms = INFINITE );
	int Execute( void* addr, void* param );

private:
	int pid;
	HANDLE hProcess;

	struct rbuf_t
	{
		void* base;
		void* free;
	} rbuf;
};

// Wrapper for a module

class CModule
{
public:
	CModule();
	~CModule();

	bool Init( CProcess& process, const _TCHAR* dllname );

	bool SigMatch( unsigned char* p, const char* sig ) const;
	int SigScan( const char* sig ) const;

	template< typename T >
	inline T& ReadData( int offset ) const
	{
		return *(T*)( (char*)data + offset );
	}

	inline void* GetAddress( int offset = 0 ) const
	{
		return me.modBaseAddr + offset;
	}
	inline int GetOffset( void* ptr ) const
	{
		return (char*)ptr - (char*)me.modBaseAddr;
	}
	inline const char* GetFileName() const
	{
		return me.szModule;
	}

protected:

private:
	MODULEENTRY32 me;
	void* data;
};
