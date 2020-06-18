




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// preliminary.
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CLIENT_DLL
// Yes, extdll.h clientside.  Hey, hl_weapons.cpp did it first.
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "util_shared.h"
#include "util_printout.h"

#include "cl_dll.h"
//#include "hud_iface.h"
#include "r_efx.h"
#include "ev_hldm.h"

#include <string.h>
//#include <stdio.h>
//#include <stdarg.h>

//#include "extdll.h"
//#include "basemonster.h"

#else
//SERVER
#include "extdll.h"
#include "cbase.h"
#include "util.h"
#include "util_shared.h"
#include "util_printout.h"

#include "enginecallback.h"
#include "progdefs.h"

#include "gamerules.h"

//??? necssary at all?
/*
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include <time.h>
#include "shake.h"
#include "decals.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
*/

#endif

//SHARED STUFF BELOW
#include "weapons.h"


//MODDD - thanks,
//http://stackoverflow.com/questions/18100097/portable-way-to-check-if-directory-exists-windows-linux-c
#include <sys/types.h>
#include <sys/stat.h>

struct stat info;


//MODDD - used by determineHiddenMemPath.
#include <iostream>
#include <fstream>
#include <string>
using namespace std;








#ifdef CLIENT_DLL



	//from cl_dll/ev_hldm.cpp, needed for clientside UTIL_Sparks2 to call.

	//extern cl_enginefunc_t gEngfuncs;

#else
	//SERVER
	extern float globalPSEUDO_cl_hornetspiral;
	EASY_CVAR_EXTERN(cl_hornetspiral)
#endif


#ifdef CLIENT_DLL
		//huh. global2PSEUDO_gamePath
#define GET_GAME_PATH_VAR globalPSEUDO_gamePath
#define DEFAULT_GET_GAME_DIR(receiver)\
	const char* tempGameDirRef = gEngfuncs.pfnGetGameDirectory();\
	strcpy(gameName, tempGameDirRef);
#else
#define GET_GAME_PATH_VAR globalPSEUDO_gamePath
#define DEFAULT_GET_GAME_DIR(receiver) GET_GAME_DIR(receiver);
#endif


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Content.
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


extern char GET_GAME_PATH_VAR[];


EASY_CVAR_EXTERN_MASS




// Used to be only in dlls/weapons.cpp (serverside only file).
// This means the server and client has its own "giAmmoIndex", for use through AddAmmoNameToAmmoRegistry
// calls.  It is good practice to set giAmmoIndex to 0 before potentially making a lot of calls to that
// method, however indirectly (such as before precaching weapons).
int giAmmoIndex = 0;

// Also from weapons.cpp, since we can refer to these in both client/server now.
// These are implementations of CBasePlayerItem's two static arrays.
ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];
AmmoInfo CBasePlayerItem::AmmoInfoArray[MAX_AMMO_SLOTS];

// flag to disable extra deploy sounds from weapons spawned by cheats, when set to TRUE before giving
// items and back to FALSE when done. Shared so that the client won't complain that this is missing,
// weapon script is shared. Yes this doesn't get transmitted to the client, but the client doesn't
// seem to do Deploy() calls anyways or doesn't play sounds called through there. In any case, no sync
// is needed as the server (player.cpp) sets this flag before doing some give's, and only the server-
// side deploys are  affected as needed which go on to play the sound for the client of course. It works.
BOOL globalflag_muteDeploySound = FALSE;








//...oh.  This was only ever referred to in weapons.cpp.  Well, whoops. Doesn't hurt to be here (shared) I suppose.
//=========================================================
// MaxAmmoCarry - pass in a name and this function will tell
// you the maximum amount of that type of ammunition that a 
// player can carry.
//=========================================================
int MaxAmmoCarry(int iszName)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo1 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo1))
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo1;
		if (CBasePlayerItem::ItemInfoArray[i].pszAmmo2 && !strcmp(STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo2))
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo2;
	}

	ALERT(at_console, "MaxAmmoCarry() doesn't recognize '%s'!\n", STRING(iszName));
	return -1;
}



// Precaches the ammo and queues the ammo info for sending to clients
void AddAmmoNameToAmmoRegistry(const char* szAmmoname)
{
	// make sure it's not already in the registry
	for (int i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		if (!CBasePlayerItem::AmmoInfoArray[i].pszName)
			continue;

		if (stricmp(CBasePlayerItem::AmmoInfoArray[i].pszName, szAmmoname) == 0)
			return; // ammo already in registry, just quit
	}


	giAmmoIndex++;
	ASSERT(giAmmoIndex < MAX_AMMO_SLOTS);
	if (giAmmoIndex >= MAX_AMMO_SLOTS)
		giAmmoIndex = 0;

	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].pszName = szAmmoname;
	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iId = giAmmoIndex;   // yes, this info is redundant
}//END OF AddAmmoNameToAmmoRegistry










	
//MODDD
const char* FClassname(CBaseEntity* derp){
	if(derp != NULL){
		return STRING(derp->pev->classname);
	}else{
		return "NULL";
	}
}



BOOL stringStartsWith(const char* source, const char* startswith){

	for(int i = 0; i < 127; i++){
		if(startswith[i] == '\0'){
			//if we made it this far (end of "startswith"), something went right.
			return TRUE;
		}
		if(source[i] == '\0'){
			//ending prematurely, source is less than startsWith and followed up to this point.
			return FALSE;
		}

		if(source[i] != startswith[i]){
			return FALSE;
		}
	}
	return TRUE;
}






BOOL checkMatch(const char* src1, char* src2){
	return checkMatch(src1, src2, 127);

}


