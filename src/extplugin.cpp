// ExtPlugin.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "extplugin.h"
#include "process.h"
#include "args.h"
#include "srcsdk.h"


int LaunchExtPlugin( bool (*pfn)( LPPROCESSENTRY32, CmdLineArgs& ), CmdLineArgs& args )
{
	// Start by picking the parameters for selecting a process
	unsigned long pid = args.GetArg<int>( "pid", 0 );
	if ( const char* window = args.GetArg( "window" ) )
	{
		if ( pid ) return 0;
		HWND hWnd = ::FindWindowA( NULL, window );
		if ( ! ::GetWindowThreadProcessId( hWnd, &pid ) )
			return 0;
	}
	// Please only specify one type...
	const char* exefile = args.GetArg( "process" );
	if ( !( !exefile ^ !pid ) )
		return 0;

	// Loop through all processes and dispatch on matches
	int cnt = 0;
	HANDLE hSnap = ::CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	if ( hSnap!=INVALID_HANDLE_VALUE )
	{
		PROCESSENTRY32 pe;

		pe.dwSize = sizeof(PROCESSENTRY32);
		if ( ::Process32First( hSnap, &pe ) )
		{
			do
			{
				if ( exefile ? (!_tcsicmp( pe.szExeFile, exefile )) : (pe.th32ProcessID==pid) )
				{
					++cnt;
					pfn( &pe, args );
				}
			}
			while ( ::Process32Next( hSnap, &pe ) );
		}
		::CloseHandle( hSnap );
	}
	return cnt;
}
bool HandleMultiProcess( LPPROCESSENTRY32 lppe, CmdLineArgs& args )
{
	// Let the user know we found a process
	_tprintf( _T("**** %s (%d) ****\n"), lppe->szExeFile, lppe->th32ProcessID );

	// Attempt to attach
	CExtPlugin plugin;
	if ( plugin.Attach( lppe->th32ProcessID ) )
	{
		// All good, now work our magic
		plugin.Work( args );
		return true;
	}

	// Something went wrong, GetLastError() may be helpful.
	_tprintf( _T(" Attach failure (%08X)!\n"), GetLastError() );
	return false;
}



bool CExtPlugin::Attach( int pid )
{
	return process.Attach( pid );
}

void CExtPlugin::Work( const CmdLineArgs& args )
{
	verbose = args.GetArg<bool>( "verbose", false );

	engine.Init( process, _T("engine.dll") );
		
	if ( args.GetArg<bool>( "picmip", false ) )
	{
		FixPicMip();
	}
	if ( args.GetArg( "cvars" ) )
	{
		LoopOverCvars();
	}
	if ( const char* cmd = args.GetArg( "command" ) )
	{
		RunCommand( cmd );
	}
}

void CExtPlugin::Feature( const char* str )
{
	_tprintf( _T("---- %s\n"), str );
}

void CExtPlugin::FixPicMip()
{
	Feature( "PicMipFix" );

	// Start with getting some required offsets
	int off1, off2, off3;
	off1 = engine.SigScan( "A3 ? ? ? ? 3B C6 74 0D 6A 02 6A FF" ); // mat_picmip checking code (CheckSpecialCheatVars())
	off2 = engine.SigScan( "8B 15 ? ? ? ? 83 7A 30 00 74 20 D9" ); // random ConVar snd_visualize
	off3 = engine.SigScan( "80 3D ? ? ? ? 00 74 03 33 C0 C3" ); // Some global variable
	if ( verbose )
	{
		_tprintf( _T(" off1 = 0x%08X\n off2 = 0x%08X\n off3 = 0x%08X\n"), off1, off2, off3 );
	}

	if ( off1>=0 && off2>=0 && off3>=0 )
	{
		// Keep in mind that these ConVar* pointers are only valid inside the game process (not here!)

		ConVar* cvar = (ConVar*)( engine.ReadData<char*>( off2+2 )-0x1C );
		ConVar** picmipvar = engine.ReadData<ConVar**>( off1+1 );
		ConVar* picmip;
		process.ReadData( picmip, picmipvar );
		process.WriteData( (char*)picmip+0x38, -10.0f );
		process.WriteData( (char*)picmip+0x40, 6.0f );
		process.WriteData( picmipvar, cvar );
		
		// Disables checks on r_lod & r_rootlod
		process.WriteData( engine.ReadData<bool*>( off3+2 ), true );
		
		_tprintf( _T(" Success!\n") );
	}
	else
	{
		_tprintf( _T(" Signature failure!\n") );
	}
}

