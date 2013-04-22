#include "stdafx.h"

#include "extplugin.h"

int _tmain( int argc, _TCHAR* argv[] )
{
	// Sue me
	CmdLineArgs& args = *(CmdLineArgs*) &argc;
	// Start our magic working
	return LaunchExtPlugin( &HandleMultiProcess, args );
	// Return number of successful affected processes
}
