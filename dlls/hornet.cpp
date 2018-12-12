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
//=========================================================
// Hornets
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include "basemonster.h"
#include	"weapons.h"
#include	"soundent.h"
#include	"hornet.h"
#include	"gamerules.h"


int iHornetTrail;
int iHornetPuff;





#define ROCKET_TRAIL 2
extern unsigned short g_sTrail;
extern unsigned short g_sTrailRA;

extern float global_trailTypeTest;



extern float global_hornetTrail;
extern float global_hornetTrailSolidColor;

extern float global_hornetDeathModEasy;
extern float global_hornetDeathModMedium;
extern float global_hornetDeathModHard;

extern float global_hornetZoomPuff;
extern float global_hornetSpiral;
extern float global_hornetSpeedMulti;
extern float global_hornetSpeedDartMulti;

extern float global_agruntHornetRandomness;


extern float global_agruntHornetRandomness;
extern float global_hornetSpiralPeriod;
extern float global_hornetSpiralAmplitude;


LINK_ENTITY_TO_CLASS( hornet, CHornet );

//=========================================================
// Save/Restore
//=========================================================
TYPEDESCRIPTION	CHornet::m_SaveData[] = 
{
	DEFINE_FIELD( CHornet, m_flStopAttack, FIELD_TIME ),
	DEFINE_FIELD( CHornet, m_iHornetType, FIELD_INTEGER ),
	DEFINE_FIELD( CHornet, m_flFlySpeed, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CHornet, CBaseMonster );

//=========================================================
// don't let hornets gib, ever.
//=========================================================



BOOL CHornet::useSpiral(void){
	if(global_hornetSpiral == 1 || (global_hornetSpiral == 2 && g_iSkillLevel == SKILL_HARD) ){
		return TRUE;
	}else{
		return FALSE;
	}
}

//fetch the fitting "hornetDeathMod" CVar.
float CHornet::getDifficultyMod(void){
	switch(g_iSkillLevel){
	case SKILL_EASY:
		return global_hornetDeathModEasy;
	break;
	case SKILL_MEDIUM:
		return global_hornetDeathModMedium;
	break;
	case SKILL_HARD:
		return global_hornetDeathModHard;
	break;
	default:  //???
		return global_hornetDeathModEasy;
	break;
	};
}




GENERATE_TRACEATTACK_IMPLEMENTATION(CHornet)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CBaseMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CHornet)
{
	
	int hornetDeathMode = (int) getDifficultyMod();
	//notice: if hornetDeathMode == 3, "takeDamage" cannot gib.  "deadTakeDamage" can though.
	//type 4 is always gib.   Type 2 is leave it to the natural way (can gib anytime if the damage wanted too).
	if(hornetDeathMode == 0 || hornetDeathMode == 1 || hornetDeathMode == 3){
		// filter these bits a little.
		bitsDamageType &= ~ ( DMG_ALWAYSGIB );
		bitsDamageType |= DMG_NEVERGIB;
	}else if(hornetDeathMode == 4){
		//now we are obliged to gib instead!
		bitsDamageType |= DMG_ALWAYSGIB ;
		bitsDamageType &= ~ (DMG_NEVERGIB);
	}

	//easyForcePrintLine("AM I GON DIE?  %.2f  %d  %d %.2f : %.2f    %d %d   %s %d   %s", pev->health, pev->deadflag, (m_pfnThink!=NULL), pev->nextthink, gpGlobals->time, this->m_IdealMonsterState, this->m_MonsterState, this->getScheduleName(), this->getTaskNumber(), getClassname() );

	int test = GENERATE_TAKEDAMAGE_PARENT_CALL(CBaseMonster);

	//easyForcePrintLine("AM I GON DIE? DEAD?  %.2f  %d   %d %.2f : %.2f   %d %d   %s %d", pev->health, pev->deadflag, (m_pfnThink!=NULL), pev->nextthink, gpGlobals->time, this->m_IdealMonsterState, this->m_MonsterState, this->getScheduleName(), this->getTaskNumber() );
	return test;

}




CHornet::CHornet(){


	hornetTouchedAnything = FALSE;
	hornetPseudoNextThink = -1;

	vecFlightDirTrue = Vector(0,0,0);
	vecFlightDirAlttt = Vector(0,0,0);
	vecFlightDirMem = Vector(0,0,0);

	spiralStartTime = 0;

}