BOOL checkMatch(const char* src1, char* src2, int size){

	BOOL matching = TRUE;
	int i;
	for(i = 0; i < size; i++){
		if(src1[i] != src2[i]){
			//no match? we're done.
			matching = FALSE;
			break;
		}
		if(src1[i] == '\0'){
			//since they are equal and either is the termination, finish gracefully.
			break;
		}
	}

	return matching;
}





BOOL checkMatchIgnoreCase(const char* src1, char* src2){
	return checkMatchIgnoreCase(src1, src2, 127);

}


BOOL checkMatchIgnoreCase(const char* src1, char* src2, int size){

	BOOL matching = TRUE;
	int i;
	for(i = 0; i < size; i++){

		if(src1[i] == src2[i]){


		}else if(src1[i] >= 'A' && src1[i] <= 'Z'){
			//try again with the lowercase char:
			char lowerChar = ((char)  (((int)(src1[i])) + 32) );
			
			if(lowerChar == src2[i]){
				//okay.
			}else{
				matching = FALSE;
				break;
			}
		}else{
			matching = FALSE;
			break;
		}

		if(src1[i] == '\0'){
			//since they are equal and either is the termination, finish gracefully.
			break;
		}
	}

	return matching;
}








const char* tryIntToString(int arg_src){

	//WARNING - they should provide a buffer instead.
	static char chrReturn[13];
	itoa(arg_src, chrReturn, 10);
	return chrReturn;
}




int tryStringToInt(const char* arg_src){
	int i = 0;

	if(arg_src == NULL){
		//can't do anything.
		throw 13;
	}

	while(i < 128){

		if(arg_src[i] == '\0'){

			if(i > 0){
				char tempChar[128];
				strncpy( &tempChar[0], &arg_src[0], i );
				tempChar[i] = '\0';
				int result = atoi(tempChar);

				return result;

			}else{
				//the very first character is the termination '\0' ?  Error.
				throw 13;
			}
		}

		if(arg_src[i] >= '0' && arg_src[i] <= '9'){
			//pass.
		}else if(i == 0 && arg_src[i] == '-'){
			//allow the minus sign, if it is the first character.
		}else{
			//invalid character!
			throw 13;
		}

		i++;

	}//END OF while(...)

	//loop above ended (did not return or throw)?  Too long!
	throw 13;

}






float tryStringToFloat(const char* arg_src){
	int i = 0;

	if(arg_src == NULL){
		//can't do anything.
		throw 13;
	}


	int decimalCount = 0;

	while(i < 128){

		if(arg_src[i] == '\0'){

			if(i > 0){
				char tempChar[128];
				strncpy( &tempChar[0], &arg_src[0], i );
				tempChar[i] = '\0';
				float result = (float)atof(tempChar);

				return result;

			}else{
				//the very first character is the termination '\0' ?  Error.
				throw 13;
			}
		}

		if(arg_src[i] >= '0' && arg_src[i] <= '9'){
			//pass.

		}else if(arg_src[i] == '.'){

			if(decimalCount > 0){
				//FAIL!
				throw 13;
			}
			decimalCount++;

		}else if(i == 0 && arg_src[i] == '-'){
			//allow the minus sign, if it is the first character.
		}else{
			//invalid character!
			throw 13;
		}

		i++;

	}//END OF while(...)

	//loop above ended (did not return or throw)?  Too long!
	throw 13;

}


const char* tryFloatToString(float arg_src){
	
	//WARNING - they should provide a buffer instead.
	static char chrReturn[13] ;
	//itoa(arg_src, chrReturn, 10);

	sprintf(chrReturn, "%f", 3.0f);


	return chrReturn;
}


void tryFloatToStringBuffer(char* dest, float arg_src){
	

	//itoa(arg_src, chrReturn, 10);

	sprintf(dest, "%f", arg_src);

}





//thanks, 
//http://stackoverflow.com/questions/11656532/returning-an-array-using-c


void lowercase(char* src){
	// assume length of 128.
	// Can end anytime the terminating character is found.
	for(int i = 0; i < 127; i++){
		// in other words, this character of the sent text is between "A" and "Z" (capital).
		if(src[i] >= 65 && src[i] <= 90){
			// adding 32 to the character makes it lowercased, since the range of lowercased chars is 97 'a' to 122 'z'.
			src[i] = (char)((int)src[i] + 32);
			//src[i] = src[i];
			//src[i] = src[i];
		}else{
			// not a letter.  Just leave it.
			src[i] = src[i];
		}
		if(src[i] == '\0'){
			//end.
			break;
			//return &src[0];
		}
	}
	src[127] = '\0';
	//return &src[0];
}



void lowercase(char* src, int size){
	BOOL broke = FALSE;
	// Yes, check all at size-1 because the very last spot [size-1] itself 
	// should be a terminating character if all the size is used.
	// Ending early at finding the terminating character first is possible
	// of course.
	for(int i = 0; i < size-1; i++){
		if(src[i] >= 65 && src[i] <= 90){
			src[i] = (char)((int)src[i] + 32);
		}else{
			src[i] = src[i];
		}
		if(src[i] == '\0'){
			broke = TRUE;
			break;
		}
	}
	if(!broke){
		src[size-1] = '\0';
	}
}













//Thanks!
//https://stackoverflow.com/questions/554204/where-is-round-in-c
float roundToNearest(float num) {
    return (num > 0.0f) ? floor(num + 0.5f) : ceil(num - 0.5f);
}





//NOTE: non-inclusive end index.
void UTIL_substring(char* dest, const char* src, int startIndex, int endIndex){

	int i = 0;

	if(endIndex == -1){
		while(true){
			const char& thisChar = src[i + startIndex];
			if(thisChar == '\0'){
				//done.
				break;
			}else{
				dest[i] = thisChar;
				i++;
			}
		}
	}else{
		while(i+startIndex < endIndex){
			dest[i] = src[i + startIndex];
			i++;
		}

	}
	//finally, wherever "i" left off.
	dest[i] = '\0';

	

}


