//ANUS



//This used to be in const.h. It now includes this file to keep custom CVars available everywhere.




/*
A few important notes:

Still need to extern CVars meant to be received by the client in cl_dll/ammo.cpp (may change what file gets this role at some point).
Also, need to extern (all?) CVars in dlls/client.cpp.

*/

















#ifndef CVAR_CUSTOM_H
#define CVAR_CUSTOM_H




//CREDIT TO TEAM:
//http://stackoverflow.com/questions/6671698/adding-quotes-to-argument-in-c-preprocessor
#define QQQQ(x) #x
#define QUOTE(x) QQQQ(x)



//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************
//MODDD - NOTE - it appears this space is visible to both the client and server side files.
//*************************************************************************************************
//*************************************************************************************************
//*************************************************************************************************

/*
#include "../debugSetting.h"
#if debugOn == 1
//In vs6, this doesn't seem to get autogenerated when we need it.  ???
#define _DEBUG 1
#define DEBUG 1
#endif
*/


//#ifndef GLOBALSTUFF
//#define GLOBALSTUFF

#define EASY_CVAR_DECLARATION_CLIENT(CVarName)\
	DUMMY

#define EASY_CVAR_UPDATE_CLIENT(CVarName)\
	DUMMY

#define EASY_CVAR_DECLARATION_SERVER(CVarName)\
	DUMMY

#define EASY_CVAR_UPDATE_SERVER(CVarName)\
	DUMMY



