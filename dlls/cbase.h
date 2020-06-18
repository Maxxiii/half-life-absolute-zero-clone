/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

// MODDD NOTICE - this file, too, is included both client/serverside.



//MODDD - WHY NOT???
#ifndef CBASE_H
#define CBASE_H



/*

Class Hierachy

CBaseEntity
	CBaseDelay
		CBaseToggle
			CBaseItem
			CBaseMonster
				CBaseCycler
				CBasePlayer
				CBaseGroup
*/

#define		MAX_PATH_SIZE	10 // max number of nodes available for a path.

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
#define		FCAP_CUSTOMSAVE				0x00000001
#define		FCAP_ACROSS_TRANSITION		0x00000002		// should transfer between transitions
#define		FCAP_MUST_SPAWN				0x00000004		// Spawn after restore
#define		FCAP_DONT_SAVE				0x80000000		// Don't save this
#define		FCAP_IMPULSE_USE			0x00000008		// can be used by the player
#define		FCAP_CONTINUOUS_USE			0x00000010		// can be used by the player
#define		FCAP_ONOFF_USE				0x00000020		// can be used by the player
#define		FCAP_DIRECTIONAL_USE		0x00000040		// Player sends +/- 1 when using (currently only tracktrains)
#define		FCAP_MASTER					0x00000080		// Can be used to "master" other entities (like multisource)

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
#define		FCAP_FORCE_TRANSITION		0x00000080		// ALWAYS goes across transitions



//MODDD - bizarre...? Shouldn't basemonster.h or .cpp include schedule.h and monsterevent.h instead?
#include "saverestore.h"
#include "schedule.h"

#include "monsterevent.h"



#include "util.h"
#include "vector_shared.h"




//MODDD - moved from nodes.h, accessible in more places.
//MODDD - new.  Don't treat like the usual NODE HULL types below. 
//              NODE_POINT_DEFAULT says we're leaving up what NODE HULL this is to retail behavior, the checks in 
//              NODE_POINT_HULL is to say, pretend like our entity has no size. Just allow if there is a node connection at all.
//              For debugging route problems to see if the issue is caused by a hull being too big even if a path was found.
#define		NODE_DEFAULT_HULL		-2
#define		NODE_POINT_HULL			-1

#define		NODE_SMALL_HULL			0
#define		NODE_HUMAN_HULL			1
#define		NODE_LARGE_HULL			2
#define		NODE_FLY_HULL			3







//MODDD - new set of constants.  Sometimes we need to tell different types of projectiles apart from one another.
//safe default for all entities.  Override "GetProjectileType" to change per entity class.
#define PROJECTILE_NONE 0
//Not really intended to be thrown or go flying to a target.  Not quite the remote charges (satchel), but includes tripmines.
#define PROJECTILE_DEPLOYABLE 1
//Just the crossbow's bolt.
#define PROJECTILE_BOLT 2
//mp5 grenades, hand grenades. Moderate weighted non-propelled grenandes.
#define PROJECTILE_GRENADE 3
//projectile with a strong sense of direction, propelled, following or not.
#define PROJECTILE_ROCKET 4
//balls of electricity from controllers, archers, kingpins.
#define PROJECTILE_ENERGYBALL 5
//organic projectiles that are usually big green and toxic like bullsquid spit.
#define PROJECTILE_ORGANIC_DUMB 6
//living things that act like projectiles but can't do damage (chumtoads)
#define PROJECTILE_ORGANIC_HARMLESS 7
//living things that act like damaging projectiles on impact, with at least minimal AI to separate it from ORGANIC_DUMB.
#define PROJECTILE_ORGANIC_HOSTILE 8









// C functions for external declarations that call the appropriate C++ methods

#ifdef _WIN32
#define EXPORT	_declspec( dllexport )
#else
#define EXPORT	/* */
#endif

extern "C" EXPORT int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion );
extern "C" EXPORT int GetEntityAPI2( DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion );

extern int DispatchSpawn( edict_t *pent );
extern void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd );
extern void DispatchTouch( edict_t *pentTouched, edict_t *pentOther );
extern void DispatchUse( edict_t *pentUsed, edict_t *pentOther );
extern void DispatchThink( edict_t *pent );
extern void DispatchBlocked( edict_t *pentBlocked, edict_t *pentOther );
extern void DispatchSave( edict_t *pent, SAVERESTOREDATA *pSaveData );
extern int  DispatchRestore( edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity );
extern void	DispatchObjectCollsionBox( edict_t *pent );
extern void SaveWriteFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount );
extern void SaveReadFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount );
extern void SaveGlobalState( SAVERESTOREDATA *pSaveData );
extern void RestoreGlobalState( SAVERESTOREDATA *pSaveData );
extern void ResetGlobalState( void );

typedef enum { USE_OFF = 0, USE_ON = 1, USE_SET = 2, USE_TOGGLE = 3 } USE_TYPE;

extern void FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

//MODDD - temporary.
extern void FireTargetsTest( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );


typedef void (CBaseEntity::*BASEPTR)(void);
typedef void (CBaseEntity::*ENTITYFUNCPTR)(CBaseEntity *pOther );
typedef void (CBaseEntity::*USEPTR)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );




// For CLASSIFY
#define CLASS_NONE				0
#define CLASS_MACHINE			1
#define CLASS_PLAYER			2
#define CLASS_HUMAN_PASSIVE		3
#define CLASS_HUMAN_MILITARY	4
#define CLASS_ALIEN_MILITARY	5
#define CLASS_ALIEN_PASSIVE		6
#define CLASS_ALIEN_MONSTER		7
#define CLASS_ALIEN_PREY		8
#define CLASS_ALIEN_PREDATOR	9
#define CLASS_INSECT			10
#define CLASS_PLAYER_ALLY		11
#define CLASS_PLAYER_BIOWEAPON	12 // hornets and snarks.launched by players
#define CLASS_ALIEN_BIOWEAPON	13 // hornets and snarks.launched by the alien menace
#define CLASS_BARNACLE			99 // special because no one pays attention to it, and it eats a wide cross-section of creatures.




// (MOVED FROM basemonster.h, originally from monsters.h for the entity methods that involve relationships)
// monster to monster relationship types

//MODDD - NEW TYPE: 
#define R_DEFAULT	-5  // A signal to allow the default relationship as intended in the table.

//#define R_FO	-4  // food. I attract the other monster, and may get eaten at melee range.
//#define R_BA	-3	// bait. I attract the other monster, but don't get attacked by it.


#define R_CA    -3 // (CAUTIOUS). Not an enemy (yet?), but looks kind of scary. Don't attack or piss off, but if cowardly (scientist), point and gawk, maybe run away. If not (Barney), point your gun at it and stare.
                   //This status should occur on enemies that are marked provokable and not yet provoked, unless Barnies are supposed to be a liablity and stupidly prvoke it regardless of your wishes.
                   //This will NOT be default AI - factions HUMAN_MILITARY and anything ALIEN don't care about avoiding conflict.

#define R_AL	-2 // (ALLY) pals. Good alternative to R_NO when applicable.
#define R_FR	-1// (FEAR)will run
#define R_NO	0// (NO RELATIONSHIP) disregard
#define R_DL	1// (DISLIKE) will attack
#define R_HT	2// (HATE)will attack this character instead of any visible DISLIKEd characters
#define R_NM	3// (NEMESIS)  A monster Will ALWAYS attack its nemsis, no matter what

//MODDD - new level. Bait. To be used in a class (Chumtoad) only.
#define R_BA    4//


#define SF_NORESPAWN	( 1 << 30 )// !!!set this bit on guns and stuff that should never respawn.






class CBaseEntity;
class CBaseMonster;
class CBasePlayerItem;
class CSquadMonster;


//MODDD - server only.
#ifndef CLIENT_DLL
class CSound;
#endif



//
// EHANDLE. Safe way to point to CBaseEntities who may die between frames
//
class EHANDLE
{
private:
	edict_t *m_pent;
	int		m_serialnumber;
public:
	edict_t *Get( void );
	edict_t *Set( edict_t *pent );

	operator int ();

	operator CBaseEntity *();

	CBaseEntity * operator = (CBaseEntity *pEntity);
	CBaseEntity * operator ->();

	CBaseEntity* GetEntity();
};


//
// Base Entity.  All entity types derive from this
//
class CBaseEntity 
{
public:

	virtual void ReportGeneric(void);

	//MODDD - NEW!!! compatability.
	Vector GetAbsVelocity(void);

	Vector GetAbsOrigin(void);
	void SetAbsOrigin(const Vector& arg_newOrigin);

	Vector GetAbsAngles(void);
	void SetAbsAngles(const Vector& arg_newOrigin);

	
	virtual BOOL isBasePlayerWeapon(void){return FALSE;}

	virtual BOOL blocksImpact(void);
	virtual float massInfluence(void);
	virtual int GetProjectileType(void);

	virtual Vector GetVelocityLogical(void);
	virtual void SetVelocityLogical(const Vector& arg_newVelocity);

	virtual void OnDeflected(CBaseEntity* arg_entDeflector);

	BOOL alreadySaved;
	int wasAttached;

	// Constructor.  Set engine to use C/C++ callback functions
	// pointers to engine data
	entvars_t *pev;		// Don't need to save/restore this pointer, the engine resets it

	// path corners
	CBaseEntity			*m_pGoalEnt;// path corner we are heading towards
	CBaseEntity			*m_pLink;// used for temporary link-list operations. 


	//MODDDMIRROR
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	CBaseEntity			*m_pMoveWith; // LRC- the entity I move with.
	int					m_MoveWith;	//LRC- Name of that entity
	CBaseEntity			*m_pChildMoveWith;	//LRC- one of the entities that's moving with me.
	CBaseEntity			*m_pSiblingMoveWith; //LRC- another entity that's Moving With the same ent as me. (linked list.)
	Vector				m_vecMoveWithOffset; // LRC- Position I should be in relative to m_pMoveWith->pev->origin.
	Vector				m_vecRotWithOffset; // LRC- Angles I should be facing relative to m_pMoveWith->pev->angles.
	CBaseEntity			*m_pAssistLink; // LRC- link to the next entity which needs to be Assisted before physics are applied.
	Vector				m_vecPostAssistVel; // LRC
	Vector				m_vecPostAssistAVel; // LRC
	float				m_fNextThink; // LRC - for SetNextThink and SetPhysThink. Marks the time when a think will be performed - not necessarily the same as pev->nextthink!
	float				m_fPevNextThink; // LRC - always set equal to pev->nextthink, so that we can tell when the latter gets changed by the @#$^�! engine.
	int					m_iLFlags; // LRC- a new set of flags. (pev->spawnflags and pev->flags are full...)
	virtual void		DesiredAction( void ) {}; // LRC - for postponing stuff until PostThink time, not as a think.
	int					m_iStyle; // LRC - almost anything can have a lightstyle these days...