int UTIL_findCharFirstPos(const char* search, char toFind){
	int i = 0;
	while(TRUE){
		if(search[i] == '\0'){
			//give up.
			return -1;
		}else if(search[i] == toFind){
			return i;
		}
		i++;
	}//END OF while(TRUE)
	//???
	return -1;
}

void UTIL_appendTo(char* dest, const char* add, int appendStartLoc){
	int lengthOfToAdd = lengthOfString(add);
	strncpy( &dest[appendStartLoc], add, lengthOfToAdd );

}

void appendTo(char* dest, const char* add, int* refIndex){
	int lengthOfToAdd = lengthOfString(add);
	strncpy( &dest[*refIndex], add, lengthOfToAdd );

	//probably implied.  NOPE?!
	//dest[*refIndex + lengthOfToAdd] = '\0';

	*refIndex += (lengthOfToAdd);
}

void appendToAndTerminate(char* dest, const char* add, int* refIndex){
	int lengthOfToAdd = lengthOfString(add);
	strncpy( &dest[*refIndex], add, lengthOfToAdd );

	//probably implied.  NOPE?!
	dest[*refIndex + lengthOfToAdd] = '\0';

	*refIndex += (lengthOfToAdd);
}



void appendTo(char* dest, const char* add, int* refIndex, char endCharacter){
	int lengthOfToAdd = lengthOfString(add);
	strncpy( &dest[*refIndex], add, lengthOfToAdd );

	dest[*refIndex + lengthOfToAdd] = endCharacter;

	*refIndex += (lengthOfToAdd + 1);
}


void appendToAndTerminate(char* dest, const char* add, int* refIndex, char endCharacter){
	int lengthOfToAdd = lengthOfString(add);
	strncpy( &dest[*refIndex], add, lengthOfToAdd );

	dest[*refIndex + lengthOfToAdd] = endCharacter;
	dest[*refIndex + lengthOfToAdd + 1] = '\0';

	*refIndex += (lengthOfToAdd + 1);
}


void appendTo(char* dest, const int numb, int* refIndex){
	
	char numbChar[4];
	itoa(numb, numbChar, 10);
	appendTo(dest, numbChar, refIndex);

}

void appendToAndTerminate(char* dest, const int numb, int* refIndex){
	
	char numbChar[4];
	itoa(numb, numbChar, 10);
	appendToAndTerminate(dest, numbChar, refIndex);

}


void appendTo(char* dest, const int numb, int* refIndex, char endCharacter){
	
	char numbChar[4];
	itoa(numb, numbChar, 10);
	appendTo(dest, numbChar, refIndex, endCharacter);

}

void appendToAndTerminate(char* dest, const int numb, int* refIndex, char endCharacter){
	
	char numbChar[4];
	itoa(numb, numbChar, 10);
	appendToAndTerminate(dest, numbChar, refIndex, endCharacter);

}



void appendTo(char* dest, const char add, int* refIndex){
	dest[*refIndex] = add;

	*refIndex += (1);
}

void appendToAndTerminate(char* dest, const char add, int* refIndex){
	dest[*refIndex] = add;
	dest[*refIndex + 1] = '\0';

	*refIndex += (1);
}


void strncpyTerminate(char* dest, const char* send, int arg_length){
	strncpy(dest, send, arg_length);
	dest[arg_length] = '\0';
}








void copyString(const char* src, char* dest){
	//no size given?  "127" is the default.
	copyString(src, dest, 127);
}



void copyString(const char* src, char* dest, int size){

	
	BOOL queueBreak = FALSE;
	int i;
	for(i = 0; i < size; i++){
		
		if(src[i] == '\0'){
			//we will be done.
			queueBreak = TRUE;
		}
		dest[i] = src[i];

		if(queueBreak){
			break;
		}
	}

	//all the way to the end, never "terminated"?
	if(queueBreak == FALSE){
		dest[size-1] = '\0';
		//last char will have to clip and say '\0' to terminate.
	}

}


void UTIL_appendToEnd(char* dest, const char* add){

	int lengthOfDest = lengthOfString(dest);
	int lengthOfToAdd = lengthOfString(add);

	//aaaabb
	//0123456
	//|a| = 4
	//|b| = 2

	//easyForcePrintLine("LENGTH TO ADD: %s ::: %d", add, lengthOfToAdd);
	strncpy( &dest[lengthOfDest], add, lengthOfToAdd );
	
	//OK, c++... I had more faith in you than that.
	dest[lengthOfDest + lengthOfToAdd] = '\0';
}


int lengthOfString(const char* src){
	int i = 0;

	while(i < 500){
		if(src[i] == '\0'){
			//this is length, done.
			return i;
		}else{
			//something that isn't the end? continue.
		}

		i++;
	}
	//ERROR CODE.  Either never ended or too long.
	return -1;
}


int lengthOfString(const char* src, int storeSize){
	//note that "storeSize" is the (optional) size of the "src" array we're printing to.
	//If we try to surpass this, cut off the string at the cap index of
	//cap = storeSize - 2;
	//why?  because "storeSize - 1", the last legal index for characters, needs to be '\0', the null terminating character.
	//this leaves storeSize - 2 as the last legal index for normal (non-terminating?) characters.

	int i = 0;

	while(i <= storeSize - 2){
		if(src[i] == '\0'){
			//this is length, done.
			return i;
		}else{
			//something that isn't the end? continue.
		}

		i++;
	}
	
	//hm, error code?  hard to say.  Cutoff can happen though.
	//not editing!
	//src[i] = '\0'; 

	return i;
}


