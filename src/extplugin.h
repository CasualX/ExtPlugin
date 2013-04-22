#pragma once

#include "process.h"
#include "args.h"


int LaunchExtPlugin( bool (*pfn)( LPPROCESSENTRY32, CmdLineArgs& ), CmdLineArgs& args );
bool HandleMultiProcess( LPPROCESSENTRY32 lppe, CmdLineArgs& args );


class CExtPlugin
{
public:
	bool Attach( int pid );

	void Work( const CmdLineArgs& args );

	void Feature( const char* str );

	void FixPicMip();

	void RunCommand( const char* cmd );

	void UnlockVar( CModule& module, const char* varname );
	void LoopOverCvars();
	bool ProcessCvar( struct ConVar_t& var );

private:
	bool verbose;
	CProcess process;
	CModule engine;
};
