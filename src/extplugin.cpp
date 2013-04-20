// ExtPlugin.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "process.h"
#include "args.h"


bool AttachToProcess( CProcess& process, const CmdLineArgs& args )
{
	if ( int pid = args.GetArg<int>( "pid", 0 ) )
		return process.Attach( pid );
	if ( const char* exe = args.GetArg( "process" ) )
		return process.Attach( exe, nullptr );
	if ( const char* wnd = args.GetArg( "window" ) )
		return process.Attach( wnd );
	return false;
}
class ConVar;

int _tmain( int argc, _TCHAR* argv[] )
{
	// Sue me
	CmdLineArgs& args = *(CmdLineArgs*)&argc;

	// Attach to a process
	CProcess proc;
	if ( AttachToProcess( proc, args ) )
	{
		// Apply mat_picmip fix
		// THIS IS TEMPORARY CODE!!

		CModule engine;
		int off1, off2;
		if ( engine.Init( proc, _T("engine.dll") ) &&
			( off1 = engine.SigScan( "A3 ? ? ? ? 3B C6 74 0D 6A 02 6A FF" ) ) && // mat_picmip checking code
			( off2 = engine.SigScan( "8B 15 ? ? ? ? 83 7A 30 00 74 20 D9" ) ) ) // random ConVar snd_visualize
		{
			ConVar* snd_visualize = (ConVar*)( engine.ReadData<int>( off2+2 ) - 0x1C );
			ConVar** mat_picmip = engine.ReadData<ConVar**>( off1+1 );
			ConVar* p;
			proc.ReadData<ConVar*>( p, mat_picmip );
			proc.WriteData<float>( (char*)p+0x38, -10.0f );
			proc.WriteData<float>( (char*)p+0x40, 6.0f );
			proc.WriteData<ConVar*>( mat_picmip, snd_visualize );
		}
	}

	return 0;
}