BOOL isStringEmpty(const char* arg_src){
	if(arg_src == NULL || arg_src[0] == '\0'){
		//empty!
		return TRUE;
	}else{
		return FALSE;
	}
}


BOOL stringEndsWith(const char* arg_src, const char* arg_endsWith){
	int i = 0;
	int lastPos;
	
	int srcLength = lengthOfString(arg_src);
	int endsWithLength = lengthOfString(arg_endsWith);

	if(srcLength < endsWithLength){
		//can't possibly end with something longer than itself (arg_src shorter than what it ends with)
		return FALSE;
	}

	//start place to compare arg_src to.
	int srcIndex = srcLength - endsWithLength;

	for(i = 0; i < endsWithLength; i++){
		if(arg_src[srcIndex + i] == arg_endsWith[i]){
			//pass...
		}else{
			//No match!
			return FALSE;
		}
	}
	//made it past the loop?  Then we're done.
	return TRUE;
}

BOOL stringEndsWithIgnoreCase(const char* arg_src, const char* arg_endsWith){
	int i = 0;
	int lastPos;
	
	int srcLength = lengthOfString(arg_src);
	int endsWithLength = lengthOfString(arg_endsWith);

	if(srcLength < endsWithLength){
		//can't possibly end with something longer than itself (arg_src shorter than what it ends with)
		return FALSE;
	}

	//derpyherp
	//herp

	//start place to compare arg_src to.
	int srcIndex = srcLength - endsWithLength;

	for( i = 0; i < endsWithLength; i++){



		if(arg_src[srcIndex + i] == arg_endsWith[i]){
			//pass...
			//capital check.  Characters may be implicitly cast to integers (as values on the ASCII table)
			//~anything between corresponding #'s for 'A' and 'Z' is capital, of course.
		}else if(arg_src[srcIndex + i] >= 'A' && arg_src[srcIndex + i] <= 'Z'){
			
			//let's try making the source character lowercase, compare again:

			//add 32 to a capitol ASCII value (range is 65 = A, 90 = Z) to change to range 97 = a, 122 = z.
			char lowerChar = ((char)  (((int)(arg_src[srcIndex + i])) + 32) );
			//easyPrintLine("lowercase check? %d %d %d", arg_src[srcIndex + i], lowerChar, arg_endsWith[i]);
			BOOL passed = (lowerChar == arg_endsWith[i]);

			if(passed){
				//continue.
			}else{
				//even lowercase'd, we still failed.
				//easyPrintLine("awnaw!!: %d %d", arg_src[srcIndex + i], arg_endsWith[i]);
				//easyPrintLine("craaap!!: %d %d", lowerChar, arg_endsWith[i]);
			
				return FALSE;
			}

		}else{
			//No match!
			//easyPrintLine("nomatch: %d %d", arg_src[srcIndex + i], arg_endsWith[i]);
			return FALSE;
		}
	}
	//made it past the loop?  Then we're done.
	return TRUE;
}
















//check the existence of this subpath under the game's directory.
//That is, arg_subdir (let's say X ) in here:
//  C:/.../<half life folder>/<mod name>/X
BOOL checkSubFileExistence(const char* arg_subdir){
	if(GET_GAME_PATH_VAR[0] == '\0'){
		return FALSE;
	}

	char tempCheck[MAX_PATH];
	tempCheck[0] = '\0';

	copyString(GET_GAME_PATH_VAR, tempCheck);

	/*
#ifdef CLIENT_DLL
	easyForcePrintLine("CLIENT: checkSubFileExistence call. Gamepath:|%s| subdir:|%s|", GET_GAME_PATH_VAR, arg_subdir);
#else
	easyForcePrintLine("SERVER: checkSubFileExistence call. Gamepath:|%s| subdir:|%s|", GET_GAME_PATH_VAR, arg_subdir);
#endif
	*/

	/*
	for(int i = 0; i < 500; i++){
		easyForcePrintLine("%d: %i:%c, %i:%c", i, globalPSEUDO_gamePath[i], globalPSEUDO_gamePath[i], tempCheck[i], tempCheck[i]);

		BOOL doBreak = FALSE;
		if(globalPSEUDO_gamePath[i] == '\0'){
			easyForcePrintLine("GP FAIL");
			doBreak = TRUE;
		}
		if(tempCheck[i] == '\0'){
			easyForcePrintLine("TC FAIL");
			doBreak = TRUE;
		}
		if(doBreak)break;
	}
	*/



	UTIL_appendToEnd(tempCheck, arg_subdir);

	/*
	for(int i = 0; i < 500; i++){
		easyForcePrintLine("-%d: %i:%c", i, arg_subdir[i], arg_subdir[i]);
		if(arg_subdir[i] == '\0'){
			break;
		}
	}

	BOOL gpBrokenYet = FALSE;

	for(int i = 0; i < 500; i++){

		
		if(globalPSEUDO_gamePath[i] == '\0'){
			gpBrokenYet = TRUE;
		}

		
		if(gpBrokenYet == FALSE){
			easyForcePrintLine("%d: %i:%c, %i:%c", i, globalPSEUDO_gamePath[i], globalPSEUDO_gamePath[i], tempCheck[i], tempCheck[i]);
		}else{
			easyForcePrintLine("%d: %i:%c, %i:%c", i, 'e', 'e', tempCheck[i], tempCheck[i]);
		}
		BOOL doBreak = FALSE;
		if(tempCheck[i] == '\0'){
			easyForcePrintLine("TC FAIL");
			doBreak = TRUE;
		}
		if(doBreak)break;
	}
	*/


	/*
#ifdef CLIENT_DLL
	easyForcePrintLine("CLIENT: checkSubFileExistence result: |%s|", tempCheck);
#else
	easyForcePrintLine("SERVER: checkSubFileExistence result: |%s|", tempCheck);
#endif
	*/


	//string lineRaw;
	FILE* myFile = fopen(tempCheck, "r" );

	//easyForcePrintLine("OOOOOOOOO %s", tempCheck);

	if (myFile)
	{
		fclose(myFile);
		//"memez" exists?  assume it has the files.
		return TRUE;
	}else{
		//myfile.close();   //???

		//no memes.
		return FALSE;
	}
}