//=========================================================
//=========================================================
void CHornet :: Spawn( void )
{
	
	CHornet();

	Precache();

	//MODDD - this stops hornets from falling back to the default grey-blood
	//(this is noticable if killed by being in the blast radius of an explosion)
	m_bloodColor		= BLOOD_COLOR_GREEN;

	pev->movetype	= MOVETYPE_FLY;
	pev->solid		= SOLID_BBOX;
	pev->takedamage = DAMAGE_YES;
	pev->flags		|= FL_MONSTER;

	//MODDD - possible skill CVar intervention...
	//pev->health		= 1;// weak!
	pev->health = gSkillData.monHealthHornet;

	
	if ( g_pGameRules->IsMultiplayer() )
	{
		// hornets don't live as long in multiplayer
		m_flStopAttack = gpGlobals->time + 3.5;
	}
	else
	{
		m_flStopAttack	= gpGlobals->time + 5.0;
	}

	m_flFieldOfView = 0.9; // +- 25 degrees


	
	if ( RANDOM_LONG ( 1, 5 ) <= 2 )
	{
		m_iHornetType = HORNET_TYPE_RED;
		m_flFlySpeed = HORNET_RED_SPEED * global_hornetSpeedMulti;
	}
	else
	{
		m_iHornetType = HORNET_TYPE_ORANGE;
		m_flFlySpeed = HORNET_ORANGE_SPEED * global_hornetSpeedMulti;
	}



	SET_MODEL(ENT( pev ), "models/hornet.mdl");
	UTIL_SetSize( pev, Vector( -4, -4, -4 ), Vector( 4, 4, 4 ) );

	SetTouch( &CHornet::DieTouch );
	SetThink( &CHornet::StartTrack );

	edict_t *pSoundEnt = pev->owner;
	if ( !pSoundEnt )
		pSoundEnt = edict();

	if ( !FNullEnt(pev->owner) && (pev->owner->v.flags & FL_CLIENT) )
	{
		pev->dmg = gSkillData.plrDmgHornet;
	}
	else
	{
		// no real owner, or owner isn't a client. 
		pev->dmg = gSkillData.monDmgHornet;
	}


	
	pev->nextthink = gpGlobals->time + 0.1;
	ResetSequenceInfo( );

}


extern int global_useSentenceSave;

void CHornet :: Precache()
{
	PRECACHE_MODEL("models/hornet.mdl");

	global_useSentenceSave = TRUE;

	PRECACHE_SOUND( "agrunt/ag_fire1.wav" );
	PRECACHE_SOUND( "agrunt/ag_fire2.wav" );
	PRECACHE_SOUND( "agrunt/ag_fire3.wav" );

	PRECACHE_SOUND( "hornet/ag_buzz1.wav" );
	PRECACHE_SOUND( "hornet/ag_buzz2.wav" );
	PRECACHE_SOUND( "hornet/ag_buzz3.wav" );

	PRECACHE_SOUND( "hornet/ag_hornethit1.wav" );
	PRECACHE_SOUND( "hornet/ag_hornethit2.wav" );
	PRECACHE_SOUND( "hornet/ag_hornethit3.wav" );

	iHornetPuff = PRECACHE_MODEL( "sprites/muz1.spr" );
	iHornetTrail = PRECACHE_MODEL("sprites/laserbeam.spr");

	global_useSentenceSave = FALSE;
}	

//=========================================================
// hornets will never get mad at each other, no matter who the owner is.
//=========================================================
int CHornet::IRelationship ( CBaseEntity *pTarget )
{
	if ( pTarget->pev->modelindex == pev->modelindex )
	{
		return R_NO;
	}

	return CBaseMonster :: IRelationship( pTarget );
}

//=========================================================
// ID's Hornet as their owner
//=========================================================
int CHornet::Classify ( void )
{

	if ( pev->owner && pev->owner->v.flags & FL_CLIENT)
	{
		return CLASS_PLAYER_BIOWEAPON;
	}

	return	CLASS_ALIEN_BIOWEAPON;
}