	Vector				m_vecSpawnOffset; // LRC- To fix things which (for example) MoveWith a door which Starts Open.
	BOOL				m_activated;	// LRC- moved here from func_train. Signifies that an entity has already been
										// activated. (and hence doesn't need reactivating.)

	//AJH Entities can now have custom names and kill techniques for deathnotices
	//E.g instead of "Player1 killed Player2 with train" you can have "Player1 decapitated Player2 with a large table saw!)
	string_t			killname;	//AJH custom 'deathnotice' name
	string_t			killmethod;	//AJH custom kill techniques

	//MODDD - new instance var.
	BOOL usingCustomSequence;
	
	//Stores whether the AI can recognize that this enemy is dead yet, by default around halfway into a death animation.
	//Kinda doesn't make sense they keep firing at something that's clearly sprawled out on the ground just because the death animation stayed there for a little.
	//Note that this will only be used by Monsters in TASK_DIE to denote when the AI should, for all intents and purposes, consider this as DEAD_DEAD and pick a new enemy.
	//This should not replace be trusted for checking already DEAD_DEAD things which can be set that way without going through TASK_DIE.
	BOOL recognizablyDead;



	//LRC - decent mechanisms for setting think times!
	// this should have been done a long time ago, but MoveWith finally forced me.
	virtual void		SetNextThink( float delay ) { SetNextThink(delay, FALSE); }
	virtual void		SetNextThink( float delay, BOOL correctSpeed );
	virtual void		AbsoluteNextThink( float time ) { AbsoluteNextThink(time, FALSE); }
	virtual void		AbsoluteNextThink( float time, BOOL correctSpeed );
	void				SetEternalThink( );
	


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	virtual BOOL isMovetypeFlying(void) const;

	//MODDD - new. 
	virtual BOOL isSizeGiant(void);
	virtual BOOL isOrganic(void);
	virtual int getHullIndexForNodes(void) const;

	//MODDD - moved here from delay.
	int getNumberOfBodyParts(void);
	int getNumberOfSkins(void);

	//New var to send printouts related to animation.  Call me on something that appears frozen.
	BOOL crazyPrintout;


	//MODDD
	virtual void onDelete(void);

	//MODDD
	const char* getClassname(void);
	const char* getClassnameShort(void);
	
	// initialization functions
	//MODDD - changed for a test.
	//virtual void	Spawn( void ) { return; }
	virtual void	Spawn( void );
	virtual void	Precache( void ) { return; }

	//MODDD - new
	CBaseEntity(void);
	//static void method_precacheAll();


	//Even though the relationship may disallow it, I can be caught by the barnacle anyways (if true).
	virtual BOOL getIsBarnacleVictimException(void);
	virtual BOOL isBreakableOrchild(void);
	virtual BOOL isDestructibleInanimate(void);
	virtual BOOL isTalkMonster(void);
	virtual BOOL isProvokable(void);
	virtual BOOL isProvoked(void);

	//also MODDD
	virtual void DefaultSpawnNotice(void);
	virtual void ForceSpawnFlag(int arg_spawnFlag);
	
	//MODDD
	BOOL spawnedDynamically;
	BOOL firstSpawnCall;
	BOOL flagForced;
	
	//is this the first time calling Killed? important sometimes.
	BOOL firstTimeKilled;

	Activity timeOfDeath_activity;
	int timeOfDeath_sequence;

	//MODDD
	virtual BOOL usesSoundSentenceSave(void);

	
	//MODDD
	virtual void playAmmoPickupSound();
	virtual void playAmmoPickupSound(entvars_t* sentPev);
	virtual void playGunPickupSound();
	virtual void playGunPickupSound(entvars_t* sentPev);
	virtual void precacheAmmoPickupSound();
	virtual void precacheGunPickupSound();

	virtual void DrawAlphaBlood(float flDamage, const Vector& vecDrawLoc, int amount );
	
	virtual void DrawAlphaBloodSlash(float flDamage, const Vector& vecDrawLoc, const Vector& vecTraceLine  );
	virtual void DrawAlphaBloodSlash(float flDamage, const Vector& vecDrawLoc, const Vector& vecTraceLine, const BOOL& extraBlood );

	
	virtual void DrawAlphaBlood(float flDamage, TraceResult *ptr );
	virtual void DrawAlphaBlood(float flDamage, const Vector& vecDrawLoc );



	virtual void	KeyValue( KeyValueData* pkvd) { pkvd->fHandled = FALSE; }
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	void PostRestore(void);




	virtual void playMetallicHitSound(int arg_channel, float arg_volume);

	//Made because this check is done a lot for traces. Need to know if a wall, floor, any part of the map, etc. was hit
	virtual BOOL IsWorld(void);
	virtual BOOL IsWorldAffiliated(void);
	BOOL IsWorldOrAffiliated(void);  //NOT virtual. Never needs to be special.
	virtual BOOL IsBreakable(void);

	virtual int		ObjectCaps( void ) { return FCAP_ACROSS_TRANSITION; }
	virtual void	Activate( void ) {}
	
	// Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	virtual void	SetObjectCollisionBox( void );


	//Method to call without a string. Can be used to check the German model system for monsters (substitute a different one if censorship is on)
	virtual void setModel(void);
	//Method called to set this entity's model. Can be overridden by classes that need to set something that comes from the model, such as number of skins for eye-blinking.
	virtual void setModel(const char* m);



	// A simpler version of forcedRelationshipWith that should be considered at the same time.
	// Just a quick check to see whether the current should be unconditionably hated no matter what.
	// It actually isn't taken to heart too much, as it would conflict with  forcedRelationshipWith.
	// This method is not involved in IRelationship.
	virtual BOOL isForceHated(CBaseEntity *pBy);


	// This method allows child classes to change how other monsters look at this particular instance (or the class in general towards other's by class, period).
	// Such as, make a Bullsquid ignore Headcrabs (a Headcrab could override forcedRelationshipWith to say R_NO if pWith's class is monster_bullsquid).
	// Or a  more realistic example: The chumtoad can force all other monsters to hate it, besides fellow toads (class check) without needing to check their class and
	//   then send a class it happens to hate to be hated (really dodgy and not possibly anyways in IRelationship, which is caller-sided)
	virtual int forcedRelationshipWith(CBaseEntity *pWith);

	
#ifndef CLIENT_DLL
	// This method lets script in GetSchedule related to picking up on bait sounds or not get handled in one Monster method instead of all over the place.
	//also, server only. AI only.
	virtual SCHEDULE_TYPE getHeardBaitSoundSchedule(CSound* pSound);
	// This underscored version skips the monsterstate (IDLE / ALERT) check. It should only be called by "getHeardBaitSoundSchedule" without a provided Sound, which had
	// to find a sound itself and already did the state check.
	virtual SCHEDULE_TYPE _getHeardBaitSoundSchedule(CSound* pSound);
	virtual SCHEDULE_TYPE getHeardBaitSoundSchedule();
#endif;



// Classify - returns the type of group (i.e, "houndeye", or "human military" so that monsters with different classnames
// still realize that they are teammates. (overridden for monsters that form groups)
	virtual int Classify ( void ) { return CLASS_NONE; };
	virtual void DeathNotice ( entvars_t *pevChild ) {}// monster maker children use this to tell the monster maker that they have died.


	static	TYPEDESCRIPTION m_SaveData[];


	//MODDD - new args possible.
	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL
	

	virtual int		TakeHealth( float flHealth, int bitsDamageType );

	GENERATE_KILLED_PROTOTYPE_VIRTUAL
	//virtual void	Killed( entvars_t *pevAttacker, int iGib );

	virtual int		BloodColor( void ) { return DONT_BLEED; }
	//MODDD - TraceBleed supports the extra damage bitmask.
	virtual void	TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType );
	virtual void	TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int bitsDamageTypeMod );
	virtual BOOL    IsTriggered( CBaseEntity *pActivator ) {return TRUE;}
	virtual CBaseMonster *MyMonsterPointer( void ) { return NULL;}
	virtual CSquadMonster *MySquadMonsterPointer( void ) { return NULL;}
	virtual	int		GetToggleState( void ) { return TS_AT_TOP; }
	virtual void	AddPoints( int score, BOOL bAllowNegativeScore ) {}
	virtual void	AddPointsToTeam( int score, BOOL bAllowNegativeScore ) {}
	virtual BOOL	AddPlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual BOOL	RemovePlayerItem( CBasePlayerItem *pItem ) { return 0; }
	virtual int 	GiveAmmo( int iAmount, char *szName, int iMax ) { return -1; };
	virtual float	GetDelay( void ) { return 0; }
	virtual int		IsMoving( void ) { return pev->velocity != g_vecZero; }
	virtual void	OverrideReset( void ) {}

	//supports the extra damage bitmask for some reason.
	virtual int		DamageDecal( int bitsDamageType );
	virtual int		DamageDecal( int bitsDamageType, int bitsDamageTypeMod );

	// This is ONLY used by the node graph to test movement through a door
	virtual void	SetToggleState( int state ) {}
	virtual void    StartSneaking( void ) {}
	virtual void    StopSneaking( void ) {}
	virtual BOOL	OnControls( entvars_t *pev ) { return FALSE; }
	virtual BOOL    IsSneaking( void ) { return FALSE; }


	//MODDD - why does IsAlive check for above 0 health?  The deadflag is always properly adjusted if it goes under 0.
	//virtual BOOL	IsAlive( void ) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL IsAlive(void){return (pev->deadflag == DEAD_NO); }
	//MODDD NEW - alternate version for special cases. Also factors in "recognizablyDead": change enemies after this one's gone far enough in the death anim.
	//            Does this mean any case where health is below 0 but still alive are ignored by AI? Not that this should ever happen.
	//            BLAH this is CBaseEntity, not CBaseMonster. Forget that, this precision really isn't helpful here.
	//virtual BOOL IsAlive_FromAI( CBaseMonster* whoWantsToKnow ) { return (pev->deadflag == DEAD_NO || (pev->deadflag == DEAD_DYING && !recognizablyDead ) ) && pev->health > 0; }
	virtual BOOL IsAlive_FromAI( CBaseMonster* whoWantsToKnow ) { return (pev->deadflag == DEAD_NO || (pev->deadflag == DEAD_DYING && !recognizablyDead ) ); }


	virtual BOOL	IsBSPModel( void ) { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	virtual BOOL	ReflectGauss( void ) { return ( IsBSPModel() && !pev->takedamage ); }
	virtual BOOL	HasTarget( string_t targetname ) { return FStrEq(STRING(targetname), STRING(pev->targetname) ); }
	virtual BOOL    IsInWorld( void );
	virtual	BOOL	IsPlayer( void ) { return FALSE; }
	virtual BOOL	IsNetClient( void ) { return FALSE; }
	virtual const char *TeamID( void ) { return ""; }


//	virtual void	SetActivator( CBaseEntity *pActivator ) {}
	virtual CBaseEntity *GetNextTarget( void );
	
	// fundamental callbacks
	void (CBaseEntity ::*m_pfnThink)(void);
	void (CBaseEntity ::*m_pfnTouch)( CBaseEntity *pOther );
	void (CBaseEntity ::*m_pfnUse)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void (CBaseEntity ::*m_pfnBlocked)( CBaseEntity *pOther );

	virtual void Think( void ) { if (m_pfnThink) (this->*m_pfnThink)(); };
	



	virtual void Touch( CBaseEntity *pOther ) { if (m_pfnTouch) (this->*m_pfnTouch)( pOther ); };
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) 
	{ 
		//easyPrintLine("WELL IS IT????? %d %s", m_pfnUse == NULL, pCaller!=NULL?pCaller->getClassname():"blankcaller?");
		if (m_pfnUse) 
			(this->*m_pfnUse)( pActivator, pCaller, useType, value );
	}
	virtual void Blocked( CBaseEntity *pOther ) { if (m_pfnBlocked) (this->*m_pfnBlocked)( pOther ); };

	// allow engine to allocate instance data
    void *operator new( size_t stAllocateBlock, entvars_t *pev )
	{
		return (void *)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
	};

	// don't use this.
