#pragma once

// Abstract launch params
// Yeah no unicode here...

class CmdLineArgs
{
public:
	CmdLineArgs( int argc, char* argv[] ) : argc(argc-2), argv(argv+2) { }

	const char* GetArg( const char* id ) const;

	template< typename T >
	inline T GetArg( const char* id, T def ) const;

protected:
	static const char* _cmp( const char* param, const char* id );

private:
	int argc;
	char** argv;
};

template<> inline int CmdLineArgs::GetArg<int>( const char* id, int def ) const
{
	const char* s = GetArg( id );
	return s ? atoi( s ) : def;
}
template<> inline float CmdLineArgs::GetArg<float>( const char* id, float def ) const
{
	const char* s = GetArg( id );
	return s ? static_cast<float>( atof( s ) ) : def;
}
template<> inline double CmdLineArgs::GetArg<double>( const char* id, double def ) const
{
	const char* s = GetArg( id );
	return s ? atof( s ) : def;
}
template<> inline bool CmdLineArgs::GetArg<bool>( const char* id, bool def ) const
{
	// This can be done a bit better...
	const char* s = GetArg( id );
	return s ? ( *s==0 ? true : atoi( s )!=0 ) : false;
}