void CExtPlugin::RunCommand( const char* cmd )
{
	// Executes CEngine::ClientCmd_Unrestricted with your command in the context of the game
	Feature( "RunCommand" );

	// ( reference "Cbuf_AddText: buffer overflow" )
	int pfn = engine.SigScan( "55 8B EC 8B 45 08 50 E8 +r4 55 8B EC 51 FF 15- ? ? ? ? 83 C4 04 5D C2 04 00" );
	if ( pfn<0 ) _tprintf( _T(" Signature not found!\n") );
	else
	{
		if ( verbose )
		{
			_tprintf( _T(" [%s+%08X] CEngine::ClientCmd_Unrestricted\n"), engine.GetFileName(), pfn );
			_tprintf( _T(" Running command \"%s\"\n"), cmd );
		}
		// We need the command to execute in the game process
		void* h = process.AddResource( cmd, strlen(cmd)+1 );
		// And execute it
		process.Execute( engine.GetAddress(pfn), h );
		
		_tprintf( _T(" Success!\n") );
	}
}

void CExtPlugin::UnlockVar( CModule& module, const char* varname )
{
	// This works by scanning for the ConVar constructor code.
	// It has various problems but it'll do the job most of the time.
	// FIXME! Not a feature yet, make it useful first!
	//        If you need to edit ConVars, do it in ProcessCvar instead
	Feature( "UnlockVar" );

	// Generate signature
	char buf[80];
	_snprintf( buf, sizeof(buf), "68 ? ? ? ? 68 +j '%s'- ? ? ? ? B9 ? ? ? ? E8", varname );

	// Lookup signature
	int p = module.SigScan( buf );
	if ( p<0 ) _tprintf( _T(" Signature not found!\n") );
	else
	{
		// Fetch a pointer to the cvar
		ConVar* pcvar;
		process.ReadData( pcvar, module.GetAddress( p + 11 ) );
		if ( verbose )
		{
			_tprintf( _T(" [%s+%08X] ConVar %s\n"), module.GetFileName(), module.GetOffset(pcvar), varname );
		}
		// Fetch the whole cvar for convenient editing
		ConVar_t cvar;
		process.ReadData( cvar, pcvar );
		cvar.bHasMin = false;
		cvar.bHasMax = false;
		cvar.nFlags &= ~((1<<14)|(1<<13)); // ~FCVAR_CHEAT
		// Write back the modified version
		process.WriteData( pcvar, cvar );

		_tprintf( _T(" Success!\n") );
	}
}

void CExtPlugin::LoopOverCvars()
{
	Feature( "LoopOverCvars" );

	// Start by getting at ICvar
	int p = engine.SigScan( "68 +j 'VEngineCvar004'- ? ? ? ? FF D0 83 C4 08 A3" );
	if ( p<0 ) _tprintf( _T(" Signature not found!\n") );
	else
	{
		if ( verbose )
		{
			_tprintf( _T(" engine.dll!%08X reference for ICvar\n"), p );
		}
		// Get pointer to the linked list of cvars
		ICvar* pvar;
		process.ReadData( pvar, engine.ReadData<ICvar**>(p+11) );
		ConCommandBase* pcmds;
		process.ReadData( pcmds, (char*)pvar + 0x30 );

		// Loop through all cvars
		for ( ConVar_t var; pcmds; pcmds = var.pNext )
		{
			if ( process.ReadData( var, pcmds ) )
			{
				if ( ProcessCvar( var ) )
					process.WriteData( pcmds, var );
			}
			else
			{
				// Failure... Can't keep searching because we need this cvar to access the next one!
				_tprintf( _T(" Failed to read a cvar!\n") );
				return;
			}
		}
		_tprintf( _T(" Done!\n") );
	}
}
bool CExtPlugin::ProcessCvar( ConVar_t& var )
{
	// Return true to apply changes made to the var

	// Now... NOT EVERYTHING PASSED IS ACTUALLY A ConVar (but a ConCommand)!
	//  it's a bit hard to check and not really needed if you're careful.
	// (to figure out you'd need to disassemble its IsCommand() to see what it returns)

	char name[80];
	process.ReadData( name, var.pszName );

	if ( !strcmp( name, "viewmodel_fov_demo" ) )
	{
		var.bHasMin = false;
		var.bHasMax = false;
	}
	else return false;
	return true;
}