#if _MSC_VER >= 1200 // only build this code if MSVC++ 6.0 or higher
	void operator delete(void *pMem, entvars_t *pev)
	{
		pev->flags |= FL_KILLME;
	};
#endif

	void UpdateOnRemove( void );

	// common member functions
	void EXPORT SUB_Remove( void );
	void EXPORT SUB_DoNothing( void );
	void EXPORT SUB_StartFadeOut ( void );
	void EXPORT SUB_FadeOut ( void );
	void EXPORT SUB_CallUseToggle( void ) { this->Use( this, this, USE_TOGGLE, 0 ); }
	int			ShouldToggle( USE_TYPE useType, BOOL currentState );


	//MODDD - new.
	BOOL CheckTracer(const Vector& vecSrc, const Vector& vecEnd, const Vector& vecDirForward, const Vector& vecDirRight, int iBulletType, int iTracerFreq, int* p_tracerCount);

	void		FireBullets( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL  );
	Vector		FireBulletsPlayer( ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t *pevAttacker = NULL, int shared_rand = 0 );

	virtual CBaseEntity *Respawn( void ) { return NULL; }

	void SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value );
	// Do the bounding boxes of these two intersect?
	int		Intersects( CBaseEntity *pOther );
	void	MakeDormant( void );
	int		IsDormant( void );
	BOOL    IsLockedByMaster( void ) { return FALSE; }

	static CBaseEntity *Instance( edict_t *pent )
	{ 
		if ( !pent )
			pent = ENT(0);
		CBaseEntity *pEnt = (CBaseEntity *)GET_PRIVATE(pent); 
		return pEnt; 
	}

	static CBaseEntity *Instance( entvars_t *pev ) { return Instance( ENT( pev ) ); }
	static CBaseEntity *Instance( int eoffset) { return Instance( ENT( eoffset) ); }


	//MODDD - the wounded NPC to seek out.
	CBaseMonster* healTargetNPC;

	//MODDD - no idea why this version didn't exist before.  From SELF.
	//...Oh.  "MyMonsterPointer", nevermind.
	CBaseMonster *GetMonsterPointer(  ) 
	{ 
		CBaseEntity *pEntity = Instance( pev );
		if ( pEntity )
			return pEntity->MyMonsterPointer();
		return NULL;
	}

	CBaseMonster *GetMonsterPointer( entvars_t *pevMonster ) 
	{ 
		CBaseEntity *pEntity = Instance( pevMonster );
		if ( pEntity )
			return pEntity->MyMonsterPointer();
		return NULL;
	}
	CBaseMonster *GetMonsterPointer( edict_t *pentMonster ) 
	{ 
		CBaseEntity *pEntity = Instance( pentMonster );
		if ( pEntity )
			return pEntity->MyMonsterPointer();
		return NULL;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
#ifdef _DEBUG
	void FunctionCheck( void *pFunction, char *name ) 
	{ 
		if (pFunction && !NAME_FOR_FUNCTION((unsigned long)(pFunction)) )
			ALERT( at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (unsigned long)pFunction );
	}

	BASEPTR	ThinkSet( BASEPTR func, char *name ) 
	{ 
		m_pfnThink = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnThink)))), name ); 
		return func;
	}
	ENTITYFUNCPTR TouchSet( ENTITYFUNCPTR func, char *name ) 
	{ 
		m_pfnTouch = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnTouch)))), name ); 
		return func;
	}
	USEPTR	UseSet( USEPTR func, char *name ) 
	{ 
		//easyPrintLine("UseSet NAME: %s", name);
		m_pfnUse = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnUse)))), name ); 
		return func;
	}
	ENTITYFUNCPTR	BlockedSet( ENTITYFUNCPTR func, char *name ) 
	{ 
		m_pfnBlocked = func; 
		FunctionCheck( (void *)*((int *)((char *)this + ( offsetof(CBaseEntity,m_pfnBlocked)))), name ); 
		return func;
	}