//=========================================================
// StartTrack - starts a hornet out tracking its target
//=========================================================
void CHornet :: StartTrack ( void )
{
	IgniteTrail();

	SetTouch( &CHornet::TrackTouch );
	SetThink( &CHornet::TrackTarget );


	//whatever that may happen to be...
	vecFlightDirTrue = pev->velocity;

	if(useSpiral() == FALSE){
		pev->nextthink = gpGlobals->time + 0.1;
	}else{
		spiralStartTime = gpGlobals->time;
		vecFlightDirMem = pev->velocity.Normalize();
		hornetPseudoNextThink = gpGlobals->time + 0.1;
		pev->nextthink = gpGlobals->time;
	}
}

//=========================================================
// StartDart - starts a hornet out just flying straight.
//=========================================================
void CHornet :: StartDart ( void )
{
	IgniteTrail();

	SetTouch( &CHornet::DartTouch );

	SetThink( &CBaseEntity::SUB_Remove );
	pev->nextthink = gpGlobals->time + 4;
}

void CHornet::IgniteTrail( void )
{
/*

  ted's suggested trail colors:

r161
g25
b97

r173
g39
b14

old colors
		case HORNET_TYPE_RED:
			WRITE_BYTE( 255 );   // r, g, b
			WRITE_BYTE( 128 );   // r, g, b
			WRITE_BYTE( 0 );   // r, g, b
			break;
		case HORNET_TYPE_ORANGE:
			WRITE_BYTE( 0   );   // r, g, b
			WRITE_BYTE( 100 );   // r, g, b
			WRITE_BYTE( 255 );   // r, g, b
			break;
	
*/





	//MODDD - quake dot trail?
	//easyForcePrintLine("YOU STUPID lover %d %d", (int)global_trailTypeTest, (int)global_hornetTrail);

	if(global_trailTypeTest > -1){
		//This was just for a test.  Enable (along with some other things in place), and this should make mp5 grenades fly with a trail of grey dots.
		PLAYBACK_EVENT_FULL (FEV_GLOBAL, this->edict(), g_sTrail, 0.0, 
		(float *)&this->pev->origin, (float *)&this->pev->angles, 0.7, 0.0, this->entindex(), (int)global_trailTypeTest, 0, 0);
	}else if(global_trailTypeTest == -2){
		//This was just for a test.  Enable (along with some other things in place), and this should make mp5 grenades fly with a trail of grey dots.
		PLAYBACK_EVENT_FULL (FEV_GLOBAL, this->edict(), g_sTrailRA, 0.0, 
		(float *)&this->pev->origin, (float *)&this->pev->angles, 0.7, 0.0, this->entindex(), 0, 0, 0);
	
	}else if(global_hornetTrail == 1 || global_hornetTrail == 2){

		//NOTE: particle type is "6", 3rd to last parameter here.
		PLAYBACK_EVENT_FULL (FEV_GLOBAL, this->edict(), g_sTrail, 0.0, 
		(float *)&this->pev->origin, (float *)&this->pev->angles, 0.7, 0.0, this->entindex(), 6, 0, 0);
	}
	
	

	if(global_hornetTrail == 0 || global_hornetTrail == 2){


	int clrChoice1[3];
	int clrChoice2[3];
	switch( (int)global_hornetTrailSolidColor ){
	case 0:
		//retail
		clrChoice1[0] = 179;
		clrChoice1[1] = 39;
		clrChoice1[2] = 14;
		clrChoice2[0] = 255;
		clrChoice2[1] = 128;
		clrChoice2[2] = 0;
	break;
	case 1:
		//alpha
		clrChoice1[0] = 255;
		clrChoice1[1] = 128;
		clrChoice1[2] = 0;
		clrChoice2[0] = 0;
		clrChoice2[1] = 100;
		clrChoice2[2] = 255;
	break;
	case 2:
		//tim's colors?
		clrChoice1[0] = 161;
		clrChoice1[1] = 25;
		clrChoice1[2] = 97;
		clrChoice2[0] = 173;
		clrChoice2[1] = 39;
		clrChoice2[2] = 14;
	break;
	default:
		//??? retail.
		clrChoice1[0] = 179;
		clrChoice1[1] = 39;
		clrChoice1[2] = 14;
		clrChoice2[0] = 255;
		clrChoice2[1] = 128;
		clrChoice2[2] = 0;
	break;
	}


	// trail
	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE(  TE_BEAMFOLLOW );
		WRITE_SHORT( entindex() );	// entity
		WRITE_SHORT( iHornetTrail );	// model
		WRITE_BYTE( 10 ); // life
		WRITE_BYTE( 2 );  // width

		
		switch ( m_iHornetType )
		{
		case HORNET_TYPE_RED:
			//MODDD - this used to be 179, 39, 14.
			WRITE_BYTE(clrChoice1[0]);   // r, g, b
			WRITE_BYTE(clrChoice1[1]);   // r, g, b
			WRITE_BYTE(clrChoice1[2]);   // r, g, b
			break;
		case HORNET_TYPE_ORANGE:
			//MODDD - this used to be 255, 128, 0.
			WRITE_BYTE(clrChoice2[0]);   // r, g, b
			WRITE_BYTE(clrChoice2[1]);   // r, g, b
			WRITE_BYTE(clrChoice2[2]);   // r, g, b
			break;
		}

		WRITE_BYTE( 128 );	// brightness

	MESSAGE_END();


	}//END OF if(global_hornetTrail is 0 or 2)
	//NOTE: global_hornetTrail of 3 (or anything else really too) means no trail.


}