BOOL globalPSEUDO_iCanHazMemez = FALSE;

char globalPSEUDO_halflifePath[512];
char globalPSEUDO_gamePath[512];
char globalPSEUDO_valveGamePath[512];
char globalPSEUDO_hiddenMemPath[512];




void determineHiddenMemPath(void){


	globalPSEUDO_halflifePath[0] = '\0';
	globalPSEUDO_gamePath[0] = '\0';
	globalPSEUDO_valveGamePath[0] = '\0';
	globalPSEUDO_hiddenMemPath[0] = '\0';

	//WCHAR;
	//LPSTR buffer;
	//char buffer[MAX_PATH];

	//Seems to work fine for getting an absolute path to the hl.exe file, but this is unnecessary.
	/*
    GetModuleFileName(NULL, globalPSEUDO_halflifePath, MAX_PATH) ;
	
	//easyForcePrintLine("Executable Path: %s", globalPSEUDO_halflifePath);

	//we expect this to be a path straight to the EXE, so cut off at the last slash.

	int recentSlashPos = -1;
	int i = 0;
	while(i < MAX_PATH){
		if(globalPSEUDO_halflifePath[i] == '\0'){
			break;
		}else if(globalPSEUDO_halflifePath[i] == '\\' || globalPSEUDO_halflifePath[i] == '/'){
			recentSlashPos = i;
		}
		i++;
	}
	
	//the "hl.exe" in here is unhelpful, cut it (substring, cut off from the last slash onwards)
	//UTIL_substring(globalPSEUDO_halflifePath, 0, recentSlashPos + 1);
	globalPSEUDO_halflifePath[recentSlashPos + 1] = '\0';  //termination... same effect.

	//what we have is "globalPSEUDO_halflifePath" = "C:\...\<half life folder>\"
	*/

	char gameName[100];
	gameName[0] = '\0';
	DEFAULT_GET_GAME_DIR(gameName);
	UTIL_appendToEnd(gameName, "\\");
	// "<modname>\" or "valve\" for vanilla
	
	char valveGameName[100] = "valve\\\0";


	copyString(globalPSEUDO_halflifePath, globalPSEUDO_gamePath);
	UTIL_appendToEnd(globalPSEUDO_gamePath, gameName);
	copyString(globalPSEUDO_halflifePath, globalPSEUDO_valveGamePath);
	UTIL_appendToEnd(globalPSEUDO_valveGamePath, valveGameName);

	copyString(globalPSEUDO_gamePath, globalPSEUDO_hiddenMemPath);

#ifdef CLIENT_DLL
	UTIL_appendToEnd(globalPSEUDO_hiddenMemPath, "absZeroMemClient.txt"); //recentSlashPos + 1);
#else
	UTIL_appendToEnd(globalPSEUDO_hiddenMemPath, "absZeroMem.txt"); //recentSlashPos + 1);
#endif
	
		
	/*
#ifdef CLIENT_DLL
	easyForcePrintLine("CLIENT: helpme.txt SEARCH?" );
#else
	easyForcePrintLine("SERVER: helpme.txt SEARCH?" );
#endif
	*/

	
	if(checkSubFileExistence("helpme.txt")){
		EASY_CVAR_SET_DEBUGONLY(hiddenMemPrintout, 1);
		
#ifdef CLIENT_DLL
			easyForcePrintLine("CLIENT: helpme.txt found - hiddenMemPrintout enabled!" );
#else
			easyForcePrintLine("SERVER: helpme.txt found - hiddenMemPrintout enabled!" );
#endif
			
	}else{
		//???
	}

	if(EASY_CVAR_GET(hiddenMemPrintout) == 1){
		easyForcePrintLine("HALF LIFE PATH: %s", globalPSEUDO_halflifePath);
		easyForcePrintLine("GAME PATH: %s", globalPSEUDO_gamePath);
		easyForcePrintLine("HIDDEN MEM PATH: %s", globalPSEUDO_hiddenMemPath);
	}
	
}//END OF determineHiddenPath





BOOL checkValveSubFileExistence(const char* arg_subdir){
	if(globalPSEUDO_valveGamePath[0] == '\0'){
		return FALSE;
	}
	char tempCheck[MAX_PATH];
	tempCheck[0] = '\0';

	copyString(globalPSEUDO_valveGamePath, tempCheck);
	
	UTIL_appendToEnd(tempCheck, arg_subdir);

	string lineRaw;
	FILE* myFile = fopen(tempCheck, "r" );

	//easyForcePrintLine("OOOOOOOOO %s", tempCheck);

	if (myFile)
	{
		fclose(myFile);
		return TRUE;
	}else{
		return FALSE;
	}
}







