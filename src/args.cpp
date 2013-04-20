#include "stdafx.h"

#include "args.h"


INLINE const char* CmdLineArgs::_cmp( const char* param, const char* id )
{
	if ( *param=='/' ) param++;
	unsigned int d = 0;
	do
	{
		if ( !id[d] )
		{
			switch ( param[d] )
			{
			case ':': return param+d+1;
			case 0: return param+d;
			default: return NULL;
			}
		}
	}
	while ( param[d]==id[d++] );
	return NULL;
}
NOINLINE const char* CmdLineArgs::GetArg( const char* id ) const
{
	typedef char** iterator;
	for ( iterator it = argv, end = it+argc; it!=end; ++it )
	{
		if ( const char* arg = _cmp( *it, id ) )
			return arg;
	}
	return nullptr;
}