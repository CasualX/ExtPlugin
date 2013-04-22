#pragma once

class ConVar;
class ConCommandBase;
class ConCommand;
class ICvar;




struct ConVar_t
{
/*00*/	void* vtable;
/*04*/	ConCommandBase* pNext;
/*08*/	bool bRegistered;
/*0C*/	const char* pszName;
/*10*/	const char* pszHelpString;
/*14*/	int nFlags;

/*18*/	void* icvar;
/*1C*/	ConVar* pParent;
/*20*/	const char* pszDefaultValue;
/*24*/	const char* pszString;
/*28*/	int StringLength;
/*2C*/	float fValue;
/*30*/	int nValue;
/*34*/	bool bHasMin;
/*38*/	float fMinVal;
/*3C*/	bool bHasMax;
/*40*/	float fMaxVal;
/*44*/	void* fnChangeCallback;
};