#endif


	// virtual functions used by a few classes
	
	// used by monsters that are created by the MonsterMaker
	virtual	void UpdateOwner( void ) { return; };

	//
	static CBaseEntity* CreateManual( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner = NULL );
	static CBaseEntity* Create( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner = NULL );
	static CBaseEntity* Create( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, int setSpawnFlags, edict_t *pentOwner = NULL );

	virtual BOOL FBecomeProne( void ) {return FALSE;};
	edict_t *edict() { return ENT( pev ); };
	EOFFSET eoffset( ) { return OFFSET( pev ); };
	int	  entindex( ) { return ENTINDEX( edict() ); };

	virtual Vector Center( ) { return (pev->absmax + pev->absmin) * 0.5; }; // center point of entity
	virtual Vector EyePosition( ) { return pev->origin + pev->view_ofs; };			// position of eyes

	//MODDD - new. Don't add the position in this one.
	//Anything overriding EyePosition should override EyeOffset too.
	virtual Vector EyeOffset( ) { return pev->view_ofs; };			// position of eyes

	virtual Vector EarPosition( ) { return pev->origin + pev->view_ofs; };			// position of ears
	virtual Vector BodyTarget( const Vector &posSrc ) { return Center( ); };		// position to shoot at
	//MODDD - This method was created to mimick BodyTarget (a clone of it most of the time across the rest
	//        of in-game objects that customize it), but for CBasePlayer, it doesn't try to "randomize" the
	//        result (shift vertically, for some reason...).
	virtual Vector BodyTargetMod( const Vector &posSrc ) { return Center( ); };		// position to shoot at

	virtual int Illumination( ) { return GETENTITYILLUM( ENT( pev ) ); };

	//MODDD NOTE - despite these being virtual, they are never implemented elsewhere.  May be for the best.
	//             Adding features to these by other smaller implementable (virtual) methods, like the new
	//             SeeThroughWaterLine, may be best.
	virtual	BOOL FVisible (CBaseEntity *pEntity );
	virtual	BOOL FVisible (const Vector &vecTargetOrigin );
	virtual BOOL FVisible (const Vector& vecLookerOrigin, CBaseEntity *pEntity );
	virtual BOOL FVisible (const Vector& vecLookerOrigin, const Vector &vecTargetOrigin );

	virtual BOOL SeeThroughWaterLine(void);

	//We use this variables to store each ammo count.
	int ammo_9mm;
	int ammo_357;
	int ammo_bolts;
	int ammo_buckshot;
	int ammo_rockets;
	int ammo_uranium;
	int ammo_hornets;
	int ammo_argrens;
	//Special stuff for grenades and satchels.
	float m_flStartThrow;
	float m_flReleaseThrow;

	//WARNING - unreliable.  Do not use.
	//float fuser4;
	/*
	float fuser5;
	float fuser6;
	float fuser7;
	float fuser8;
	*/

	int m_chargeReady;
	int m_fInAttack;


	enum EGON_FIRESTATE { FIRE_OFF, FIRE_CHARGE };
	int m_fireState;
};



// Ugly technique to override base member functions
// Normally it's illegal to cast a pointer to a member function of a derived class to a pointer to a 
// member function of a base class.  static_cast is a sleezy way around that problem.

#ifdef _DEBUG

#define SetThink( a ) ThinkSet( static_cast <void (CBaseEntity::*)(void)> (a), #a )
#define SetTouch( a ) TouchSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )
#define SetUse( a ) UseSet( static_cast <void (CBaseEntity::*)(	CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a), #a )
#define SetBlocked( a ) BlockedSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )

#else

#define SetThink( a ) m_pfnThink = static_cast <void (CBaseEntity::*)(void)> (a)
#define SetTouch( a ) m_pfnTouch = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)
#define SetUse( a ) m_pfnUse = static_cast <void (CBaseEntity::*)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a)
#define SetBlocked( a ) m_pfnBlocked = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)

#endif


class CPointEntity : public CBaseEntity
{
public:
	//MODDD
	CPointEntity();
	void	Spawn( void );
	virtual int	ObjectCaps( void ) { return CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
private:
};


typedef struct locksounds			// sounds that doors and buttons make when locked/unlocked
{
	string_t	sLockedSound;		// sound a door makes when it's locked
	string_t	sLockedSentence;	// sentence group played when door is locked
	string_t	sUnlockedSound;		// sound a door makes when it's unlocked
	string_t	sUnlockedSentence;	// sentence group played when door is unlocked

	int		iLockedSentence;		// which sentence in sentence group to play next
	int		iUnlockedSentence;		// which sentence in sentence group to play next

	float	flwaitSound;			// time delay between playing consecutive 'locked/unlocked' sounds
	float	flwaitSentence;			// time delay between playing consecutive sentences
	BYTE	bEOFLocked;				// true if hit end of list of locked sentences
	BYTE	bEOFUnlocked;			// true if hit end of list of unlocked sentences
} locksound_t;

void PlayLockSounds(entvars_t *pev, locksound_t *pls, int flocked, int fbutton);

//
// MultiSouce
//

#define MAX_MULTI_TARGETS	16 // maximum number of targets a single multi_manager entity may be assigned.
#define MS_MAX_TARGETS 32

class CMultiSource : public CPointEntity
{
public:
	void Spawn( );
	void KeyValue( KeyValueData *pkvd );