//check the existence of this subpath under the game's directory.
//That is, arg_subdir (let's say X ) in here:
//  C:/.../<half life folder>/<mod name>/X
BOOL checkSubDirectoryExistence(const char* arg_subdir){
	if(globalPSEUDO_gamePath[0] == '\0'){
		return FALSE;
	}

	char tempCheck[MAX_PATH];
	tempCheck[0] = '\0';

	copyString(globalPSEUDO_gamePath, tempCheck);


	/*
	for(int i = 0; i < 500; i++){
		easyForcePrintLine("%d: %i:%c, %i:%c", i, globalPSEUDO_gamePath[i], globalPSEUDO_gamePath[i], tempCheck[i], tempCheck[i]);

		BOOL doBreak = FALSE;
		if(globalPSEUDO_gamePath[i] == '\0'){
			easyForcePrintLine("GP FAIL");
			doBreak = TRUE;
		}
		if(tempCheck[i] == '\0'){
			easyForcePrintLine("TC FAIL");
			doBreak = TRUE;
		}
		if(doBreak)break;
	}
	*/



	UTIL_appendToEnd(tempCheck, arg_subdir);

	/*
	for(int i = 0; i < 500; i++){
		easyForcePrintLine("-%d: %i:%c", i, arg_subdir[i], arg_subdir[i]);
		if(arg_subdir[i] == '\0'){
			break;
		}
	}

	BOOL gpBrokenYet = FALSE;

	for(int i = 0; i < 500; i++){

		
		if(globalPSEUDO_gamePath[i] == '\0'){
			gpBrokenYet = TRUE;
		}

		
		if(gpBrokenYet == FALSE){
			easyForcePrintLine("%d: %i:%c, %i:%c", i, globalPSEUDO_gamePath[i], globalPSEUDO_gamePath[i], tempCheck[i], tempCheck[i]);
		}else{
			easyForcePrintLine("%d: %i:%c, %i:%c", i, 'e', 'e', tempCheck[i], tempCheck[i]);
		}
		BOOL doBreak = FALSE;
		if(tempCheck[i] == '\0'){
			easyForcePrintLine("TC FAIL");
			doBreak = TRUE;
		}
		if(doBreak)break;
	}
	*/


	//NOTE: see the "Thanks, ..." stack overflow link near the top of util.cpp for the source of this approach.  Nifty.
	if(stat( tempCheck, &info ) != 0)
        return FALSE;
    else if(info.st_mode & S_IFDIR)
        return TRUE;
    else
        return FALSE;


}









void processLoadHiddenCVarLine(const char* aryChrLineBuffer){

	if(EASY_CVAR_GET(hiddenMemPrintout) == 1){
		easyForcePrintLine("RAW INPUT LINE: %s", aryChrLineBuffer);
	}

	if(isStringEmpty(aryChrLineBuffer) || lengthOfString(aryChrLineBuffer) < 2){
		//no / not enough content.  Garbage.
		return;
	}
			
	//find the space.
	int spacePos = UTIL_findCharFirstPos(aryChrLineBuffer, ' ');

	if(spacePos == -1){
		if(EASY_CVAR_GET(hiddenMemPrintout) == 1){
			easyForcePrintLine("ISSUE; NO SPACE");
		}
		//try another line?
		return;
	}

	//atof();

	char identifier[128];
	char valueRaw[128];

	UTIL_substring(identifier, aryChrLineBuffer, 0, spacePos);
	lowercase(identifier);
	UTIL_substring(valueRaw, aryChrLineBuffer, spacePos + 1, -1);

	float value = 0;

	try{
		value = tryStringToFloat(valueRaw);
	}catch(int){
		if(EASY_CVAR_GET(hiddenMemPrintout) == 1){
			easyForcePrintLine("ISSUE; CANT FIND NUMBER");
		}
		easyForcePrintLine("Error loading %s : \"%s\" could not be converted to a decimal.", identifier, valueRaw);
		//Must be able to extract a number to really do anything
		return;
	}


	//easyForcePrintLine("??????? %d : %s", strcmp(identifier, "glockuselastbulletanim" ), identifier);
			
	//easyForcePrintLine("are you serial    %s", line.c_str() );
			
	if(EASY_CVAR_GET(hiddenMemPrintout) == 1){
		easyForcePrintLine("IDENTITY VALUE PAIR: %s %.2f", identifier, value);
	}
	
	EASY_CVAR_HIDDEN_LOAD_MASS;

	//if we reach here, it means the loaded name wasn't matched to a CVar we know of.
	easyForcePrintLine("WARNING: absMem entry \"%s\" not linked to a known CVar. Value discarded.", identifier);

}




void loadHiddenCVars(void){
	
#ifndef _DEBUG
	if(globalPSEUDO_hiddenMemPath[0] == '\0'){
		return;
	}

	char aryChrLineBuffer[256];
	int intLineBufferPos = 0;
	
	//http://www.cplusplus.com/reference/cstdio/fscanf/
	FILE* myFile = fopen( globalPSEUDO_hiddenMemPath, "r");
	if (myFile)
	{
		char c;
		while(true){
			c = fgetc(myFile);
			//next character.

			if(c == '\r'){
				//skip, unnecessary.
			}else if(c == '\n' || c == EOF){
				//End of file or new line? In any case count as a line break just in case the very last line didn't end in a newline character.
				
				aryChrLineBuffer[intLineBufferPos] = '\0'; //this makes some string-processing methods happier.
				processLoadHiddenCVarLine(&aryChrLineBuffer[0]);

				intLineBufferPos = 0;

				//If EOF, stop reading altogether.
				if(c == EOF){
					break;
				}
			}else{
				//any other character? Add to the buffer for this line.
				if(intLineBufferPos < 255){
					aryChrLineBuffer[intLineBufferPos] = c;
					intLineBufferPos++;
				}
			}
		}//END OF while reading file.

		//fprintf(fp, "%s", string);
		fclose(myFile);
	}

#endif
	










	/*
#ifndef _DEBUG

	if(globalPSEUDO_hiddenMemPath[0] == '\0'){
		return;
	}

	string lineRaw;
	ifstream myfile (globalPSEUDO_hiddenMemPath);
	if (myfile.is_open())
	{
		//easyForcePrintLine("IMA LOAD THIS");
		while ( getline (myfile,lineRaw) )
		{
			const char* thisLine = lineRaw.c_str();
			processLoadHiddenCVarLine(thisLine);
		}
		myfile.close();
	}
	else 
	{
		
	}
#endif
	*/



#ifndef CLIENT_DLL
	//easyForcePrintLine("hornetcvartest: what1 ", CVAR_GET_FLOAT("cl_hornetspiral"), globalPSEUDO_cl_hornetspiral);

	//hack. Force the PSEUDO value to match the current loaded (possibly) value of cl_hornetspiral so that the game doesn't automatically change more specific settings
	//about hornet movement the user may have specified. Note that cl_hornetspiral isn't hidden (normal CVar even in Release), so you have to get it from the CVar
	//system directly and not the stored variables (global_...)
	globalPSEUDO_cl_hornetspiral = EASY_CVAR_GET(cl_hornetspiral);
	
	//No need to set this to itself now...?
	//"settin"
	//global_cl_hornetspiral = CVAR_GET_FLOAT("cl_hornetspiral");
	EASY_CVAR_SET(cl_hornetspiral, EASY_CVAR_GET(cl_hornetspiral));
	

	//TODO: do something like this with the gaussmode CVar?
#endif



}