//!!!!
#define CALL_EASY_CVAR_CREATE_CLIENT(CVarName, flags)\
	CVAR_CREATE(#CVarName, QUOTE(DEFAULT_##CVarName), flags);\
	//global2_##CVarName = DEFAULT_##CVarName;

/*
#define EASY_CVAR_CREATE_CLIENT(CVarName)\
	CALL_EASY_CVAR_CREATE_CLIENT(CVarName, 0);
#define EASY_CVAR_CREATE_CLIENT_A(CVarName)\
	CALL_EASY_CVAR_CREATE_CLIENT(CVarName, FCVAR_ARCHIVE);
*/

#define EASY_CVAR_CREATE_CLIENT_CLIENTONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_CLIENT(CVarName, 0);
#define EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_CLIENT(CVarName, FCVAR_ARCHIVE);

#define EASY_CVAR_CREATE_CLIENT_SERVERONLY(CVarName)\
	DUMMY
#define EASY_CVAR_CREATE_CLIENT_A_SERVERONLY(CVarName)\
	DUMMY












//QUOTE(DEFAULT_##CVarName);
#define CALL_EASY_CVAR_CREATE_SERVER(CVarName, argFlags)\
	CVAR_REGISTER(&globalcvar_##CVarName);\
	DUMMY

#define CALL_EASY_CVAR_CREATE_SERVER_SETUP(CVarName, argFlags)\
	cvar_t globalcvar_##CVarName = {#CVarName, QUOTE(DEFAULT_##CVarName), argFlags};


#define EASY_CVAR_CREATE_SERVER_CLIENTONLY(CVarName)\
	DUMMY;
#define EASY_CVAR_CREATE_SERVER_A_CLIENTONLY(CVarName)\
	DUMMY;

#define EASY_CVAR_CREATE_SERVER_SERVERONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_SERVER(CVarName, 0);
#define EASY_CVAR_CREATE_SERVER_A_SERVERONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_SERVER(CVarName, FCVAR_ARCHIVE);



#define EASY_CVAR_CREATE_SERVER_SETUP_CLIENTONLY(CVarName)\
	DUMMY;
#define EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY(CVarName)\
	DUMMY;

#define EASY_CVAR_CREATE_SERVER_SETUP_SERVERONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_SERVER_SETUP(CVarName, 0);
#define EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_SERVER_SETUP(CVarName, FCVAR_ARCHIVE);











#define EASY_CVAR_SET(CVarName, valueV)\
	CVAR_SET_FLOAT(#CVarName, valueV);

#define EASY_CVAR_RESET(CVarName)\
	CVAR_SET_FLOAT(#CVarName, DEFAULT_##CVarName);



/*
#ifndef CLIENT_DLL
	//server.
	#define EASY_CVAR_GET(CVarName)\
	global_##CVarName

	#define EASY_CVAR_EXTERN(CVarName)\
	extern float global_##CVarName;

	#define EASY_CVAR_EXTERN_CLIENTONLY(CVarName)
	#define EASY_CVAR_EXTERN_SERVERONLY(CVarName) EASY_CVAR_EXTERN(CVarName)

#else
	//client.
	#define EASY_CVAR_GET(CVarName)\
	global2_##CVarName

	#define EASY_CVAR_EXTERN(CVarName)\
	extern float global2_##CVarName;

	#define EASY_CVAR_EXTERN_CLIENTONLY(CVarName) EASY_CVAR_EXTERN(CVarName)
	#define EASY_CVAR_EXTERN_SERVERONLY(CVarName)

#endif
	*/

#ifndef CLIENT_DLL
	//server.
#define EASY_CVAR_GET(CVarName)\
	CVAR_GET_FLOAT( #CVarName )

#define EASY_CVAR_EXTERN(CVarName)\
	DUMMY

#else
	//client.
#define EASY_CVAR_GET(CVarName)\
	CVAR_GET_FLOAT( #CVarName )

#define EASY_CVAR_EXTERN(CVarName)\
	DUMMY
#endif






//MSG_ONE or MSG_ALL ???
#define EASY_CVAR_RESET_MASS_CLIENT_SIGNAL\
	if(pev != NULL){\
		MESSAGE_BEGIN( MSG_ONE, gmsgResetClientCVar, NULL, pev );\
		MESSAGE_END();\
	}






/////////////////////////////////////////////////////////////////////////////////////////////////////////
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG



#define EASY_CVAR_DECLARE_HASH_ARRAY\
	DUMMY;

#define EASY_CVAR_EXTERN_HASH_ARRAY\
	DUMMY;

//Let the hash array know what variable this is.
#define EASY_CVAR_HASH(CVarName, ID)\
	DUMMY;





#define EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(CVarName)\
	DUMMY

#define EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(CVarName)\
	DUMMY

#define EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(CVarName)\
	DUMMY

#define EASY_CVAR_UPDATE_SERVER_DEBUGONLY(CVarName)\
	DUMMY


#define CALL_EASY_CVAR_CREATE_CLIENT_DEBUGONLY(CVarName, flags)\
	CALL_EASY_CVAR_CREATE_CLIENT(CVarName, flags);

/*
#define EASY_CVAR_CREATE_CLIENT_DEBUGONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_CLIENT(CVarName, 0);
#define EASY_CVAR_CREATE_CLIENT_A_DEBUGONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_CLIENT(CVarName, FCVAR_ARCHIVE);
*/


#define EASY_CVAR_CREATE_CLIENT_CLIENTONLY_DEBUGONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_CLIENT(CVarName, 0);
#define EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_CLIENT(CVarName, FCVAR_ARCHIVE);

#define EASY_CVAR_CREATE_CLIENT_SERVERONLY_DEBUGONLY(CVarName)\
	DUMMY
#define EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(CVarName)\
	DUMMY





#define CALL_EASY_CVAR_CREATE_SERVER_DEBUGONLY(CVarName, argFlags)\
	CALL_EASY_CVAR_CREATE_SERVER(CVarName, argFlags)

#define CALL_EASY_CVAR_CREATE_SERVER_SETUP_DEBUGONLY(CVarName, argFlags)\
	CALL_EASY_CVAR_CREATE_SERVER_SETUP(CVarName, argFlags)


#define EASY_CVAR_CREATE_SERVER_CLIENTONLY_DEBUGONLY(CVarName)\
	DUMMY;
#define EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(CVarName)\
	DUMMY;

#define EASY_CVAR_CREATE_SERVER_SERVERONLY_DEBUGONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_SERVER_DEBUGONLY(CVarName, 0);
#define EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_SERVER_DEBUGONLY(CVarName, FCVAR_ARCHIVE);



#define EASY_CVAR_CREATE_SERVER_SETUP_CLIENTONLY_DEBUGONLY(CVarName)\
	DUMMY;
#define EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(CVarName)\
	DUMMY;

#define EASY_CVAR_CREATE_SERVER_SETUP_SERVERONLY_DEBUGONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_SERVER_SETUP_DEBUGONLY(CVarName, 0);
#define EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(CVarName)\
	CALL_EASY_CVAR_CREATE_SERVER_SETUP_DEBUGONLY(CVarName, FCVAR_ARCHIVE);







#define EASY_CVAR_SET_DEBUGONLY(CVarName, valueV)\
	CVAR_SET_FLOAT(#CVarName, valueV);

#define EASY_CVAR_RESET_DEBUGONLY(CVarName)\
	CVAR_SET_FLOAT(#CVarName, DEFAULT_##CVarName);





//DUMMIED!
#define EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(CVarName, CVarNameLower)
#define EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName, CVarNameLower, ID)
#define EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY_CLIENTONLY(CVarName, CVarNameLower, ID)

//Dummied!
//#define EASY_CVAR_CLIENTSENDOFF(CVarName, ID)














//NOTE: for now, only 
//EASY_CVAR_DECLARATION_SERVER_DEBUGONLY_CLIENTSENDOFF
//EASY_CVAR_UPDATE_SERVER_DEBUGONLY_CLIENTSENDOFF
//~varry from the default.

//Identical to non-clientsendoff.
#define EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName)\
	DUMMY
#define EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY_CLIENTONLY(CVarName)\
	DUMMY


//Identical to non-clientsendoff.
#define EASY_CVAR_UPDATE_CLIENT_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName)\
	DUMMY
#define EASY_CVAR_UPDATE_CLIENT_DEBUGONLY_CLIENTONLY(CVarName)\
	DUMMY


//Identical to non-clientsendoff, changes for RELEASE builds.
#define EASY_CVAR_DECLARATION_SERVER_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName)\
	DUMMY
#define EASY_CVAR_DECLARATION_SERVER_DEBUGONLY_CLIENTONLY(CVarName)\
	DUMMY




//Identical.
#define EASY_CVAR_UPDATE_SERVER_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName, ID)\
	DUMMY

#define EASY_CVAR_UPDATE_SERVER_DEBUGONLY_CLIENTONLY(CVarName, ID)\
	DUMMY
//no CVAR creation redos.


//is that okay?  Implied only for the server (global_...).
#define EASY_CVAR_SET_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName, valueV)\
	CVAR_SET_FLOAT(#CVarName, valueV);
#define EASY_CVAR_SET_DEBUGONLY_CLIENTONLY(CVarName, valueV)\
	CVAR_SET_FLOAT(#CVarName, valueV);

#define EASY_CVAR_RESET_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName)\
	CVAR_SET_FLOAT(#CVarName, DEFAULT_##CVarName);
#define EASY_CVAR_RESET_DEBUGONLY_CLIENTONLY(CVarName)\
	CVAR_SET_FLOAT(#CVarName, DEFAULT_##CVarName);


#ifdef CLIENT_DLL
#define EASY_CVAR_EXTERN_DEBUGONLY(CVarName) EASY_CVAR_EXTERN(CVarName)
#define EASY_CVAR_EXTERN_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName) EASY_CVAR_EXTERN(CVarName)
#define EASY_CVAR_EXTERN_DEBUGONLY_CLIENTONLY(CVarName) EASY_CVAR_EXTERN(CVarName)
#else
#define EASY_CVAR_EXTERN_DEBUGONLY(CVarName) EASY_CVAR_EXTERN(CVarName)
#define EASY_CVAR_EXTERN_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName) EASY_CVAR_EXTERN(CVarName)
#define EASY_CVAR_EXTERN_DEBUGONLY_CLIENTONLY(CVarName) EASY_CVAR_EXTERN(CVarName)
#endif





/////////////////////////////////////////////////////////////////////////////////////////////////////////
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#else
// TODOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO-wuh!!

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/////////////////////////////////////////////////////////////////////////////////////////////////////////




#define EASY_CVAR_DECLARE_HASH_ARRAY\
	float* aryCVarHash[CVAR_CLIENTSENDOFF_COUNT];\
	char* aryCVarHashName[CVAR_CLIENTSENDOFF_COUNT];

#define EASY_CVAR_EXTERN_HASH_ARRAY\
	extern float* aryCVarHash[];\
	extern char* aryCVarHashName[];

//Let the hash array know what variable this is.
#define EASY_CVAR_HASH(CVarName, ID)\
	aryCVarHash[ID] = &global2_##CVarName;\
	aryCVarHashName[ID] = #CVarName;





//Not debugging.  Convert these into constants only!

//now server only.
#define EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY(CVarName) DUMMY
	//float global2_##CVarName = DEFAULT_##CVarName;

#define EASY_CVAR_UPDATE_CLIENT_DEBUGONLY(CVarName) DUMMY

//!!!!
#define EASY_CVAR_DECLARATION_SERVER_DEBUGONLY(CVarName)\
	//float global_##CVarName = DEFAULT_##CVarName;

#define EASY_CVAR_UPDATE_SERVER_DEBUGONLY(CVarName) DUMMY

/*
#define EASY_CVAR_CREATE_CLIENT_DEBUGONLY(CVarName) DUMMY
#define EASY_CVAR_CREATE_CLIENT_A_DEBUGONLY(CVarName) DUMMY
*/

#define EASY_CVAR_CREATE_CLIENT_CLIENTONLY_DEBUGONLY(CVarName)\
	DUMMY
#define EASY_CVAR_CREATE_CLIENT_A_CLIENTONLY_DEBUGONLY(CVarName)\
	DUMMY

#define EASY_CVAR_CREATE_CLIENT_SERVERONLY_DEBUGONLY(CVarName)\
	DUMMY
#define EASY_CVAR_CREATE_CLIENT_A_SERVERONLY_DEBUGONLY(CVarName)\
	DUMMY





#define CALL_EASY_CVAR_CREATE_SERVER_DEBUGONLY(CVarName, argFlags)\
	DUMMY

#define CALL_EASY_CVAR_CREATE_SERVER_SETUP_DEBUGONLY(CVarName, argFlags)\
	DUMMY


#define EASY_CVAR_CREATE_SERVER_CLIENTONLY_DEBUGONLY(CVarName)\
	DUMMY
#define EASY_CVAR_CREATE_SERVER_A_CLIENTONLY_DEBUGONLY(CVarName)\
	DUMMY

#define EASY_CVAR_CREATE_SERVER_SERVERONLY_DEBUGONLY(CVarName)\
	DUMMY
#define EASY_CVAR_CREATE_SERVER_A_SERVERONLY_DEBUGONLY(CVarName)\
	DUMMY



#define EASY_CVAR_CREATE_SERVER_SETUP_CLIENTONLY_DEBUGONLY(CVarName)\
	DUMMY
#define EASY_CVAR_CREATE_SERVER_SETUP_A_CLIENTONLY_DEBUGONLY(CVarName)\
	DUMMY

#define EASY_CVAR_CREATE_SERVER_SETUP_SERVERONLY_DEBUGONLY(CVarName)\
	DUMMY
#define EASY_CVAR_CREATE_SERVER_SETUP_A_SERVERONLY_DEBUGONLY(CVarName)\
	DUMMY















//Don't do it this way.
///////////////////////////////////////////////////////////////////////////
#define EASY_CVAR_CREATE_SERVER_DEBUGONLY(CVarName) DUMMY
#define EASY_CVAR_CREATE_SERVER_AC_DEBUGONLY(CVarName) DUMMY
#define EASY_CVAR_CREATE_SERVER_A_DEBUGONLY(CVarName) DUMMY
#define EASY_CVAR_CREATE_SERVER_C_DEBUGONLY(CVarName) DUMMY

#define EASY_CVAR_CREATE_SERVER_SETUP_DEBUGONLY(CVarName) DUMMY
#define EASY_CVAR_CREATE_SERVER_SETUP_AC_DEBUGONLY(CVarName) DUMMY
#define EASY_CVAR_CREATE_SERVER_SETUP_A_DEBUGONLY(CVarName) DUMMY
#define EASY_CVAR_CREATE_SERVER_SETUP_C_DEBUGONLY(CVarName) DUMMY
///////////////////////////////////////////////////////////////////////////


#ifdef CLIENT_DLL
//client.



//is that okay?  Implied only for the server (global_...).
#define EASY_CVAR_SET_DEBUGONLY(CVarName, valueV)\
	if(global2_hiddenMemPrintout)easyForcePrintLine("CVAR DEBUG: Server: Set %s to %g", #CVarName, (float)valueV);\
	global2_##CVarName = valueV;


#define EASY_CVAR_RESET_ALT(CVarName)\
	global2_##CVarName = DEFAULT_##CVarName;

#define EASY_CVAR_RESET_DEBUGONLY(CVarName)


#define EASY_CVAR_RESET_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName) EASY_CVAR_RESET_ALT(CVarName)
#define EASY_CVAR_RESET_DEBUGONLY_CLIENTONLY(CVarName) EASY_CVAR_RESET_ALT(CVarName)





#else
//server.



#define EASY_CVAR_SET_DEBUGONLY(CVarName, valueV)\
	if(global_hiddenMemPrintout)easyForcePrintLine("CVAR DEBUG: Server: Set %s to %g", #CVarName, (float)valueV);\
	global_##CVarName = valueV;


#define EASY_CVAR_RESET_ALT(CVarName)\
	global_##CVarName = DEFAULT_##CVarName;

#define EASY_CVAR_RESET_DEBUGONLY(CVarName) EASY_CVAR_RESET_ALT(CVarName)

#define EASY_CVAR_RESET_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName) EASY_CVAR_RESET_ALT(CVarName)
#define EASY_CVAR_RESET_DEBUGONLY_CLIENTONLY(CVarName)




#endif




//g_engfuncs.pfnCVarSetFloat
//g_engfuncs.pfnCVarGetPointer

//EASY_CVAR_SET_DEBUGONLY(CVarName, tempF)\

#define EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY(CVarName, CVarNameLower)\
if( FStrEq(pcmdRefinedRef, #CVarNameLower)  ){\
	CBasePlayer* tempplayer = GetClassPtr((CBasePlayer *)pev);\
	const char* arg1ref = CMD_ARGV(1);\
	const char* arg2ref = CMD_ARGV(2);\
	if(!isStringEmpty(arg2ref)){\
		easyForcePrintLine("ERROR: Only one arg accepted following the CVar. No more than 1 space total.");\
	}if(!isStringEmpty(arg1ref)){\
		float tempF = 0;\
		try{\
			tempF = tryStringToFloat(arg1ref);\
			global_##CVarName = tempF;\
			saveHiddenCVars();\
		}catch(int){\
			easyForcePrintLine("ERROR: Bad input. No effect.");\
		}\
	}else{\
		easyForcePrintLine("\"%s\" is %g",#CVarName,global_##CVarName);\
	}\
	return;\
}


#define EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName, CVarNameLower, ID)\
if( FStrEq(pcmdRefinedRef, #CVarNameLower)  ){\
	CBasePlayer* tempplayer = GetClassPtr((CBasePlayer *)pev);\
	const char* arg1ref = CMD_ARGV(1);\
	const char* arg2ref = CMD_ARGV(2);\
	if(!isStringEmpty(arg2ref)){\
		easyForcePrintLine("ERROR: Only one arg accepted following the CVar. No more than 1 space total.");\
	}if(!isStringEmpty(arg1ref)){\
		float tempF = 0;\
		try{\
			tempF = tryStringToFloat(arg1ref);\
			global_##CVarName = tempF;\
			EASY_CVAR_UPDATE_SERVER_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName, ID)\
			saveHiddenCVars();\
		}catch(int){\
			easyForcePrintLine("ERROR: Bad input. No effect.");\
		}\
	}else{\
		easyForcePrintLine("\"%s\" is %g",#CVarName,global_##CVarName);\
	}\
	return;\
}



//NOTE - that call used to be EASY_CVAR_UPDATE_SERVER_DEBUGONLY_CLIENTONLY.
#define EASY_CVAR_HIDDEN_ACCESS_DEBUGONLY_CLIENTONLY(CVarName, CVarNameLower, ID)\
if( FStrEq(pcmdRefinedRef, #CVarNameLower)  ){\
	CBasePlayer* tempplayer = GetClassPtr((CBasePlayer *)pev);\
	const char* arg1ref = CMD_ARGV(1);\
	const char* arg2ref = CMD_ARGV(2);\
	if(!isStringEmpty(arg2ref)){\
		easyForcePrintLine("ERROR: Only one arg accepted following the CVar. No more than 1 space total.");\
	}if(!isStringEmpty(arg1ref)){\
		float tempF = 0;\
		try{\
			tempF = tryStringToFloat(arg1ref);\
			CUSTOM_CLIENT_CALL(ID, tempF)\
		}catch(int){\
			easyForcePrintLine("ERROR: Bad input. No effect.");\
		}\
	}else{\
		EASY_CVAR_PRINT_CLIENTONLY(ID)\
	}\
	return;\
}


/*
#define EASY_CVAR_CLIENTSENDOFF(CVarName, ID)\
	else if(argID == ID){\
		global2_##CVarName = arg;\
		if(global2_hiddenMemPrintout)easyForcePrintLine("CVAR DEBUG: Client: found ID %d. Set CVar %s to %g", ID, #CVarName, arg);\
	}
*/



#define EASY_CVAR_PRINT_CLIENTONLY(ID)\
	if(pev != NULL){\
		MESSAGE_BEGIN( MSG_ONE, gmsgPrintClientCVar, NULL, pev );\
			WRITE_SHORT( ID );\
		MESSAGE_END();\
	}



//NOTE: for now, only 
//EASY_CVAR_DECLARATION_SERVER_DEBUGONLY_CLIENTSENDOFF
//EASY_CVAR_UPDATE_SERVER_DEBUGONLY_CLIENTSENDOFF
//~varry from the default.

//Identical to non-clientsendoff.
#define EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName)\
	float global2_##CVarName = DEFAULT_##CVarName;

#define EASY_CVAR_DECLARATION_CLIENT_DEBUGONLY_CLIENTONLY(CVarName)\
	float global2_##CVarName = DEFAULT_##CVarName;



//Identical to non-clientsendoff.
#define EASY_CVAR_UPDATE_CLIENT_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName) DUMMY
#define EASY_CVAR_UPDATE_CLIENT_DEBUGONLY_CLIENTONLY(CVarName) DUMMY


//CHANGED
#define EASY_CVAR_DECLARATION_SERVER_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName)\
	float global_##CVarName = DEFAULT_##CVarName;\
	float globalMEM_##CVarName = DEFAULT_##CVarName;


//Nothing for this version, no serverside storage.
#define EASY_CVAR_DECLARATION_SERVER_DEBUGONLY_CLIENTONLY(CVarName) DUMMY


//MODDD - is this okay?
//MSG_BROADCAST, gmsgUpdateClientCVar
//MSG_ONE, gmsgRetrieveFOV, NULL, pev

//pev


//NOTE: currently unsure of the best way to do handling sending off client CVars.

//Looks like if it's at startup (pev is "null"), you can do without the PEV ("Broadcast").
//Otherwise, just involve the PEV.   (PEV is the player's own vars?).
//(if-thens are presently in reverse order of the aforementioned)



//CHANGED. NOTICE: is the 2nd NULL in the MESAGE_BEGIN for PEV ok, yes or no?!
#define EASY_CVAR_UPDATE_SERVER_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName, ID)\
	if(globalMEM_##CVarName != global_##CVarName || globalPSEUDO_queueClientSendoff){\
		globalMEM_##CVarName = global_##CVarName;\
		if(pev != NULL){\
			MESSAGE_BEGIN( MSG_ALL, gmsgUpdateClientCVar, NULL);\
				WRITE_SHORT( ID);\
				WRITE_SHORT( global_##CVarName*100);\
			MESSAGE_END();\
		}\
	}

//TEST - dummying the default one out.
#define EASY_CVAR_UPDATE_SERVER_DEBUGONLY_CLIENTONLY(CVarName, ID) DUMMY


#define CUSTOM_CLIENT_CALL(ID, argVal)\
	if(pev != NULL){\
		MESSAGE_BEGIN( MSG_ONE, gmsgUpdateClientCVar, NULL, pev );\
			WRITE_SHORT( ID);\
			WRITE_SHORT( argVal*100);\
		MESSAGE_END();\
	}


/*
#define EASY_CVAR_UPDATE_SERVER_DEBUGONLY_CLIENTSENDOFF(CVarName, ID)\
	if(globalMEM_##CVarName != global_##CVarName){\
		globalMEM_##CVarName = global_##CVarName;\
		if(pev != NULL){\
			MESSAGE_BEGIN( MSG_ONE, gmsgUpdateClientCVar, NULL, pev );\
				WRITE_SHORT( ID);\
				WRITE_SHORT( global_##CVarName*100);\
			MESSAGE_END();\
		}else{\
			MESSAGE_BEGIN( MSG_BROADCAST, gmsgUpdateClientCVar );\
				WRITE_SHORT( ID);\
				WRITE_SHORT( global_##CVarName*100);\
			MESSAGE_END();\
		}\
	}
*/


//no CVar Redos.


//is that okay?  Implied only for the server (global_...).
#define EASY_CVAR_SET_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName, valueV)\
	global_##CVarName = valueV;


//this works a little differently!
#define EASY_CVAR_SET_DEBUGONLY_CLIENTONLY(ID, valueV) CUSTOM_CLIENT_CALL(ID##_ID, valueV)



#ifdef CLIENT_DLL
#define EASY_CVAR_EXTERN_DEBUGONLY(CVarName) EASY_CVAR_EXTERN(CVarName)
#define EASY_CVAR_EXTERN_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName) EASY_CVAR_EXTERN(CVarName)
#define EASY_CVAR_EXTERN_DEBUGONLY_CLIENTONLY(CVarName) EASY_CVAR_EXTERN(CVarName)
#else
#define EASY_CVAR_EXTERN_DEBUGONLY(CVarName) EASY_CVAR_EXTERN(CVarName)
#define EASY_CVAR_EXTERN_DEBUGONLY_CLIENTSENDOFF_BROADCAST(CVarName) EASY_CVAR_EXTERN(CVarName)\
	extern float globalMEM_##CVarName;
#define EASY_CVAR_EXTERN_DEBUGONLY_CLIENTONLY(CVarName) EASY_CVAR_EXTERN(CVarName)
#endif



#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/////////////////////////////////////////////////////////////////////////////////////////////////////////






#define EASY_CVAR_EXTERN_CLIENT(CVarName)\
	extern float global2_##CVarName;

#define EASY_CVAR_EXTERN_SERVER(CVarName)\
	extern float global_##CVarName;





//!!!! REPLACE
//#define EASY_CVAR_PRINTIF_PRE(requirementName, weee)\
//if(global_##requirementName == 1){\
//	weee;\
//}
#define EASY_CVAR_PRINTIF_PRE(requirementName, weee)\
if(EASY_CVAR_GET(requirementName) == 1){\
	weee;\
}



#define EASY_CVAR_GET_CLIENT(CVarName)\
	global2_##CVarName;

#define EASY_CVAR_GET_SERVER(CVarName)\
	global_##CVarName;





#ifdef CLIENT_DLL

#define EASY_CVAR_HIDDEN_SAVE(CVarName)\
	fprintf(myFile, "%s %g\n", QUOTE(CVarName), global2_##CVarName);

#define EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(CVarName) EASY_CVAR_HIDDEN_SAVE(CVarName)

#define EASY_CVAR_HIDDEN_SAVE_SERVERONLY(CVarName)
#else

#define EASY_CVAR_HIDDEN_SAVE(CVarName)\
	fprintf(myFile, "%s %g\n", QUOTE(CVarName), global_##CVarName);

#define EASY_CVAR_HIDDEN_SAVE_CLIENTONLY(CVarName)
#define EASY_CVAR_HIDDEN_SAVE_SERVERONLY(CVarName) EASY_CVAR_HIDDEN_SAVE(CVarName)
#endif


	//myfile << QUOTE(CVarName) << " " << global_##CVarName << "\n";
	//Now now, that's not the C way of doing it. No C++ for you scallywags!
	//

/*
#define EASY_CVAR_HIDDENLOAD(CVarName, CVarNameLower)\
	else if(strcmp(identifier, QUOTE(CVarNameLower) ) == 0){\
		global_##CVarName = value;\
	}
*/



#ifdef CLIENT_DLL

//!!!!
#define EASY_CVAR_HIDDEN_LOAD(CVarName, CVarNameLower)\
	//if(strcmp(identifier, QUOTE(CVarNameLower) ) == 0){\
	//	global2_##CVarName = value;\
	//	return;\
	//}

#define EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(CVarName, CVarNameLower) EASY_CVAR_HIDDEN_LOAD(CVarName, CVarNameLower)
#define EASY_CVAR_HIDDEN_LOAD_SERVERONLY(CVarName, CVarNameLower)
#else


//!!!!
#define EASY_CVAR_HIDDEN_LOAD(CVarName, CVarNameLower)\
	//if(strcmp(identifier, QUOTE(CVarNameLower) ) == 0){\
	//	global_##CVarName = value;\
	//	return;\
	//}

#define EASY_CVAR_HIDDEN_LOAD_CLIENTONLY(CVarName, CVarNameLower)
#define EASY_CVAR_HIDDEN_LOAD_SERVERONLY(CVarName, CVarNameLower) EASY_CVAR_HIDDEN_LOAD(CVarName, CVarNameLower)
#endif

//COMPILE COMPATABILITY: is a blank #define like this okay?
#define DUMMY



//some info needed first like the size of the array.
#include "cvar_custom_info.h"

//The CVar calls themselves. Not to be edited by hand by the faint of heart. Or by anyone really.
#include "cvar_custom_list.h"



#endif //END OF CVAR_CUSTOM_H