//=========================================================
// Hornet is flying, gently tracking target
//=========================================================
void CHornet :: TrackTarget ( void )
{
	Vector	vecFlightDir;
	Vector	vecDirToEnemy;
	float	flDelta;






	BOOL timeBlock = FALSE;

	if(useSpiral() == FALSE){
		//normal.
		vecFlightDirAlttt = Vector(0,0,0);

	}else{

		pev->nextthink = gpGlobals->time;
		//only proceed, if we wanted to think here.
		if(gpGlobals->time <= hornetPseudoNextThink){
			timeBlock = TRUE;
		}else{
			//no time block, but reset this...
			//MODDD
			//hornetPseudoNextThink = gpGlobals->time + 0.1;
		}

	}

	
		


	if(!timeBlock){

		StudioFrameAdvance( );

		if (gpGlobals->time > m_flStopAttack)
		{
			SetTouch( NULL );
			SetThink( &CBaseEntity::SUB_Remove );
			pev->nextthink = gpGlobals->time + 0.1;
			return;
		}

		//MODDD - if we're dead, none of the logic below applies (AI).
		if(pev->deadflag != DEAD_NO){
			return;
		}


		// UNDONE: The player pointer should come back after returning from another level
		if ( m_hEnemy == NULL )
		{// enemy is dead.
			Look( 512 );
			m_hEnemy = BestVisibleEnemy( );
		}

	
		if ( m_hEnemy != NULL && FVisible( m_hEnemy ))
		{
			//m_vecEnemyLKP = ...;
			setEnemyLKP( m_hEnemy->BodyTarget( pev->origin ) );
		}
		else
		{
			//m_vecEnemyLKP = m_vecEnemyLKP + pev->velocity * m_flFlySpeed * 0.1;
			
			//m_vecEnemyLKP = ...;
			setEnemyLKP(m_vecEnemyLKP + vecFlightDirTrue * m_flFlySpeed * 0.1);

		}

		vecDirToEnemy = ( m_vecEnemyLKP - pev->origin ).Normalize();

		/*
		if (pev->velocity.Length() < 0.1)
			vecFlightDir = vecDirToEnemy;
		else 
			vecFlightDir = pev->velocity.Normalize();
		*/
		if (vecFlightDirTrue.Length() < 0.1)
			vecFlightDir = vecDirToEnemy;
		else 
			vecFlightDir = vecFlightDirTrue.Normalize();
		





		// measure how far the turn is, the wider the turn, the slow we'll go this time.
		flDelta = DotProduct ( vecFlightDir, vecDirToEnemy );
	
		vecFlightDirMem = vecFlightDir;

	}







	




	if(!timeBlock){




	if ( flDelta < 0.5 )
	{// hafta turn wide again. play sound
		switch (RANDOM_LONG(0,2))
		{
		case 0:	EMIT_SOUND_FILTERED( ENT(pev), CHAN_VOICE, "hornet/ag_buzz1.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
		case 1:	EMIT_SOUND_FILTERED( ENT(pev), CHAN_VOICE, "hornet/ag_buzz2.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
		case 2:	EMIT_SOUND_FILTERED( ENT(pev), CHAN_VOICE, "hornet/ag_buzz3.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
		}
	}

	if ( flDelta <= 0 && m_iHornetType == HORNET_TYPE_RED )
	{// no flying backwards, but we don't want to invert this, cause we'd go fast when we have to turn REAL far.
		flDelta = 0.25;
	}

	//pev->velocity = ( vecFlightDir + vecDirToEnemy).Normalize();
	vecFlightDirTrue = ( vecFlightDir + vecDirToEnemy).Normalize();

	


	if ( (global_agruntHornetRandomness > 0) && (pev->owner && (pev->owner->v.flags & FL_MONSTER)) )
	{
		// random pattern only applies to hornets fired by monsters, not players. 

		//pev->velocity.x += RANDOM_FLOAT ( -0.10, 0.10 );// scramble the flight dir a bit.
		//pev->velocity.y += RANDOM_FLOAT ( -0.10, 0.10 );
		//pev->velocity.z += RANDOM_FLOAT ( -0.10, 0.10 );
		vecFlightDirTrue.x += RANDOM_FLOAT ( -global_agruntHornetRandomness, global_agruntHornetRandomness );// scramble the flight dir a bit.
		vecFlightDirTrue.y += RANDOM_FLOAT ( -global_agruntHornetRandomness, global_agruntHornetRandomness );
		vecFlightDirTrue.z += RANDOM_FLOAT ( -global_agruntHornetRandomness, global_agruntHornetRandomness );


	}


	if(useSpiral() == FALSE){
		switch ( m_iHornetType )
		{
			case HORNET_TYPE_RED:
				//pev->velocity = pev->velocity * ( m_flFlySpeed * flDelta );// scale the dir by the ( speed * width of turn )
				vecFlightDirTrue = vecFlightDirTrue * ( m_flFlySpeed * flDelta );// scale the dir by the ( speed * width of turn )
				pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1, 0.3 );
				break;
			case HORNET_TYPE_ORANGE:
				//pev->velocity = pev->velocity * m_flFlySpeed;// do not have to slow down to turn.
				vecFlightDirTrue = vecFlightDirTrue * m_flFlySpeed;// do not have to slow down to turn.
				pev->nextthink = gpGlobals->time + 0.1;// fixed think time
				break;
		}
	}else{
		switch ( m_iHornetType )
		{
			case HORNET_TYPE_RED:
				//pev->velocity = pev->velocity * ( m_flFlySpeed * flDelta );// scale the dir by the ( speed * width of turn )
				vecFlightDirTrue = vecFlightDirTrue * ( m_flFlySpeed * flDelta );// scale the dir by the ( speed * width of turn )
				hornetPseudoNextThink = gpGlobals->time + RANDOM_FLOAT( 0.1, 0.3 );
				break;
			case HORNET_TYPE_ORANGE:
				//pev->velocity = pev->velocity * m_flFlySpeed;// do not have to slow down to turn.
				vecFlightDirTrue = vecFlightDirTrue * m_flFlySpeed;// do not have to slow down to turn.
				hornetPseudoNextThink = gpGlobals->time + 0.1;// fixed think time
				break;
		}
	}

	//pev->angles = UTIL_VecToAngles (pev->velocity);
	pev->angles = UTIL_VecToAngles(vecFlightDirTrue);



	pev->solid = SOLID_BBOX;

	// if hornet is close to the enemy, jet in a straight line for a half second.
	// (only in the single player game)
	if ( m_hEnemy != NULL && !g_pGameRules->IsMultiplayer() )
	{
		if ( flDelta >= 0.4 && ( pev->origin - m_vecEnemyLKP ).Length() <= 300 )
		{

			if(global_hornetZoomPuff == 1){
				MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
					WRITE_BYTE( TE_SPRITE );
					WRITE_COORD( pev->origin.x);	// pos
					WRITE_COORD( pev->origin.y);
					WRITE_COORD( pev->origin.z);
					WRITE_SHORT( iHornetPuff );		// model
					// WRITE_BYTE( 0 );				// life * 10
					WRITE_BYTE( 2 );				// size * 10
					WRITE_BYTE( 128 );			// brightness
				MESSAGE_END();
			}

			switch (RANDOM_LONG(0,2))
			{
			case 0:	EMIT_SOUND_FILTERED( ENT(pev), CHAN_VOICE, "hornet/ag_buzz1.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
			case 1:	EMIT_SOUND_FILTERED( ENT(pev), CHAN_VOICE, "hornet/ag_buzz2.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
			case 2:	EMIT_SOUND_FILTERED( ENT(pev), CHAN_VOICE, "hornet/ag_buzz3.wav", HORNET_BUZZ_VOLUME, ATTN_NORM);	break;
			}
			
			//pev->velocity = pev->velocity * global_hornetSpeedDartMulti;
			vecFlightDirTrue = vecFlightDirTrue * global_hornetSpeedDartMulti;
			
			pev->nextthink = gpGlobals->time + 1.0;
			// don't attack again
			m_flStopAttack = gpGlobals->time;
		}
	}


	}//END OF the thing.


	
	pev->velocity = vecFlightDirTrue; //+ vecFlightDirAlttt;

	//is this ok?



	if(useSpiral() == TRUE){
		
		//can we just push the velocity ?
		
		float timeVal = spiralStartTime-gpGlobals->time;

		float xShift = cos(timeVal / global_hornetSpiralPeriod);
		float yShift = sin(timeVal / global_hornetSpiralPeriod);

		float len = global_hornetSpiralAmplitude;
		

		//get vector perpendicular to vecFlightDir ...
		//Vector perp = CrossProduct(vecFlightDir, Vector(0,0,1));
		//faces "right" of movement.

		//Time for linear algebra!
		//Say we have our vector...
		//(x, y, z)
		//Now imagine it on a 
		//......

		Vector ang = UTIL_velocityToAngles(vecFlightDirTrue);
		

		/*
		Vector angUp = Vector(ang.x + 90, ang.y, 0);
		Vector angRight = Vector(ang.x, ang.y + 90, 0);

		Vector upVect = UTIL_VecGetForward(angUp);
		Vector rightVect = UTIL_VecGetForward(angRight);
		*/

		
		Vector crossProto = CrossProduct(vecFlightDirTrue, Vector(0, 0, 1)).Normalize();
		Vector crossProto2 = CrossProduct(vecFlightDirTrue, crossProto).Normalize();
		Vector crossProto3 = CrossProduct(vecFlightDirTrue, crossProto2).Normalize();


		//UTIL_drawLineFrame(this->pev->origin, this->pev->origin + crossProto*6, 4, 255, 0, 0);
		//UTIL_drawLineFrame(this->pev->origin, this->pev->origin + crossProto2*6, 4, 0, 255, 0);
		//UTIL_drawLineFrame(this->pev->origin, this->pev->origin + crossProto3*6, 4, 0, 0, 255);
		
		//red = across,  green = vertical.

		//pev->origin = pev->origin + xShift*len*crossProto*2;
		//pev->origin = pev->origin + yShift*len*crossProto2*2;

		//pev->velocity = pev->velocity + xShift*len*crossProto*9;
		//pev->velocity = pev->velocity + yShift*len*crossProto2*9;


		

		vecFlightDirAlttt = xShift*len*crossProto*1 + yShift*len*crossProto2*1;
		
		pev->velocity = vecFlightDirTrue  +vecFlightDirAlttt;
		
		//easyForcePrintLine("WHATTTTT %.2f %.2f", xShift, yShift);

		
	}






}

//=========================================================
// Tracking Hornet hit something
//=========================================================
void CHornet :: TrackTouch ( CBaseEntity *pOther )
{


	hornetTouchedAnything = TRUE;

	if ( pOther->edict() == pev->owner || pOther->pev->modelindex == pev->modelindex )
	{// bumped into the guy that shot it.
		pev->solid = SOLID_NOT;
		return;
	}

	if ( IRelationship( pOther ) <= R_NO )
	{
		// hit something we don't want to hurt, so turn around.
		/*
		pev->velocity = pev->velocity.Normalize();

		pev->velocity.x *= -1;
		pev->velocity.y *= -1;

		pev->origin = pev->origin + pev->velocity * 4; // bounce the hornet off a bit.
		pev->velocity = pev->velocity * m_flFlySpeed;
		*/
		vecFlightDirTrue = vecFlightDirTrue.Normalize();

		vecFlightDirTrue.x *= -1;
		vecFlightDirTrue.y *= -1;

		pev->origin = pev->origin + vecFlightDirTrue * 4; // bounce the hornet off a bit.
		vecFlightDirTrue = vecFlightDirTrue * m_flFlySpeed;

		pev->velocity = vecFlightDirTrue;

		return;
	}

	DieTouch( pOther );
}

void CHornet::DartTouch( CBaseEntity *pOther )
{
	DieTouch( pOther );
}

void CHornet::DieTouch ( CBaseEntity *pOther )
{
	if ( pOther && pOther->pev->takedamage )
	{// do the damage

		switch (RANDOM_LONG(0,2))
		{// buzz when you plug someone
			case 0:	EMIT_SOUND_FILTERED( ENT(pev), CHAN_VOICE, "hornet/ag_hornethit1.wav", 1, ATTN_NORM);	break;
			case 1:	EMIT_SOUND_FILTERED( ENT(pev), CHAN_VOICE, "hornet/ag_hornethit2.wav", 1, ATTN_NORM);	break;
			case 2:	EMIT_SOUND_FILTERED( ENT(pev), CHAN_VOICE, "hornet/ag_hornethit3.wav", 1, ATTN_NORM);	break;
		}
			
		pOther->TakeDamage( pev, VARS( pev->owner ), pev->dmg, DMG_BULLET );
	}

	pev->modelindex = 0;// so will disappear for the 0.1 secs we wait until NEXTTHINK gets rid
	pev->solid = SOLID_NOT;

	SetThink ( &CBaseEntity::SUB_Remove );
	pev->nextthink = gpGlobals->time + 1;// stick around long enough for the sound to finish!
}



//SetThink ( &CBaseEntity::SUB_Remove );
//pev->nextthink = gpGlobals->time + 1;// stick around long enough for the sound to finish!
////Do we need to force a timed-death since being shot down (hitting the ground)?  Doubt it.













GENERATE_DEADTAKEDAMAGE_IMPLEMENTATION(CHornet){
	
	switch ( (int) getDifficultyMod()){
		case 0:
			//nothing, should never be reached, but... just in case?
			return 1;
		break;
		case 1:
			//no gibbing, only purpose.
			return 1;
		break;
		case 2:

		break;
		case 3:

		break;
		case 4:

		break;

	}//MODD - end of switch.



	//any point here?
	//MODDD - been a while since I looked at Hornets. Is there a reason this was cutoff at here (early return always)? Investigate.
	//Below here could've started as a complete copy of CBaseMonster's own "DeadTakeDamage" script, and whatever change was made then was deemed unwanted.
	//So may as well just call the parent method of CBaseMonster instead if no changes are to be made in the copy below.
	return GENERATE_DEADTAKEDAMAGE_PARENT_CALL(CBaseMonster);
	
	
	
	

	Vector			vecDir;

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector( 0, 0, 0 );
	if (!FNullEnt( pevInflictor ))
	{
		CBaseEntity *pInflictor = CBaseEntity :: Instance( pevInflictor );
		if (pInflictor)
		{
			vecDir = ( pInflictor->Center() - Vector ( 0, 0, 10 ) - Center() ).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}


	//MODDD - TODO: intriguing... see later.
#if 0// turn this back on when the bounding box issues are resolved.

	pev->flags &= ~FL_ONGROUND;
	pev->origin.z += 1;
	
	// let the damage scoot the corpse around a bit.
	if ( !FNullEnt(pevInflictor) && (pevAttacker->solid != SOLID_TRIGGER) )
	{
		pev->velocity = pev->velocity + vecDir * -DamageForce( flDamage );
	}

#endif

	// kill the corpse if enough damage was done to destroy the corpse and the damage is of a type that is allowed to destroy the corpse.
	if ( bitsDamageType & DMG_GIB_CORPSE )
	{
		if ( pev->health <= flDamage )
		{
			pev->health = -50;
			//MODDD - added in case this gets used again. Have a pevInflictor, may as well send it now.
			Killed( pevInflictor, pevAttacker, GIB_ALWAYS );
			return 0;
		}
		// Accumulate corpse gibbing damage, so you can gib with multiple hits
		pev->health -= flDamage * 0.1;
	}
	
	return 1;

}
	










/*
0: retail.
1: shot-down (flies to the ground) for any death, not gibbable at all.
2: shot-down for dying by non-gibbing causes (like shot). Otherwise, gibbed (disappears).
3: shot-down for dying anytime. Can be gibbed after this.
4: always gib (disappears).
*/

//MODDD - new
GENERATE_KILLED_IMPLEMENTATION(CHornet){
	

	
			//pev->deadflag = DEAD_DEAD;
			//return;

	int hornetDeathMode = (int) getDifficultyMod();

	UTIL_SetSize(pev, Vector(0,0,0), Vector(0,0,0));
	pev->solid = SOLID_NOT;
	

	BOOL canGib = TRUE;



	switch ( hornetDeathMode ){
		case 0:
			//nothing.  Usual way, actually.
			GENERATE_KILLED_PARENT_CALL(CBaseMonster);
			return;
		break;
		case 1:
			//straight to "DEAD_DEAD"
			////pev->deadflag = DEAD_DEAD;
			//pev->velocity = Vector(0,0,0);

			canGib = FALSE;

		break;
		case 2:
			//pev->movetype	= MOVETYPE_TOSS;
			//pev->deadflag = DEAD_DEAD;
			//pev->velocity = Vector(0,0,0);

		break;
		case 3:
			
			if(pev->deadflag == DEAD_NO){
				//no gibbing.
				canGib = FALSE;
			}

			//pev->deadflag = DEAD_DEAD;
			//pev->velocity = Vector(0,0,0);




		break;
		case 4:
			//FORCE GIB... remove me.   eh, leave it to the method.
			//cleanDelete();
			//return;
		break;

	}//MODD - end of switch.




	
	
	//if ( HasMemory( bits_MEMORY_KILLED ) )
	if(pev->deadflag != DEAD_DEAD){
		//BEEP.  Will despawn at this time.
		//m_flStopAttack = gpGlobals->time + 2;
		//SetThink( &CBaseEntity::SUB_Remove );
		//pev->nextthink = gpGlobals->time + 2;

		
		//SetThink( NULL );
		SetThink( &CBaseEntity::SUB_Remove );
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(3.4, 5.3);

		pev->deadflag = DEAD_DEAD;
		
		SetTouch( NULL );

		pev->velocity = Vector(pev->velocity.x * 0.18, pev->velocity.y * 0.18, pev->velocity.z * 0.46);

	}






	//NOTICE: natural "killed"'s  "BecomeDead" method will set the movetype to "toss" to make this fall.
	


	unsigned int	cCount = 0;
	BOOL			fDone = FALSE;

	//MODDD - EMERGENCY TEST FLAG
	iAmDead = TRUE;

	//MODDD - if stuck to a barnacle, not anymore.
	barnacleLocked = FALSE;
	
	//easyForcePrintLine("SO DO WE GIB IT %.2f %d", pev->health, iGib);


	if ( canGib && HasMemory( bits_MEMORY_KILLED ) )
	{
		if ( ShouldGibMonster( iGib ) )
			cleanDelete();
		return;
	}

	Remember( bits_MEMORY_KILLED );

	// clear the deceased's sound channels.(may have been firing or reloading when killed)
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);

	//pDeathSounds
	//MODDD - TODO: for voice maybe at some point?  Unless that would stop death-cries or something.

	m_IdealMonsterState = MONSTERSTATE_DEAD;
	// Make sure this condition is fired too (TakeDamage breaks out before this happens on death)
	SetConditions( bits_COND_LIGHT_DAMAGE );
	

	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	if ( pOwner )
	{
		pOwner->DeathNotice( pev );
	}

	if	( canGib && ShouldGibMonster( iGib ) )
	{
		cleanDelete();
		return;
	}
	else if ( pev->flags & FL_MONSTER )
	{
		SetTouch( NULL );
		BecomeDead();
		//does the "cleanDelete();
	}
	
	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}

	//MODDD - just to make sure.
	m_bitsDamageType = 0;
	m_bitsDamageTypeMod = 0;

	
	//pev->enemy = ENT( pevAttacker );//why? (sjb)
	
	m_IdealMonsterState = MONSTERSTATE_DEAD;



}//END OF Killed


float CHornet::massInfluence(void){
	return 0.01f;
}//END OF massInfluence




