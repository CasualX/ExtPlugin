#pragma once

inline unsigned char parsehex_unsafe( char h )
{
	static const unsigned char lookup[] = {
		0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,
		10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		10,11,12,13,14,15,0
	};
	assert( (static_cast<unsigned char>(h-'0'))<(sizeof(lookup)/sizeof(lookup[0])) );
	return lookup[static_cast<unsigned char>(h-'0')];
}
inline unsigned char parsehex( char h )
{
	unsigned char i = h-'0';
	unsigned char c = 0;
	if ( static_cast<unsigned char>(h-'0')<56 ) c = parsehex_unsafe(h);
	return c;
}
inline bool ishex( char h )
{
	return parsehex( h ) || h=='0';
}