void saveHiddenCVars(void){
	
#ifndef _DEBUG
	if(globalPSEUDO_hiddenMemPath[0] == '\0'){
		return;
	}
	FILE* myFile = fopen( globalPSEUDO_hiddenMemPath, "w");  //"a+t");
	if (myFile)
	{
		
		EASY_CVAR_HIDDEN_SAVE_MASS;

		fclose(myFile);
		if(EASY_CVAR_GET(hiddenMemPrintout) == 1){
			easyForcePrintLine("File generation okay?"); 
		}
	}else{
		if(EASY_CVAR_GET(hiddenMemPrintout) == 1){
			easyForcePrintLine("File generation failed?"); 
		}
	}
#endif

	
	/*
#ifndef _DEBUG
	if(globalPSEUDO_hiddenMemPath[0] == '\0'){
		return;
	}
	//easyForcePrintLine("Unable to open file, generating...");
	ofstream myfile (globalPSEUDO_hiddenMemPath);
	if (myfile.is_open())
	{
		EASY_CVAR_HIDDENSAVE_MASS;
		myfile.close();
		if(global_hiddenMemPrintout == 1){
			easyForcePrintLine("File generation okay...?"); 
		}
	}else{
		if(global_hiddenMemPrintout == 1){
			easyForcePrintLine("File generation failed?"); 
		}
	}
#endif
	*/
}//END OF saveHiddenCVars













//Provided character buffer (character array) must be binaryDigits+1 large to account for all digits and the terminating '/0', or 0-valued null character.
//Assuming an unsigned 32 bit int is provided.
void convertIntToBinary(char* buffer, unsigned int arg, unsigned int binaryDigits){

	//char aryChrReturn[9];
	//aryChrReturn[8] = '\0';

	buffer[0] = '\0';  //terminating character at the start just in case of error.

	unsigned int maxNumberPossible;

	if(binaryDigits > 32){
		easyForcePrintLine("ERROR: Too many biniary digits! Max for int is 32");
		return;
	}else if(binaryDigits == 32){
		//do-able, but need a different method.  (1 << 32) would overflow, leaving only "1". Just invert all bits of solid 0 for straight 1's.
		maxNumberPossible = ~0;
	}else if(binaryDigits < 0){
		easyForcePrintLine("ERROR: negative number of binaryDigits not allowed!");
		return;
	}else if(binaryDigits == 0){
		easyForcePrintLine("ERROR: binaryDigits cannot be 0. Nothing to do.");
		return;
	}else{
		//between 1 and 31 inclusive? this is fine. 2 to the binaryDigits power, minus 1 to remove the left-most single 1 bit and make all the ones right-ward into 1's.
		//That produces the largest number possibly stored by our number of bits all being 1's.
		//This is because actually all bits from 2^0 to 2^(binaryDigits - 1) are included,  2^(binaryDigits) itself is excluded.
		maxNumberPossible = (1 << binaryDigits) - 1;
	}

	/*
	unsigned int maxNumberPossible2 = (1 << binaryDigits);
	unsigned int maxNumberPossible3 = ~0;
	unsigned int maxNumberPossible = (1 << binaryDigits) - 1;
	*/

	if (arg > maxNumberPossible) {
		easyForcePrintLine("convertToBinary: ERROR. Overflow. Input number of %u is above the maximum allowed for %u digits!", arg, binaryDigits);
		return;
	}

	buffer[binaryDigits] = '\0';  //terminating character.

	for (unsigned int i = 0; i < binaryDigits; i++) {
		
		if (  (arg & (1 << i)) != 0 ) {
			//this digit is 1.
			buffer[binaryDigits-1 - i] = '1';
		}
		else {
			buffer[binaryDigits-1 - i] = '0';
		}
	}
	
	//return string(aryChrReturn);
}//END OF convertIntToBinary





BOOL IsMultiplayer(void){
#ifdef CLIENT_DLL
	return gEngfuncs.GetMaxClients() == 1 ? 0 : 1;
#else
	return g_pGameRules->IsMultiplayer();
#endif
}//END OF IsMultiplayer





void UTIL_Sparks(const Vector& position){
	//This starting method from the SDK in particular should no longer be called, having been replaced
	//by UTIL_Sparks2 (all calls to UTIL_Sparks in this project refer to UTIL_Sparks2 instead).

	//If this is somehow called again, please say so.
	//easyPrintLine("!!!!!!!!! SPARK CREATION UNSOURCED 2!!!!!!!!!");
	UTIL_Sparks2(position, DEFAULT_SPARK_BALLS, EASY_CVAR_GET(sparksEnvMulti));


}//END OF Util_Sparks(...)