	virtual BOOL IsWorldAffiliated(void);

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	int	ObjectCaps( void ) { return (CPointEntity::ObjectCaps() | FCAP_MASTER); }
	BOOL IsTriggered( CBaseEntity *pActivator );
	void EXPORT Register( void );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE		m_rgEntities[MS_MAX_TARGETS];
	int			m_rgTriggered[MS_MAX_TARGETS];

	int			m_iTotal;
	string_t	m_globalstate;
};


//
// generic Delay entity.
//
class CBaseDelay : public CBaseEntity
{
public:
	float		m_flDelay;
	int			m_iszKillTarget;

	virtual void	KeyValue( KeyValueData* pkvd);
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	//MODDD - new
	void PostRestore(void);
	
	static	TYPEDESCRIPTION m_SaveData[];
	// common member functions
	void SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value );
	void EXPORT DelayThink( void );
};




//MODDD - CBaseAnimating class used to be here, moved to its own file animating.h.

//MODDD - CBaseToggle used to be here, moved to its own file basetoggle.h. Implementations still in subs.cpp or wherever else they were.



//MODDD - interesting. Several of these capabilities are never referred to anywhere else, hinting that they may have been intended for
//        use in telling whether a monster could say, "Duck" or "Jump" for use in dodging or pathfinding

// people gib if their health is <= this at the time of death
#define GIB_HEALTH_VALUE	-30

#define ROUTE_SIZE			8 // how many waypoints a monster can store at one time
#define MAX_OLD_ENEMIES		4 // how many old enemies to remember

#define bits_CAP_DUCK			( 1 << 0 )// crouch
#define bits_CAP_JUMP			( 1 << 1 )// jump/leap
#define bits_CAP_STRAFE			( 1 << 2 )// strafe ( walk/run sideways)
#define bits_CAP_SQUAD			( 1 << 3 )// can form squads
#define bits_CAP_SWIM			( 1 << 4 )// proficiently navigate in water
#define bits_CAP_CLIMB			( 1 << 5 )// climb ladders/ropes
#define bits_CAP_USE			( 1 << 6 )// open doors/push buttons/pull levers
#define bits_CAP_HEAR			( 1 << 7 )// can hear forced sounds
#define bits_CAP_AUTO_DOORS		( 1 << 8 )// can trigger auto doors
#define bits_CAP_OPEN_DOORS		( 1 << 9 )// can open manual doors
#define bits_CAP_TURN_HEAD		( 1 << 10)// can turn head, always bone controller 0

#define bits_CAP_RANGE_ATTACK1	( 1 << 11)// can do a range attack 1
#define bits_CAP_RANGE_ATTACK2	( 1 << 12)// can do a range attack 2
#define bits_CAP_MELEE_ATTACK1	( 1 << 13)// can do a melee attack 1
#define bits_CAP_MELEE_ATTACK2	( 1 << 14)// can do a melee attack 2

#define bits_CAP_FLY			( 1 << 15)// can fly, move all around

#define bits_CAP_DOORS_GROUP    (bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS)

// used by suit voice to indicate damage sustained and repaired type to player

// instant damage

//MODDD - damage types (DMG_...) moved to util_shared.h.  Merged with some other redundant stuff in some
// clientside files.

//MODDD - NOTE - Pay little attention to these.  They're just arbitrary, for helping an iterator method
//see which damage type is which.  For instance, "itbd_Poison" and "DMG_POISON" further below have no link,
//but "itbd_Poison" is useless outside of Player.cpp (and monsters.cpp since they take timeddamage too now)
//while "DMG_POISON" is referred to both by attackers and player.cpp.
#define itbd_Paralyze		0		
#define itbd_NerveGas		1
#define itbd_Poison			2
#define itbd_Radiation		3
#define itbd_DrownRecover	4
#define itbd_Acid			5
#define itbd_SlowBurn		6
#define itbd_SlowFreeze		7
//MODDD - addition.
#define itbd_Bleeding		8
//MODDD - size is now 9. 
#define CDMG_TIMEBASED		9


// when calling KILLED(), a value that governs gib behavior is expected to be 
// one of these three values
#define GIB_NORMAL			0// gib if entity was overkilled
#define GIB_NEVER			1// never gib, no matter how much death damage is done ( freezing, etc )
#define GIB_ALWAYS			2// always gib ( Houndeye Shock, Barnacle Bite )
#define GIB_ALWAYS_NODECAL	3// MODDD - new. Always gib, but no decals (blood splatter on the ground) for spawned gibs.






//... ???
class CBaseMonster;
class CCineMonster;
class CSound;


//MODDD - removed. CBaseMonster's file, basemonster.h, can now be included anywhere else without any assumptions about what's been included by the caller at that point.
//#include "basemonster.h"






//MODDD TODO - move to cbase.h? Why is it here for monsters only?
//
// A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
//
class CGib : public CBaseEntity
{
public:

	void Spawn( const char *szGibModel );
	void Spawn( const char *szGibModel, BOOL spawnsDecal  );

	void EXPORT BounceGibTouch ( CBaseEntity *pOther );
	void EXPORT StickyGibTouch ( CBaseEntity *pOther );
	void EXPORT WaitTillLand( void );
	void		LimitVelocity( void );

	virtual int	ObjectCaps( void ) { return (CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

	
	static void SpawnHeadGib( entvars_t *pevVictim );
	static void SpawnHeadGib( entvars_t *pevVictim, BOOL spawnDecals );
	//MODDD - this version accepts a different origin from the default determined typically.
	static void SpawnHeadGib( entvars_t *pevVictim, const Vector gibSpawnOrigin );
	static void SpawnHeadGib( entvars_t *pevVictim, const Vector gibSpawnOrigin, BOOL spawnDecals );
	
	//MODDD - third parameter chagned from "human" (as in, "should we spawn human gibs or, otherwise, alien gibs"?) to "argSpawnGibID", to call upon a particular aryGibInfo choice (element of that ID).
	
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int argSpawnGibID);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int argSpawnGibID, BOOL spawnDecals);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, int argSpawnGibID, BOOL spawnDecals, int argBloodColor);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const GibInfo_t& gibInfoChoice);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const GibInfo_t& gibInfoChoice, BOOL spawnDecals);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const GibInfo_t& gibInfoChoice, BOOL spawnDecals, int argBloodColor);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* argGibPath, int gibBodyMin, int gibBodyMax);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* argGibPath, int gibBodyMin, int gibBodyMax, BOOL spawnDecals);
	static void SpawnRandomGibs(entvars_t* pevVictim, int cGibs, const char* argGibPath, int gibBodyMin, int gibBodyMax, BOOL spawnDecals, int argBloodColor);

	
	static void SpawnStickyGibs( entvars_t *pevVictim, Vector vecOrigin, int cGibs );
	static void SpawnStickyGibs( entvars_t *pevVictim, Vector vecOrigin, int cGibs, BOOL spawnDecals );
	
	


	int		m_bloodColor;
	int		m_cBloodDecals;
	int		m_material;
	float	m_lifeTime;



	//MODDD - stubs.  Because why not.
	GENERATE_TRACEATTACK_PROTOTYPE
	GENERATE_TAKEDAMAGE_PROTOTYPE

	float massInfluence(void);

};




//MODDD - CBaseButton used to be here, moved to its own file basebutton.h. Implementations still in buttons.cpp or wherever they were.

//
// Weapons 
//

#define BAD_WEAPON 0x00007FFF

//
// Converts a entvars_t * to a class pointer
// It will allocate the class and entity if necessary
//
template <class T> T * GetClassPtr( T *a )
{
	entvars_t *pev = (entvars_t *)a;

	// allocate entity if necessary
	if (pev == NULL)
		pev = VARS(CREATE_ENTITY());

	// get the private data
	a = (T *)GET_PRIVATE(ENT(pev));

	if (a == NULL) 
	{
		// allocate private data 
		a = new(pev) T;
		a->pev = pev;
	}
	return a;
}


/*
bit_PUSHBRUSH_DATA | bit_TOGGLE_DATA
bit_MONSTER_DATA
bit_DELAY_DATA
bit_TOGGLE_DATA | bit_DELAY_DATA | bit_MONSTER_DATA
bit_PLAYER_DATA | bit_MONSTER_DATA
bit_MONSTER_DATA | CYCLER_DATA
bit_LIGHT_DATA
path_corner_data
bit_MONSTER_DATA | wildcard_data
bit_MONSTER_DATA | bit_GROUP_DATA
boid_flock_data
boid_data
CYCLER_DATA
bit_ITEM_DATA
bit_ITEM_DATA | func_hud_data
bit_TOGGLE_DATA | bit_ITEM_DATA
EOFFSET
env_sound_data
env_sound_data
push_trigger_data
*/

#define TRACER_FREQ		4			// Tracers fire every 4 bullets

//MODDD - SelAmmo moved to enginecallback.h to be accessible to all serverside.


// this moved here from world.cpp, to allow classes to be derived from it
//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================
class CWorld : public CBaseEntity
{
public:
	//MODDD - constructor?
	CWorld(void);

	void Activate(void);
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );


	GENERATE_TRACEATTACK_PROTOTYPE_VIRTUAL
	GENERATE_TAKEDAMAGE_PROTOTYPE_VIRTUAL

	
	//////////////////////////////////////////////////////////////////////////////////
	static TYPEDESCRIPTION m_SaveData[];
	virtual int Save( CSave &save ); 
	virtual int Restore( CRestore &restore );
	//////////////////////////////////////////////////////////////////////////////////
	//void EXPORT WorldThink( void );

	void applyLoadedCustomMapSettingsToGlobal(void);
	void getCustomMapSettingsFromGlobal(void);


	
	//Must reset these to defaults in case of changing the map.
	float m_fl_node_linktest_height;
	float m_fl_node_hulltest_height;
	BOOL m_f_node_hulltest_heightswap;
	BOOL m_f_map_anyAirNodes;

	BOOL IsWorld(void);
	BOOL IsWorldAffiliated(void);


};



#endif //END OF #ifndef CBASE_H