/*
//MODDD - can now accept a "ballsToSpawn" var, generally for spawning less to prevent crashes if a CVar is set.
void UTIL_Sparks2( const Vector &position){
	//but, imply "DEFAULT_SPARK_BALLS" if unspecified.  (See const.h)
	UTIL_Sparks2( position, DEFAULT_SPARK_BALLS, 1 );

}

void UTIL_Sparks2( const Vector &position, int arg_ballsToSpawn ){
	UTIL_Sparks2(position, arg_ballsToSpawn, 1);

}
*/



void UTIL_Sparks2(const Vector& position, int arg_ballsToSpawn, float arg_extraSparkMulti){


	if (EASY_CVAR_GET(useAlphaSparks) == 0) {
		//use retail then.

#ifdef CLIENT_DLL
		//Clientside? Call for the effect directly.
		//float aryfl_pos[3];
		//aryfl_pos[0] = position.x;
		//aryfl_pos[1] = position.y;
		//aryfl_pos[2] = position.z;
		gEngfuncs.pEfxAPI->R_SparkShower((float*)&position);
#else
		//Serverside?  Tell it to call for that.
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
		WRITE_BYTE(TE_SPARKS);
		WRITE_COORD(position.x);
		WRITE_COORD(position.y);
		WRITE_COORD(position.z);
		MESSAGE_END();
#endif
		return;
	}//END OF if

	//PLAYBACK_EVENT_FULL (FEV_GLOBAL, pGib->edict(), g_sTrail, 0.0, 
	//	(float *)&pGib->pev->origin, (float *)&pGib->pev->angles, 0.7, 0.0, pGib->entindex(), ROCKET_TRAIL, 0, 0);


	int ballsToSpawn;

	float multToUse = arg_extraSparkMulti * EASY_CVAR_GET(sparksAllMulti);

	/*
	float multToUse = arg_extraSparkMulti;
	if(arg_extraSparkMulti == -1){
		//fall back to global.
		easyPrintLine("!!!!!!!!! SPARK CREATION UNSOURCED 1!!!!!!!!!");
		multToUse = EASY_CVAR_GET(sparkBallAmmountMulti);
	}
	*/

	if (multToUse != 1) {
		//multiplying by 1 is useless, so don't if it is.
		ballsToSpawn = (int)((float)arg_ballsToSpawn * multToUse);
	}
	else {
		ballsToSpawn = arg_ballsToSpawn;
	}






#ifdef CLIENT_DLL
	//Clientside?  Can call EV_ShowBalls directly.
	event_args_t tempArgSendoff;
	memset(&tempArgSendoff, 0, sizeof tempArgSendoff);
	// is this the same as setting to  {}, like 
	//     poopyDick = {}
	// ?

	tempArgSendoff.origin[0] = position.x;
	tempArgSendoff.origin[1] = position.y;
	tempArgSendoff.origin[2] = position.z;
	tempArgSendoff.iparam1 = ballsToSpawn;

	EV_ShowBalls(&tempArgSendoff);
#else
	//Serverside?  Tell the client to call "EV_ShowBalls" with the parameters.
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	PLAYBACK_EVENT_FULL(FEV_GLOBAL, NULL, g_sCustomBalls, 0.0, (float*)&position, (float*)&Vector(0, 0, 0), 0.0, 0.0, ballsToSpawn, 0, FALSE, FALSE);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//( int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
#endif





	/*
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, position );
		WRITE_BYTE( TE_SPRAY );
		WRITE_COORD( position.x );
		WRITE_COORD( position.y );
		WRITE_COORD( position.z );

		WRITE_COORD( 0 );
		WRITE_COORD( 0 );
		WRITE_COORD( 0 );

		WRITE_BYTE(g_sGaussBallSprite);

		WRITE_BYTE(8);
		WRITE_BYTE(10);
		WRITE_BYTE(1);
		WRITE_BYTE(1);


	MESSAGE_END();
	*/

	// coord, coord, coord (position)
	// coord, coord, coord (direction)
	// short (modelindex)
	// byte (count)
	// byte (speed)
	// byte (noise)
	// byte (rendermode)






	//gEngfuncs.pEfxAPI->R_Sprite_Trail( TE_SPRITETRAIL, tr.endpos, fwd, m_iBalls, 8, 0.6, gEngfuncs.pfnRandomFloat( 10, 20 ) / 100.0, 100,
	//							255, 200 );


	//int type, float * start, float * end, int modelIndex, int count, float life, float size, float amplitude, int renderamt, float speed 

	//#define TE_SPRITETRAIL		15		// line of moving glow sprites with gravity, fadeout, and collisions
// coord, coord, coord (start) 
// coord, coord, coord (end) 
// short (sprite index)
// byte (count)
// byte (life in 0.1's) 
// byte (scale in 0.1's) 
// byte (velocity along vector in 10's)
// byte (randomness of velocity in 10's)

	//client: gEngfuncs.pfnRandomFloat( 10, 20 );
	//server: g_engfuncs.pfnRandomFloat( 10, 20 );
	//*not all commands are this squeakly clean from one to the other.  (make that uh, virtually none that are useful).


	/*
	byte rando = (byte)((g_engfuncs.pfnRandomFloat( 10, 20 ) / 100) * 10) ;

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, position );
		WRITE_BYTE( TE_SPRITETRAIL );
		WRITE_COORD( position.x );
		WRITE_COORD( position.y );
		WRITE_COORD( position.z );

		WRITE_COORD( position.x );
		WRITE_COORD( position.y );
		WRITE_COORD( position.z );

		WRITE_SHORT(g_sGaussBallSprite);
		WRITE_BYTE(8);
		WRITE_BYTE( rando );

		WRITE_BYTE(10);
		WRITE_BYTE(10);



	MESSAGE_END();

	*/

}//END OF UTIL_Sparks2




/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


