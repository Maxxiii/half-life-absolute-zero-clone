/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Hassault - started by Osiris / OsirisGodoftheDead / THE_YETI
//=========================================================

// UNDONE: Don't flinch every time you get hit


//TODO - the condition for hassault being in frozen sequence 0, frame 0 should
//extend to spinning, period, not necessariy looking at something hostile?

//NOTICE - ACT_SIGNAL3, the retreat sequence, is completely unused!
//Also get ACT_SIGNAL1 and ACT_SIGNAL2 (are those hooked up in the modern hassault model?) to play like they do
//for hgrunts, take into account squad role maybe if they did (hassault is typically leader unless there are multiple hassaults in the squad), etc.


#include "hassault.h"
#include "schedule.h"
#include "weapons.h"
#include "defaultai.h"
#include "util_model.h"
#include "soundent.h"

#include "cvar_custom_info.h"


EASY_CVAR_EXTERN_DEBUGONLY(hassaultSpinMovement)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultFriendlyFire)
EASY_CVAR_EXTERN_DEBUGONLY(drawDebugPathfinding2)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultIdleSpinSound)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultFireSound)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultIdleSpinSoundChannel)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultFireSoundChannel)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultSpinUpDownSoundChannel)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultWaitTime)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultSpinupRemainTime)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultResidualAttackTime)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultSpinupStartTime)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultVoicePitchMin)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultVoicePitchMax)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultFireSpread)
EASY_CVAR_EXTERN_DEBUGONLY(noFlinchOnHard)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultDrawLKP)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultPrintout)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultExtraMuzzleFlashRadius)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultExtraMuzzleFlashForward)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultBulletDamageMulti)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultBulletsPerShot)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultFireAnimSpeedMulti)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultMeleeAnimSpeedMulti)
EASY_CVAR_EXTERN_DEBUGONLY(hassaultMeleeAttackEnabled)
EASY_CVAR_EXTERN_CLIENTSENDOFF_BROADCAST_DEBUGONLY(sv_germancensorship)
extern BOOL globalPSEUDO_germanModel_hassaultFound;


// oh wait nothing uses this.  oops.
#define HASSAULT_SENTENCE_VOLUME (float)0.56


#if REMOVE_ORIGINAL_NAMES != 1
	//BLUE24 here.  Just as I found, leaving here.
	//NOTICE - it looks like other monsters use full names like "monster_human_grunt" in retail for spawning.
	//         In the case of CHassault shouldn't this be, "monster_human_assault" for the default  name to match?
	//         CHANGE MADE.
	LINK_ENTITY_TO_CLASS( monster_human_assault, CHAssault );
	LINK_ENTITY_TO_CLASS( monster_astro, CHAssault );
#endif

#if EXTRA_NAMES > 0
	LINK_ENTITY_TO_CLASS( human_assault, CHAssault );
	LINK_ENTITY_TO_CLASS( astro, CHAssault );

	#if EXTRA_NAMES == 2
		LINK_ENTITY_TO_CLASS( monster_hassault, CHAssault );
		LINK_ENTITY_TO_CLASS( hassault, CHAssault );
	#endif
#endif




//Try to use these to make some forced sequence work easier to understand?
enum hassault_sequence{  //key: frames, FPS
	SEQ_HASSAULT_IDLE1,
	SEQ_HASSAULT_IDLE2,
	SEQ_HASSAULT_CREEPING_WALK,
	SEQ_HASSAULT_TURN_LEFT,
	SEQ_HASSAULT_TURN_RIGHT,
	SEQ_HASSAULT_ATTACK,
	SEQ_HASSAULT_SMALL_PAIN1,
	SEQ_HASSAULT_SMALL_PAIN2,
	SEQ_HASSAULT_DIE_BACKWARDS,
	SEQ_HASSAULT_DIE_CRUMPLE,
	SEQ_HASSAULT_DIE_VIOLENT,
	SEQ_HASSAULT_HEAVY_AMBUSH

};





//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_HASSAULT_SPIN = LAST_COMMON_SCHEDULE + 1,
	SCHED_HASSAULT_FOUND_ENEMY,
	SCHED_HASSAULT_SUPPRESS,
	SCHED_HASSAULT_FIRE,
	SCHED_HASSAULT_FIRE_OVER,
	SCHED_HASSAULT_GENERIC_FAIL,
	SCHED_HASSAULT_RESIDUAL_FIRE,
	SCHED_HASSAULT_FORCEFIRE,
	SCHED_HASSAULT_MELEE1,

};

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_HASSAULT_SPIN = LAST_COMMON_TASK + 1,
	TASK_HASSAULT_SPINDOWN,
	TASK_HASSAULT_FIRE,
	TASK_HASSAULT_CHECK_FIRE,
	TASK_HASSAULT_RESIDUAL_FIRE,
	TASK_HASSAULT_FACE_FORCEFIRE,
	TASK_HASSAULT_FORCEFIRE,
	TASK_HASSAULT_START_SPIN,
	TASK_HASSAULT_WAIT_FOR_SPIN_FINISH

};


Task_t	tlHAssaultFireOver[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_SCHEDULE,		(float)SCHED_HASSAULT_RESIDUAL_FIRE	},
	/*
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)0.2		},
	{ TASK_WAIT_PVS,			(float)0		},
	*/
};

Schedule_t	slHAssaultFireOver[] =
{
	{
		tlHAssaultFireOver,
		ARRAYSIZE ( tlHAssaultFireOver ),
		0,
		0,
		"HAssault_FireOverFail"
	},
};

Task_t	tlHAssaultGenericFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)0.2		},
	{ TASK_WAIT_PVS,			(float)0		},
	// is that a good idea?
	{ TASK_SET_SCHEDULE,			(float)SCHED_TAKE_COVER_FROM_ORIGIN },
};

Schedule_t	slHAssaultGenericFail[] =
{
	{
		tlHAssaultGenericFail,
		ARRAYSIZE ( tlHAssaultGenericFail ),
		bits_COND_CAN_ATTACK,
		0,
		"HAssault_genFail"
	},
};



Task_t	tlHAssault_spin[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_HASSAULT_FIRE_OVER},
	{ TASK_STOP_MOVING,			(float)0		},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_HASSAULT_SPIN,					(float)1.5f},
	//{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_ENEMY,			(float)0		},
	//not yet dummkopf!
	//{ TASK_RANGE_ATTACK1,				(float)0		},
	
	//{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_IDLE_ANGRY  },

	/*
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	
	*/
	{ TASK_SET_SCHEDULE,			(float)SCHED_HASSAULT_FIRE	},
};


//GruntRangeAttack1B

Schedule_t	slHAssault_spin[] =
{
	{ 
		tlHAssault_spin,
		ARRAYSIZE ( tlHAssault_spin ), 

		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		//bits_COND_HEAR_SOUND		|
		//bits_COND_GRUNT_NOFIRE		|
		bits_COND_SPECIAL1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_NO_AMMO_LOADED,
		
		//FOR CRYIN OUT LOUD DONT INTERRUPT THE SPIN.  or maybe it's ok?
		//But at least find a way not to interrupt the startup spin anim perhaps if so?
		//bits_SOUND_DANGER | bits_SOUND_PLAYER | bits_SOUND_WORLD,
		0,

		"tlHAssault_spin"
	},
};


Task_t	tlHAssault_foundEnemy[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_HASSAULT_START_SPIN,				(float)0					},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_SIGNAL1			},
	{ TASK_HASSAULT_WAIT_FOR_SPIN_FINISH,		(float)0				},
	{ TASK_SET_SCHEDULE,			(float)SCHED_HASSAULT_FIRE	},
};

Schedule_t	slHAssault_foundEnemy[] =
{
	{ 
		tlHAssault_foundEnemy,
		ARRAYSIZE ( tlHAssault_foundEnemy ), 
		
		0,

		0,

		"tlHAssault_FoundEn"
	},
};


Task_t	tlHAssault_suppress[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_SOUND_WAKE,				(float)0					},   //is that okaY?
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_HASSAULT_START_SPIN,				(float)0					},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_SIGNAL2			},
	{ TASK_HASSAULT_WAIT_FOR_SPIN_FINISH,		(float)0				},
	{ TASK_SET_SCHEDULE,			(float)SCHED_HASSAULT_FIRE	},
};

Schedule_t	slHAssault_suppress[] =
{
	{ 
		tlHAssault_suppress,
		ARRAYSIZE ( tlHAssault_suppress ), 
		
		0,

		0,

		"tlHAssault_Suppress"
	},
};





/*
// Chase enemy schedule
Task_t tlHAssault_follow[] = 
{
	//{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_CHASE_ENEMY_FAILED	},
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_HASSAULT_GENERIC_FAIL	},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	{ TASK_RUN_PATH,			(float)0		},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
	{ TASK_FACE_ENEMY,	(float)0		},
};
*/

Task_t tlHAssault_follow[] = 
{
	//NO
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_HASSAULT_GENERIC_FAIL	},
	
	//{ TASK_GET_PATH_TO_ENEMY,	(float)0					},
	//{ TASK_RUN_PATH,			(float)0					},
	//{ TASK_WAIT_FOR_MOVEMENT,	(float)0					},
	{TASK_MOVE_TO_ENEMY_RANGE, (float)0					},
	{ TASK_CHECK_STUMPED,(float)0			},
};



//NOTICE: Based off of "slChaseEnemy" from defaultai.cpp.  Customize as needed.  Ditto for above.
Schedule_t slHAssault_follow[] =
{
	{ 
		tlHAssault_follow,
		ARRAYSIZE ( tlHAssault_follow ),

		/*
		bits_COND_NEW_ENEMY			|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_TASK_FAILED		|

		bits_COND_SEE_ENEMY |  //MODDD - ok?
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |

		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER | bits_SOUND_PLAYER,
		*/

		bits_COND_NEW_ENEMY			|
		//MODDD - added, the bullsquid counts this.  Why doesn't everything?
		bits_COND_ENEMY_DEAD |

		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_TASK_FAILED		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER | bits_SOUND_PLAYER | bits_SOUND_WORLD,
		"HA Chase Enemy"
	},
};



Task_t	tlHAssault_fire[] =
{
	
	//{ TASK_FACE_ENEMY,			(float)0		},
	//{ TASK_HASSAULT_SPIN,					(float)1.5f},


	{ TASK_STOP_MOVING, (float)0},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_HASSAULT_FIRE_OVER},
	
	//these are redundant now.
	//{ TASK_HASSAULT_CHECK_FIRE, (float)0	},
	//{ TASK_FACE_ENEMY,			(float)0		},
	
	
	//set activity to ACT_RANGE_ATTACK1 ?
	//{TASK_SET_ACTIVITY,  (float)ACT_RANGE_ATTACK1},
	
	//MODDD - this will now last as long as the hgrunt has a clear firing view of the enemy. It automatically loops,
	//        and relying on the looping behavior instead of going through this schedule again when it's done saves
	//        a frame that would otherwise be wasted.
	{ TASK_RANGE_ATTACK1,				(float)0		},
	
	//{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_IDLE_ANGRY  },

	/*
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	
	*/
	//{ TASK_SET_SCHEDULE,			(float)SCHED_HASSAULT_FIRE	},
};


Schedule_t	slHAssault_fire[] =
{
	{ 
		tlHAssault_fire,
		ARRAYSIZE ( tlHAssault_fire ), 

		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		//bits_COND_HEAR_SOUND		|

		//bits_COND_GRUNT_NOFIRE		|
		bits_COND_SPECIAL1	| 
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_NO_AMMO_LOADED,
		

		//bits_SOUND_DANGER,
		0,
		"slHAssault_fire"
	},
};



Task_t	tlHAssault_residualFire[] =
{
	//{ TASK_FACE_ENEMY,			(float)0		},
	//{ TASK_HASSAULT_SPIN,					(float)1.5f},

	
	{ TASK_STOP_MOVING, (float)0},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_HASSAULT_FIRE_OVER},
	{ TASK_HASSAULT_CHECK_FIRE, (float)0	},

	//This probably isn't the same as "LKM", it's the "last place this monster was before conflict".
	//No issues, so... no complaints I guess.
	
	//hm... best not do this.
	//{ TASK_FACE_LASTPOSITION,			(float)0		},


	//MODDD TODO - make this TASK_RANGE_ATTACK1, the endless looping task later.
	//{ TASK_RANGE_ATTACK1,				(float)0		},


	//set activity to ACT_RANGE_ATTACK1 ?
	{TASK_SET_ACTIVITY,  (float)ACT_RANGE_ATTACK1},
	
	{ TASK_HASSAULT_RESIDUAL_FIRE,				(float)0		},
	
	//{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_IDLE_ANGRY  },

	/*
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	
	*/
	//{ TASK_SET_SCHEDULE,			(float)SCHED_HASSAULT_FIRE	},
};


Schedule_t	slHAssault_residualFire[] =
{
	{ 
		tlHAssault_residualFire,
		ARRAYSIZE ( tlHAssault_residualFire ), 

		bits_COND_NEW_ENEMY			|

		//MODDD - Surpsrisingly no. Just because we can see an enemy behind cover doesn't mean we can hit them, this tricks the system.
		//////bits_COND_SEE_ENEMY			|
		//But being able to do a ranged attack (clear shot) means we can stop using residual fire.
		bits_COND_CAN_RANGE_ATTACK1 |

		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND		|
		//bits_COND_GRUNT_NOFIRE		|
		bits_COND_SPECIAL1	| 
		//MODDD - yes.
		bits_COND_CAN_MELEE_ATTACK1 |

		bits_COND_NO_AMMO_LOADED,
		//bits_COND_SEE_ENEMY,  //NEW!  ... no, we must be more specific.

		
		bits_SOUND_DANGER,

		"slHAssault_resfire"
	},
};


// MODDD TODO - Make the hgrunt check to see if the player is blocked by something, even while spinning up, and remember that object to then begin
// firing at it when done spinning up.
Task_t	tlHAssault_forceFireAtTarget[] =
{
	
	//{ TASK_FACE_ENEMY,			(float)0		},
	//{ TASK_HASSAULT_SPIN,					(float)1.5f},


	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_HASSAULT_FIRE_OVER},
	{ TASK_HASSAULT_CHECK_FIRE, (float)0	},

	//This probably isn't the same as "LKM", it's the "last place this monster was before conflict".
	//No issues, so... no complaints I guess.
	{ TASK_HASSAULT_FACE_FORCEFIRE,			(float)0		},


	//set activity to ACT_RANGE_ATTACK1 ?
	{TASK_SET_ACTIVITY,  (float)ACT_RANGE_ATTACK1},
	
	{ TASK_HASSAULT_FORCEFIRE,				(float)0		},
	
	//{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_IDLE_ANGRY  },

	/*
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_STOP_MOVING,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	
	*/
	//{ TASK_SET_SCHEDULE,			(float)SCHED_HASSAULT_FIRE	},
	{ TASK_SET_SCHEDULE,			(float)SCHED_HASSAULT_FORCEFIRE	},
};


Schedule_t	slHAssault_forceFireAtTarget[] =
{
	{ 
		tlHAssault_forceFireAtTarget,
		ARRAYSIZE ( tlHAssault_forceFireAtTarget ), 

		//This little addition here is ok, right?
		bits_COND_CAN_RANGE_ATTACK1 |

		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		//bits_COND_ENEMY_OCCLUDED	|
		//bits_COND_HEAR_SOUND		|
		//bits_COND_GRUNT_NOFIRE		|
		bits_COND_SPECIAL1	| 
		bits_COND_NO_AMMO_LOADED,
		//bits_COND_SEE_ENEMY,  //NEW!  ... no, we must be more specific.

		
		//bits_SOUND_DANGER,
		0,
		"slHAssault_forceFi"
	},
};



// primary melee attack
Task_t	tlHAssault_melee1[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_HASSAULT_GENERIC_FAIL},
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK1,		(float)0		},
};

Schedule_t	slHAssault_melee1[] =
{
	{ 
		tlHAssault_melee1,
		ARRAYSIZE ( tlHAssault_melee1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		//MODDD - also, WHY NOT?
		bits_COND_TASK_FAILED |
		//bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_ENEMY_OCCLUDED,
		0,
		"hassault MeleeAttack1"
	},
};



DEFINE_CUSTOM_SCHEDULES( CHAssault )
{
	slHAssault_spin,
	slHAssault_foundEnemy,
	slHAssault_suppress,
	slHAssault_fire,
	slHAssault_follow,
	slHAssaultGenericFail,
	slHAssaultFireOver,
	slHAssault_residualFire,
	slHAssault_forceFireAtTarget,
	slHAssault_melee1

};

IMPLEMENT_CUSTOM_SCHEDULES( CHAssault, CSquadMonster );



const char* CHAssault::pIdleSounds[] =
{
	"hgrunt/gr_radio1.wav",
	"hgrunt/gr_radio2.wav",
	"hgrunt/gr_radio3.wav",
	"hgrunt/gr_radio4.wav",
	"hgrunt/gr_radio5.wav",
	"hgrunt/gr_radio6.wav",
};

const char *CHAssault::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CHAssault::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char *CHAssault::pAttackSounds[] = 
{
	"zombie/zo_attack1.wav",
	"zombie/zo_attack2.wav",
};

//"cannot allocate an array of constant size 0" my ass.
/*
const char *CHAssault::pIdleSounds[] = 
{
	//Play a sentence instead. HGrunt does that with gr_idle's 1-3 through sentences HG_IDLE 0-2.
	//"hgrunt/gr_radio1.wav",
	//"hgrunt/gr_radio2.wav",
	//"hgrunt/gr_radio3.wav",
	//"hgrunt/gr_radio4.wav",
	//"hgrunt/gr_radio5.wav",
	//"hgrunt/gr_radio6.wav",
	"hgrunt/gr_idle1.wav",
	"hgrunt/gr_idle2.wav",
	"hgrunt/gr_idle3.wav",
};
*/


/*
const char *CHAssault::pAlertSounds[] = 
{
	"hassault/hw_alert.wav",
};
*/

const char *CHAssault::pPainSounds[] = 
{
	"hgrunt/gr_pain1.wav",
	"hgrunt/gr_pain2.wav",
	"hgrunt/gr_pain3.wav",
	"hgrunt/gr_pain4.wav",
	"hgrunt/gr_pain5.wav",
};



TYPEDESCRIPTION	CHAssault::m_SaveData[] = 
{
	DEFINE_FIELD( CBaseEntity,m_pfnThink , FIELD_FUNCTION),
	DEFINE_FIELD( CHAssault, m_voicePitch, FIELD_INTEGER),
	DEFINE_FIELD( CHAssault, spinuptime, FIELD_TIME),
	DEFINE_FIELD( CHAssault, spinuptimeremain, FIELD_TIME),
	
};
//IMPLEMENT_SAVERESTORE( CHAssault, CSquadMonster );    //parent was "CBaseMonster", assuming CSquadMonster is okay though?
int CHAssault::Save( CSave &save )
{
	if ( !CSquadMonster::Save(save) )
		return 0;
	return save.WriteFields( "CHAssault", this, m_SaveData, ARRAYSIZE(m_SaveData) );
}
int CHAssault::Restore( CRestore &restore )
{
	if ( !CSquadMonster::Restore(restore) )
		return 0;
	return restore.ReadFields( "CHAssault", this, m_SaveData, ARRAYSIZE(m_SaveData) );
}


float CHAssault::SafeSetBlending ( int iBlender, float flValue ){

	if(pev->sequence == SEQ_HASSAULT_ATTACK){
		//startup spin? reverse the angle.
		flValue *= -1;
	}


	return CBaseAnimating::SetBlending(iBlender, flValue);
}//END OF SafeSetBlending


//Some commonly used script for aiminat at the enemy consistently (no jitter).
//Also doesn't aim ridiculously high if the enemy gets close like it usually would.
void CHAssault::AimAtEnemy(Vector& refVecShootOrigin, Vector& refVecShootDir ){
	//DebugLine_ClearAll();
	
	//if there is no enemy or this is residual fire, just fire straight.
	if(m_hEnemy == NULL || m_pSchedule == slHAssault_residualFire){
		//no enemy? Just aim straight across.
		refVecShootOrigin = GetGunPosition();

		Vector vecto;
		::UTIL_MakeVectorsPrivate(pev->angles, vecto, NULL, NULL);
		Vector vectoFlat = Vector(vecto.x, vecto.y, 0).Normalize();

		refVecShootDir = vectoFlat;
		SafeSetBlending( 0, 0 );
		return;
	}else{
		//Vector vecShootOrigin = GetGunPositionAI();
		//good distance? Normal aiming.
		//Vector vecShootDirMod = ShootAtEnemyMod( argVecShootOrigin);
		Vector vecShootOriginAI = GetGunPositionAI();
		Vector vecShootDirAI = ShootAtEnemyMod( vecShootOriginAI );  //less jitter.
		//MODDD - just like hgrunt does it.

		//easyForcePrintLine("IS THE eee %.2f", (m_hEnemy->pev->origin - this->pev->origin).Length());

		if((m_hEnemy->pev->origin - this->pev->origin).Length() < 150){
			//See if a straight line ground-wise hits them. Then try the usual aiming if not.
			TraceResult tr;

			//2D direction:
			//Vector vecShootDirMod2D = ( (m_hEnemy->BodyTargetMod( vecShootOrigin ) - m_hEnemy->pev->origin) + m_vecEnemyLKP - shootOrigin );
		
			Vector enemyLoc = m_hEnemy->BodyTargetMod(vecShootOriginAI);
			//this is the enemy position at the end of a floor-wise line from me to them.
			Vector enemyLocLinedup = Vector(enemyLoc.x, enemyLoc.y, vecShootOriginAI.z);

			UTIL_TraceLine( vecShootOriginAI, enemyLocLinedup, dont_ignore_monsters, ignore_glass, ENT(pev), &tr);

			//::DebugLine_Setup(0, vecShootOriginAI, enemyLocLinedup, tr.flFraction);

			if(tr.pHit != NULL && tr.pHit == m_hEnemy.Get()){
				//success - aim straight.

				//refVecShootDir = Vector(vecShootDirMod.x, vecShootDirMod.y, 0).Normalize();

				//only the pitch changes, so reuse the same angle, doesn't not differ groundwise.
				//Vector angDir2D = Vector(angDirAI.x, angDirAI.y, 0).Normalize();
				//SafeSetBlending( 0, angDir2D.x );
				
				//wait... just aim straight, sheesh.
				SafeSetBlending(0, 0);

				refVecShootOrigin = GetGunPosition(); //no change needed.
				Vector vecShootDirRaw = ShootAtEnemy(refVecShootOrigin);
				refVecShootDir = Vector(vecShootDirRaw.x, vecShootDirRaw.y, 0).Normalize();  //direction ground-wise.

				return; //don't do the default below.
			}else{
				//DebugLine_ColorFail(0);
				//fail? Fall thru for the default.
			}

		}else{
			//No impact. fall thru for the default.
		}

		Vector angDirAI = UTIL_VecToAngles( vecShootDirAI );

		refVecShootOrigin = GetGunPosition();
		refVecShootDir = ShootAtEnemy(refVecShootOrigin);
		
		SafeSetBlending( 0, angDirAI.x );
		//SafeSetBlending( 0, 30 );
		
	}


}//END OF AimAtEnemy


int CHAssault::IRelationship ( CBaseEntity *pTarget )
{
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1){
		//I just don't give a damn man
		return R_NO;
	}
	
	//MODDD - well the hgrunt has these priorties.
	if ( FClassnameIs( pTarget->pev, "monster_alien_grunt" ) || ( FClassnameIs( pTarget->pev,  "monster_gargantua" ) ) )
	{
		return R_NM;
	}

	return CSquadMonster::IRelationship(pTarget);
}


//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CHAssault::Classify ( void )
{
	return	CLASS_HUMAN_MILITARY;
}

BOOL CHAssault::getGermanModelRequirement(void){
	return globalPSEUDO_germanModel_hassaultFound;
}
const char* CHAssault::getGermanModel(void){
	return "models/g_hassault.mdl";
}
const char* CHAssault::getNormalModel(void){
	return "models/hassault.mdl";
}


//copied from hgrunt.
//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CHAssault::PrescheduleThink ( void )
{
	if ( InSquad() && m_hEnemy != NULL )
	{
		if ( HasConditions ( bits_COND_SEE_ENEMY ) )
		{
			// update the squad's last enemy sighting time.
			MySquadLeader()->m_flLastEnemySightTime = gpGlobals->time;
		}
		else
		{
			if ( gpGlobals->time - MySquadLeader()->m_flLastEnemySightTime > 5 )
			{
				// been a while since we've seen the enemy
				MySquadLeader()->m_fEnemyEluded = TRUE;
			}
		}
	}
}//END OF PrescheduleThink


//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CHAssault::SetYawSpeed ( void )
{
	int ys;

	ys = 120;

	switch ( m_Activity )
	{
		case ACT_RANGE_ATTACK1:
			MakeIdealYaw ( m_vecEnemyLKP );
			ys = 150;
			break;
	}

	pev->yaw_speed = ys;
}


GENERATE_TRACEATTACK_IMPLEMENTATION(CHAssault)
{
	GENERATE_TRACEATTACK_PARENT_CALL(CSquadMonster);
}

GENERATE_TAKEDAMAGE_IMPLEMENTATION(CHAssault)
{
	//Anytime I take damage, if spun up, it resets the spinup timer. Not unspinning now.

	if(spinuptimeremain != -1 && gpGlobals->time < spinuptimeremain){
		spinuptimeremain = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultSpinupRemainTime);
	}

	//taking damage makes the hassault walk faster while following.
	rageTimer = gpGlobals->time + 8;
	waittime = -1;  //if waiting, sure aren't now.

	// HACK HACK -- until we fix this.
	//if ( IsAlive() )
	//MODDD - only play pain sounds if completely alive (not in the dying animation either)
	
	//return 1;
	int eck = GENERATE_TAKEDAMAGE_PARENT_CALL(CSquadMonster);

	if (HasConditions(bits_COND_HEAVY_DAMAGE)) {
		int pitch = randomValueInt(m_voicePitch - 3, m_voicePitch + 5);
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "hgrunt/gr_cover2.wav", 1.0, ATTN_NORM - 0.5, 0, pitch);
	}

	return eck;
}

void CHAssault::PainSound( void )
{
	//int pitch = 95 + RANDOM_LONG(0,9);
	int pitch = randomValueInt(m_voicePitch - 4, m_voicePitch + 4);

	if (RANDOM_LONG(0,5) < 2)
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, pPainSounds[ RANDOM_LONG(0,ARRAYSIZE(pPainSounds)-1) ], 1.0, ATTN_NORM, 0, pitch );
}

void CHAssault::AlertSound( void )
{
	if(alertSoundCooldown != -1 && gpGlobals->time < alertSoundCooldown){
		return; //not so soon.
	}

	alertSoundCooldown = gpGlobals->time + 10;

	//int pitch = 95 + RANDOM_LONG(0,9);
	int pitch = randomValueInt(m_voicePitch - 0, m_voicePitch + 7);

	if (InSquad()) {
		if (RANDOM_FLOAT(0, 1) <= 0.6) {
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "hgrunt/gr_squadform.wav", 1.0, ATTN_NORM - 0.23, 0, pitch);
		}
		else {
			//same as usual
			SENTENCEG_PlayRndSz(ENT(pev), "HG_ALERT", 1, ATTN_NORM - 0.23, 0, pitch);
			//UTIL_PlaySound(ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_LONG(0, ARRAYSIZE(pAlertSounds) - 1)], 1.0, ATTN_NORM - 0.2, 0, pitch);
		}
	}
	else {
		SENTENCEG_PlayRndSz(ENT(pev), "HG_ALERT", 1, ATTN_NORM - 0.23, 0, pitch);
		//UTIL_PlaySound( ENT(pev), CHAN_VOICE, pAlertSounds[ RANDOM_LONG(0,ARRAYSIZE(pAlertSounds)-1) ], 1.0, ATTN_NORM-0.2, 0, pitch );
	}
}

void CHAssault::IdleSound( void )
{
	//int pitch = 95 + RANDOM_LONG(0,9);
	int pitch = randomValueInt(m_voicePitch - 7, m_voicePitch + 7);

	// Play a random idle sound
	//...if we add different direct sounds, can randomly choose between this and the HG_IDLE sentence.
	//UTIL_PlaySound( ENT(pev), CHAN_VOICE, pIdleSounds[ RANDOM_LONG(0,ARRAYSIZE(pIdleSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );

	//mODDD - this instead??  depend on being in a squad? I don't know.
	//SENTENCEG_PlayRndSz(ENT(pev), "HG_IDLE", HASSAULT_SENTENCE_VOLUME, ATTN_NORM, 0, pitch);
	
	// also less attenuation, go further.
	UTIL_PlaySound(ENT(pev), CHAN_WEAPON, pIdleSounds[RANDOM_LONG(0, ARRAYSIZE(pIdleSounds) - 1)], 1.0, ATTN_NORM - 0.21, 0, 100 + RANDOM_LONG(-8, 8));

}


void CHAssault::DeathSound ( void )
{
	int pitch = randomValueInt(m_voicePitch - 4, m_voicePitch + 3);

	switch ( RANDOM_LONG(0,2) )
	{
	case 0:	
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "hgrunt/gr_die1.wav", 1, ATTN_IDLE, 0, pitch );
		break;
	case 1:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "hgrunt/gr_die2.wav", 1, ATTN_IDLE, 0, pitch );
		break;
	case 2:
		UTIL_PlaySound( ENT(pev), CHAN_VOICE, "hgrunt/gr_die3.wav", 1, ATTN_IDLE, 0, pitch );
		break;
	}
}


//well, melee, currently unused.
void CHAssault::AttackSound( void )
{
	// Play a random attack sound
	UTIL_PlaySound( ENT(pev), CHAN_VOICE, pAttackSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
}


void CHAssault::HandleEventQueueEvent(int arg_eventID){

	switch(arg_eventID){
	case 0:
		{		
		CBaseEntity *pHurt = HumanKick(77);

		if ( pHurt )
		{
			//MODDD - TODO:  make this draw blood on the victim...
			UTIL_MakeVectors( pev->angles );

			if(!pHurt->blocksImpact()){
				//for now..
				pHurt->pev->punchangle.x = 15;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 47 + gpGlobals->v_up * 28;
			}

			pHurt->TakeDamage( pev, pev, gSkillData.hassaultDmgMelee*0.6, DMG_CLUB );

			//UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
							
		}else{
			playStandardMeleeAttackMissSound();
		}
	break;
		}
	case 1:
		{
		CBaseEntity *pHurt = HumanKick(82);

		if ( pHurt )
		{

			//MODDD - TODO:  make this draw blood on the victim...
			UTIL_MakeVectors( pev->angles );
			
			if(!pHurt->blocksImpact()){
				//for now..
				pHurt->pev->punchangle.x = 15;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 225 + gpGlobals->v_up * 88;
			}

			pHurt->TakeDamage( pev, pev, gSkillData.hassaultDmgMelee, DMG_CLUB );
			UTIL_PlaySound( ENT(pev), CHAN_WEAPON, pAttackHitSounds[ RANDOM_LONG(0,ARRAYSIZE(pAttackHitSounds)-1) ], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5) );
							
		}else{
			playStandardMeleeAttackMissSound();
		}
	break;
		}
	case 2:
		//soft thud

		//EVENT REMOVED, model already has them working fine.

	break;
	case 3:
		//harder thud
						
		//EVENT REMOVED, model already has them working fine.

	break;

	}//END OF SWITCH (animQueueEventID check)

}
//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CHAssault::HandleAnimEvent( MonsterEvent_t *pEvent )
{

	EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "OOHHHH ANIM EVENT : %d", pEvent->event) );
	switch( pEvent->event )
	{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			{
				//pev->framerate = 2;

				if(fireDelay != -1 && gpGlobals->time < fireDelay){
					pev->renderfx |= NOMUZZLEFLASH;
					return;  //not yet!
				}

				//restore if missing.
				pev->renderfx &= ~NOMUZZLEFLASH;


				pev->framerate = EASY_CVAR_GET_DEBUGONLY(hassaultFireAnimSpeedMulti);

				if(m_pSchedule != slHAssault_forceFireAtTarget){
					Shoot();
				}else{
					//shoots at target instead.
					ShootAtForceFireTarget();
				}
				//return;
				break;
			}
		default:
			CSquadMonster::HandleAnimEvent( pEvent );
		break;
	}
	
	
	//is that okay (to be commented out)?
	//CBaseMonster::HandleAnimEvent( pEvent );
}


CHAssault::CHAssault(){
	signal1Cooldown = -1;
	signal2Cooldown = -1;
	
	nonStumpableCombatLook = FALSE;
	previousAnimationActivity = -1;
	fireDelay = -1;


	forceBlockResidual = FALSE;
	meleeAttackTimeMax = -1;
	spinuptime = -1;
	spinuptimeremain = -1;
	waittime = -1;

	rageTimer = -1;

	movementBaseFramerate = 1.0f;

	firing = FALSE;
	recentSchedule = NULL;
	recentRecentSchedule = NULL;

	residualFireTime = -1;
	idleSpinSoundDelay = -1;
	chainFireSoundDelay = -1;
	chainFiredRecently = FALSE;
	spinuptimeIdleSoundDelay = -1;
	residualFireTimeBehindCheck = -1;

	alertSoundCooldown = -1;
}


float CHAssault::getBarnacleForwardOffset(void){
	return 1.2f;
}
float CHAssault::getBarnaclePulledTopOffset(void){
	return -9.7;
}


//=========================================================
// Spawn
//=========================================================
void CHAssault::Spawn()
{
	Precache( );

	//SET_MODEL(ENT(pev), "models/hassault.mdl");
	setModel(); //german model check?

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	//pev->classname = MAKE_STRING("monster_hassault");
	pev->classname = MAKE_STRING("monster_human_assault");

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->health			= gSkillData.hassaultHealth;


	//MonsterInit() will call SetEyePosition and get this from the model anyways, so...?
	pev->view_ofs		= VEC_VIEW;// position of the eyes relative to monster's origin.

	
	//m_flFieldOfView		= VIEW_FIELD_FULL;//0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_flFieldOfView = 0.24;
	//m_flFieldOfView		= EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(testVar);

	//MODDD - grunt seems to lack this line in spawn (or anywhere), disabling?
	//nah, let's try with this on again.
	//m_MonsterState		= MONSTERSTATE_COMBAT;
	//Off now.

	
	//m_afCapability		= bits_CAP_DOORS_GROUP;
	//m_afCapability		= bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;
	//MODDD - can join the squad with "hssault".
	m_afCapability		= bits_CAP_SQUAD | bits_CAP_HEAR | bits_CAP_DOORS_GROUP;
	
	// get voice pitch at the start.
	m_voicePitch = randomValueInt((int)EASY_CVAR_GET_DEBUGONLY(hassaultVoicePitchMin), (int)EASY_CVAR_GET_DEBUGONLY(hassaultVoicePitchMax));
	

	//| bits_CAP_RANGE_ATTACK1;
	//???

	//spinuptime = 0; ???

	//spinuptime = 999999; //stupidly large, defaults to not being possible until set manually.
	//nah, just rely on this new var below:
	spinuptimeSet = FALSE;


	//offset that's forced on the origin for getting the start point for generating bullets.
	//Should probably be starting a bit outwards in the direction this monster is facing, but other monsters do this so I suppose it is ok.
	//m_HackedGunPos = Vector(0, 0, 48);
	//More accurate version to an observed average position while the gun is firing.
	m_HackedGunPos = Vector(9.594678, 41.80166, 42.344753);


	//keep THESE in mind for your monster.
	m_flDistTooFar		= 1024.0;
	m_flDistLook		= 2048.0;

	MonsterInit();
//	SetThink(HAssaultThink);

}

extern int global_useSentenceSave;
//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CHAssault::Precache()
{
	int i;

	//IMPORTANT!!!      On adding new sounds, make SURE to add to hgrunt's own "precache" method at the bottom too.
	//HGrunts must precache the HAssault's assets in case an HGrunt is "promoted" to an HAssault.


	PRECACHE_MODEL("models/hassault.mdl");
	m_iBrassShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell

	global_useSentenceSave = TRUE;
	
	PRECACHE_SOUND("hassault/hw_spin.wav");	
	PRECACHE_SOUND("hassault/hw_spinup.wav");	
	PRECACHE_SOUND("hassault/hw_spindown.wav");	
	PRECACHE_SOUND("hassault/hw_shoot1.wav");	
	PRECACHE_SOUND("hassault/hw_shoot2.wav");
	PRECACHE_SOUND("hassault/hw_shoot3.wav");
	PRECACHE_SOUND("hassault/hw_gun4.wav");

	PRECACHE_SOUND("hgrunt/gr_die1.wav");
	PRECACHE_SOUND("hgrunt/gr_die2.wav");
	PRECACHE_SOUND("hgrunt/gr_die3.wav");


	PRECACHE_SOUND("hgrunt/gr_squadform.wav");
	PRECACHE_SOUND("hgrunt/gr_cover2.wav");
	

	// already covered in "pAttackMissSounds".  Check it.
	//PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event
	// zombie strike's precached by the hgrunt already.
	//precacheStandardMeleeAttackMissSounds();

	PRECACHE_SOUND_ARRAY(pIdleSounds)
	PRECACHE_SOUND_ARRAY(pAttackHitSounds)
	PRECACHE_SOUND_ARRAY(pAttackMissSounds)
	PRECACHE_SOUND_ARRAY(pAttackSounds)
	//PRECACHE_SOUND_ARRAY(pAlertSounds)
	PRECACHE_SOUND_ARRAY(pPainSounds)

	global_useSentenceSave = FALSE;

}//END OF precache

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

/*
int CHAssault::IgnoreConditions ( void )
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if ((m_Activity == ACT_MELEE_ATTACK1) || (m_Activity == ACT_MELEE_ATTACK1))
	{
#if 0
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE);
		else
#endif			
		if (m_flNextFlinch >= gpGlobals->time)
			iIgnore |= (bits_COND_LIGHT_DAMAGE|bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + ZOMBIE_FLINCH_DELAY;
	}

	return iIgnore;
	
}*/


Vector CHAssault::GetGunPosition(void){
	//maybe this would be better?
	Vector vecGunPos;
	Vector vecGunAngles;
	GetAttachment( 0, vecGunPos, vecGunAngles );
	//::UTIL_printLineVector("yehhh", vecGunPos-pev->origin);

	return vecGunPos;
}//END OF GetGunPosition

Vector CHAssault::GetGunPositionAI(void){
	Vector v_forward, v_right, v_up, angle;


	//TEST: if I WERE facing the enemy right now...
	if(m_hEnemy != NULL){
		angle = ::UTIL_VecToAngles(m_hEnemy->pev->origin - pev->origin);
	}else{
		angle = pev->angles;
	}
	
	angle.x = 0; //pitch is not a factor here.
	UTIL_MakeVectorsPrivate( angle, v_forward, v_right, v_up);
	
	const Vector vecSrc = pev->origin 
					+ v_forward * m_HackedGunPos.y 
					+ v_right * m_HackedGunPos.x 
					+ v_up * m_HackedGunPos.z;

	return vecSrc;
}//END OF GetGunPositionAI

//MODDD - ...hgrunt does this. Could we do this constant height and a gpGlobals->forward (after the right call) to go forward a little too?
//           this would avoid the jitter issue with barely getting behind cover.
//return pev->origin + Vector( 0, 0, 60 );


void CHAssault::Shoot ( void )
{
	//return;
	/*
	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemyMod( vecShootOrigin );
	Vector vecShootDirAI = ShootAtEnemyMod( GetGunPositionAI() );  //less jitter.
	*/

	Vector vecShootOrigin;
	Vector vecShootDir;

	AimAtEnemy(vecShootOrigin, vecShootDir);
	

	//residual fire will not try to focus on the enemy. The assumption is we don't know where they are at the moment.
	if (m_hEnemy != NULL && !(m_pSchedule == slHAssault_residualFire) )
	{
		MakeIdealYaw(m_hEnemy->pev->origin);
	}

	//UTIL_MakeAimVectors(pev->angles);

	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 
//	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
//	if (((pev->origin.z - m_vecEnemyLKP.z) > -50) && ((pev->origin.z - m_vecEnemyLKP.z) < 50))
//	{
//		FireBullets(1, vecShootOrigin, gpGlobals->v_forward, VECTOR_CONE_10DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
//	}
//	else
//	{
		//vecShootDir = gpGlobals->v_forward;
		//vecShootDir.z = ShootAtEnemyMod( vecShootOrigin ).z;
		//ALSO TRIED CONE_1DEGREES!
		//FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees

		if(EASY_CVAR_GET_DEBUGONLY(hassaultBulletsPerShot) >= 1){
			int tracerFreq;
			float filteredBulletsPerShot = EASY_CVAR_GET_DEBUGONLY(hassaultBulletsPerShot);
			if(filteredBulletsPerShot <= 0){
				//not allowed, enforce default.
				filteredBulletsPerShot = DEFAULT_hassaultBulletsPerShot;
			}
			
			tracerFreq = (int) (4 * (1 / filteredBulletsPerShot ));
			FireBullets((int)EASY_CVAR_GET_DEBUGONLY(hassaultBulletsPerShot), vecShootOrigin, vecShootDir, Vector(EASY_CVAR_GET_DEBUGONLY(hassaultFireSpread),EASY_CVAR_GET_DEBUGONLY(hassaultFireSpread),EASY_CVAR_GET_DEBUGONLY(hassaultFireSpread) ), 2048, BULLET_MONSTER_MP5, tracerFreq , EASY_CVAR_GET_DEBUGONLY(hassaultBulletDamageMulti) ); // shoot +-5 degrees
		}

//	}

		
	//if(RANDOM_FLOAT(0, 1) <= EASY_CVAR_GET(hassaultMuzzleFlashChance)){
		pev->effects |= EF_MUZZLEFLASH;
	//}
	
//	m_cAmmoLoaded--;// take away a bullet!

//	Vector angDir = UTIL_VecToAngles( vecShootDir );
//	SafeSetBlending( 0, angDir.x );
	
	if(EASY_CVAR_GET_DEBUGONLY(hassaultFireSound) == 1){
		switch(RANDOM_LONG(0,2))
		{
			case 0:
				UTIL_PlaySound( ENT(pev), getChainFireChannel(), "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, 100 );
			break;
			case 1:
				UTIL_PlaySound( ENT(pev), getChainFireChannel(), "hassault/hw_shoot2.wav", 1, ATTN_NORM, 0, 100 );
			break;
			case 2:
				UTIL_PlaySound( ENT(pev), getChainFireChannel(), "hassault/hw_shoot3.wav", 1, ATTN_NORM, 0, 100 );
			break;

			//just to know when the idle spin is allowed to be played, if it cannot play alongside fire sounds.
			chainFireSoundDelay = gpGlobals->time + 0.4;
			attemptStopIdleSpinSound();

		}
	}else if(EASY_CVAR_GET_DEBUGONLY(hassaultFireSound) == 2){
		if(chainFireSoundDelay == -1 || chainFireSoundDelay <= gpGlobals->time){
			//chainFireSoundDelay = gpGlobals->time + 2.385;
			chainFireSoundDelay = gpGlobals->time + 2.07;
			UTIL_StopSound(ENT(pev), getChainFireChannel(), "hassault/hw_gun4.wav");
			UTIL_PlaySound(ENT(pev), getChainFireChannel(), "hassault/hw_gun4.wav", 1, ATTN_NORM, 0, 100 );
			attemptStopIdleSpinSound();
		}
	}else if(EASY_CVAR_GET_DEBUGONLY(hassaultFireSound) == 3){
		UTIL_PlaySound( ENT(pev), getChainFireChannel(), "hassault/hw_gun4.wav", 1, ATTN_NORM, 0, 100 );
		attemptStopIdleSpinSound();
	}

	if(EASY_CVAR_GET_DEBUGONLY(hassaultExtraMuzzleFlashRadius)){
		Vector extraFlashOrigin = vecShootOrigin + vecShootDir * EASY_CVAR_GET_DEBUGONLY(hassaultExtraMuzzleFlashForward);
	
		float brightness = (EASY_CVAR_GET_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)*255);

		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, extraFlashOrigin );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(extraFlashOrigin.x);	// X
			WRITE_COORD(extraFlashOrigin.y);	// Y
			WRITE_COORD(extraFlashOrigin.z);	// Z
			WRITE_BYTE( (int)(EASY_CVAR_GET_DEBUGONLY(hassaultExtraMuzzleFlashRadius)*10) );		// radius * 0.1
			WRITE_BYTE( brightness );		// r
			WRITE_BYTE( brightness );		// g
			WRITE_BYTE( brightness );		// b
			WRITE_BYTE( 1 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );
	}

	//This will use the gun position for AI instead so that the pitch doesn't jitter around.
	//MODDD - just like hgrunt does it.
	//Vector angDir = UTIL_VecToAngles( vecShootDirAI );
	//SafeSetBlending( 0, angDir.x );
}

void CHAssault::ShootAtForceFireTarget ( void )
{

	//EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "is forceFireTargetObject free %d", forceFireTargetObject->free));

	//return;

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = forceFireTargetPosition - vecShootOrigin;

	Vector vecShootDirAI = forceFireTargetPosition - GetGunPositionAI(); //less jitter

	
	MakeIdealYaw(forceFireTargetPosition);
	

	UTIL_MakeAimVectors(pev->angles);


	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL); 

		////vecShootDir = gpGlobals->v_forward;
		////vecShootDir.z = ShootAtEnemyMod( vecShootOrigin ).z;
		
	if(EASY_CVAR_GET_DEBUGONLY(hassaultBulletsPerShot) >= 1){
		int tracerFreq;
		float filteredBulletsPerShot = EASY_CVAR_GET_DEBUGONLY(hassaultBulletsPerShot);
		if(filteredBulletsPerShot <= 0){
			//not allowed, enforce default.
			filteredBulletsPerShot = DEFAULT_hassaultBulletsPerShot;
		}
			
		tracerFreq = (int) (4 * (1 / filteredBulletsPerShot ));
		
		
		FireBullets((int)EASY_CVAR_GET_DEBUGONLY(hassaultBulletsPerShot), vecShootOrigin, vecShootDir, Vector(EASY_CVAR_GET_DEBUGONLY(hassaultFireSpread),EASY_CVAR_GET_DEBUGONLY(hassaultFireSpread),EASY_CVAR_GET_DEBUGONLY(hassaultFireSpread) ), 2048, BULLET_MONSTER_MP5, tracerFreq , EASY_CVAR_GET_DEBUGONLY(hassaultBulletDamageMulti) ); // shoot +-5 degrees
	}
	//if(RANDOM_FLOAT(0, 1) <= EASY_CVAR_GET(hassaultMuzzleFlashChance)){
		pev->effects |= EF_MUZZLEFLASH;
	//}
	
//	m_cAmmoLoaded--;// take away a bullet!

//	Vector angDir = UTIL_VecToAngles( vecShootDir );
//	SafeSetBlending( 0, angDir.x );
	
	if(EASY_CVAR_GET_DEBUGONLY(hassaultFireSound) == 1){
		switch(RANDOM_LONG(0,2))
		{
			case 0:
				UTIL_PlaySound( ENT(pev), getChainFireChannel(), "hassault/hw_shoot1.wav", 1, ATTN_NORM, 0, 100 );
			break;
			case 1:
				UTIL_PlaySound( ENT(pev), getChainFireChannel(), "hassault/hw_shoot2.wav", 1, ATTN_NORM, 0, 100 );
			break;
			case 2:
				UTIL_PlaySound( ENT(pev), getChainFireChannel(), "hassault/hw_shoot3.wav", 1, ATTN_NORM, 0, 100 );
			break;

			//just to know when the idle spin is allowed to be played, if it cannot play alongside fire sounds.
			chainFireSoundDelay = gpGlobals->time + 0.4;
			attemptStopIdleSpinSound();

		}
	}else if(EASY_CVAR_GET_DEBUGONLY(hassaultFireSound) == 2){
		if(chainFireSoundDelay == -1 || chainFireSoundDelay <= gpGlobals->time){
			//chainFireSoundDelay = gpGlobals->time + 2.385;
			chainFireSoundDelay = gpGlobals->time + 2.07;
			UTIL_StopSound(ENT(pev), getChainFireChannel(), "hassault/hw_gun4.wav");
			UTIL_PlaySound(ENT(pev), getChainFireChannel(), "hassault/hw_gun4.wav", 1, ATTN_NORM, 0, 100 );
			attemptStopIdleSpinSound();
		}
	}else if(EASY_CVAR_GET_DEBUGONLY(hassaultFireSound) == 3){
		UTIL_PlaySound( ENT(pev), getChainFireChannel(), "hassault/hw_gun4.wav", 1, ATTN_NORM, 0, 100 );
		attemptStopIdleSpinSound();
	}
	
	if(EASY_CVAR_GET_DEBUGONLY(hassaultExtraMuzzleFlashRadius)){
		Vector extraFlashOrigin = vecShootOrigin + vecShootDir * EASY_CVAR_GET_DEBUGONLY(hassaultExtraMuzzleFlashForward);
	
		float brightness = (EASY_CVAR_GET_DEBUGONLY(hassaultExtraMuzzleFlashBrightness)*255);

		//default radius was just 12... wow.  1.2 is surpsinginy big I guess.
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, extraFlashOrigin );
			WRITE_BYTE(TE_DLIGHT);
			WRITE_COORD(extraFlashOrigin.x);	// X
			WRITE_COORD(extraFlashOrigin.y);	// Y
			WRITE_COORD(extraFlashOrigin.z);	// Z
			WRITE_BYTE( (int)(EASY_CVAR_GET_DEBUGONLY(hassaultExtraMuzzleFlashRadius)*10) );		// radius * 0.1
			WRITE_BYTE( brightness );		// r
			WRITE_BYTE( brightness );		// g
			WRITE_BYTE( brightness );		// b
			WRITE_BYTE( 1 );		// time * 10
			WRITE_BYTE( 0 );		// decay * 0.1
		MESSAGE_END( );
	}

	//This will use the gun position for AI instead so that the pitch doesn't jitter around.
	//MODDD - just like hgrunt does it.
	Vector angDir = UTIL_VecToAngles( vecShootDirAI );

	SafeSetBlending( 0, angDir.x );
}




int CHAssault::getIdleSpinChannel(void){
	return (int)EASY_CVAR_GET_DEBUGONLY(hassaultIdleSpinSoundChannel);
}
int CHAssault::getSpinUpDownChannel(void){
	return (int)EASY_CVAR_GET_DEBUGONLY(hassaultSpinUpDownSoundChannel);
}
int CHAssault::getChainFireChannel(void){
	return (int)EASY_CVAR_GET_DEBUGONLY(hassaultFireSoundChannel);
}


void CHAssault::resetChainFireSound(void){
	UTIL_StopSound(ENT(pev), getChainFireChannel(), "hassault/hw_gun4.wav");
	//EMIT_SOUND( ENT(pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_IDLE );
	chainFireSoundDelay = -1;
}


void CHAssault::attemptStopIdleSpinSound(void){
	attemptStopIdleSpinSound(FALSE);
}

void CHAssault::attemptStopIdleSpinSound(BOOL forceStop){
	if(EASY_CVAR_GET_DEBUGONLY(hassaultIdleSpinSound) > 0 && idleSpinSoundDelay != -1 && ((EASY_CVAR_GET_DEBUGONLY(hassaultIdleSpinSound) >= 3)||forceStop) ){
		//be safe...
		idleSpinSoundDelay = -1;
		UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav");
	}
}
		
void CHAssault::SetTurnActivity(void){
	//If in the spinup anim, don't interrupt it with a turn.
	if(pev->sequence == SEQ_HASSAULT_ATTACK){

		//no.
	}else{
		//see if a change to a turn activity makes sense as normal.
		CSquadMonster::SetTurnActivity();
	}
}//END OF SetTurnActivity


void CHAssault::SetActivity(Activity NewActivity){
	if(m_pSchedule != slHAssault_residualFire && NewActivity != ACT_RANGE_ATTACK1 && EASY_CVAR_GET_DEBUGONLY(hassaultFireSound) >= 2){
			
		resetChainFireSound();
	}
	if(spinuptimeremain >= gpGlobals->time && NewActivity == ACT_IDLE){
		//HACK - if spun up and trying to do an IDLE animation, stay frozen in the attack anim instead.

		//this->SetSequenceByName("attack");
		//this->pev->frame = 0;
		//this->pev->framerate = 0;
		//return;
	}

	int pitch = randomValueInt(m_voicePitch - 1, m_voicePitch + 6);

	if (NewActivity == ACT_SIGNAL1 || NewActivity == ACT_SIGNAL3) {
		//play the sound with it
		UTIL_PlaySound(ENT(pev), CHAN_VOICE, "hgrunt/gr_squadform.wav", 1.0, ATTN_NORM - 0.23, 0, pitch);
	}
	else if (NewActivity == ACT_SIGNAL2) {   //flank_signal"
		//require a 60% chance
		if (RANDOM_FLOAT(0, 1) <= 0.6) {
			UTIL_PlaySound(ENT(pev), CHAN_VOICE, "hgrunt/gr_squadform.wav", 1.0, ATTN_NORM - 0.23, 0, pitch);
		}
		else if(RANDOM_FLOAT(0, 1) <= 0.8) {
			//same as usual
			SENTENCEG_PlayRndSz(ENT(pev), "HG_ALERT", 1, ATTN_NORM - 0.23, 0, pitch);
			//UTIL_PlaySound(ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_LONG(0, ARRAYSIZE(pAlertSounds) - 1)], 1.0, ATTN_NORM - 0.2, 0, pitch);
		}
	}

	EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "HASS ACTIVITY: %d", NewActivity));
	CSquadMonster::SetActivity(NewActivity);

	int ass = 45;
}


BOOL CHAssault::FCanCheckAttacks ( void )
{
	EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "WELL DO YA PUNK %d %d", HasConditions(bits_COND_SEE_ENEMY), HasConditions( bits_COND_ENEMY_TOOFAR )));
	if ( HasConditions(bits_COND_SEE_ENEMY) && !HasConditions( bits_COND_ENEMY_TOOFAR ) )
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CHAssault::CheckRangeAttack1 ( float flDot, float flDist )
{
	//OK. So the sputtering glitch (firing for a little, stopping, firing again) appears to be caused by 
	//the gun's firing origin shifting based on the frame of the anim (it jostles around a little for recoil).

	//Scenario: Gun in default position.
	//          A point from the gun's position to the target is unobscured. Start firing.
	//          But then the gun position changes while firing. Another check may now be obscurred by something in the way. Stop firing.
	//          Gun returns to the default position.
	//          Another check: a point from the gun's position to the target is unobscured, continue firing.
	//          repeat.
	
	if (m_hEnemy == NULL){
		EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "NOOOO 1"));
		
		return FALSE;
	}
	
	const Vector vecShootOrigin = GetGunPositionAI();

	/*
	//MODDD - this seems fairly important to update...
	if(HasConditions(bits_COND_SEE_ENEMY) && m_hEnemy != NULL){
		//is that ok?
		//m_vecEnemyLKP = m_hEnemy->pev->origin + m_hEnemy->EyePosition();
		m_vecEnemyLKP = m_hEnemy->pev->origin;
	}
	*/


	//EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "WHAT THE range %.2f, %.2f", flDot, flDist);
	
	//MODDD - why was "64" ever used for distance?  Seems like melee distance.
	//if ( flDist <= 64 && flDot >= 0.7	&& 


	//Notice that HAssault uses "BodyTargetMod" instead of "BodyTarget".
	//"BodyTargetMod" will not apply the random vertical shift on the player (no idea why its BodyTarget did).
	//For anything else, no difference, cloned per anything else's own "BodyTarget".

	EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "CONDS:::%d %.2f %.2f %d %d %d",
		!HasConditions( bits_COND_ENEMY_OCCLUDED ),
		flDist,
		flDot,
		 m_hEnemy ->Classify() != CLASS_ALIEN_BIOWEAPON,
		 m_hEnemy ->Classify() != CLASS_PLAYER_BIOWEAPON,
	 (EASY_CVAR_GET_DEBUGONLY(hassaultFriendlyFire) != 0 || NoFriendlyFireImp(vecShootOrigin, m_hEnemy->BodyTargetMod(vecShootOrigin)  ) ) 
	 ));


	//make the range larger in the future, seems like they are still fairly accurate further away than this.
	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= m_flDistTooFar &&
	 m_hEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
	 m_hEnemy->Classify() != CLASS_PLAYER_BIOWEAPON && (EASY_CVAR_GET_DEBUGONLY(hassaultFriendlyFire) != 0 || NoFriendlyFireImp(vecShootOrigin, m_hEnemy->BodyTargetMod(vecShootOrigin)  ) ) )
	{
		TraceResult	tr;
		Vector vecSrc = vecShootOrigin;
		UTIL_TraceLine( vecSrc, m_hEnemy->BodyTargetMod(vecSrc), ignore_monsters, ignore_glass, ENT(pev), &tr);

		if(EASY_CVAR_GET_DEBUGONLY(drawDebugPathfinding2) == 1){
			UTIL_drawLineFrame(vecSrc, m_hEnemy->BodyTargetMod(vecSrc), 7, 0, 255, 125);
		}

		/*
		//What? does this say anything about the game being paused when done on the player my nayer?
		//this->edict()->v.gamestate;
		
		edict_t* test2 = tr.pHit;

		//edict_t *edict() { return ENT( pev ); };

		//equivalent.
		edict_t* test3 = m_hEnemy->edict();
		edict_t* test5 = ENT( m_hEnemy->pev );

		//EHANDLE assignment:
		//m_pent = ENT( pEntity->pev );

		edict_t* test4 = m_hEnemy.Get();

		entvars_t* tesst = ((CBaseEntity*)GET_PRIVATE( m_hEnemy.Get( ) ))->pev  ;
		entvars_t* test = &m_hEnemy.Get()->v;
		*/

		//m_hEnemy->edict();
		//m_hEnemy.Get()

		if( (!tr.fStartSolid && tr.flFraction == 1.0) || (tr.pHit != NULL && tr.pHit == m_hEnemy.Get())  )
		{
			//nothing in the way, or the blocking object is our target? good.
			EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "PASSLINE PASSED!"));
		}else{
			EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "Passline Failed"));
			return FALSE;
		}

		
		// in the very least, could attack if turned the right way. And we're looking at them, we aren't psychic after all.
		if(HasConditions(bits_COND_SEE_ENEMY)){
			SetConditionsMod(bits_COND_COULD_RANGE_ATTACK1);
		}
		

		if(flDot >= 0.5 && HasConditions(bits_COND_SEE_ENEMY)){
			//proceed, nothing to see here.
			return TRUE;
		}else{
			//would've happened.
			return FALSE;
		}
	}

	EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "NOOOO 2 %d", m_hEnemy ->Classify()));
	//for one reason or another, could not even attack by turning the right way.
	ClearConditionsMod(bits_COND_COULD_RANGE_ATTACK1);
	return FALSE;
}

BOOL CHAssault::CheckRangeAttack2 ( float flDot, float flDist )
{
	return FALSE;
}


//ripped from HGrunt
BOOL CHAssault::CheckMeleeAttack1(float flDot, float flDist){
	CBaseMonster *pEnemy;
	
	if(EASY_CVAR_GET(hassaultMeleeAttackEnabled	) == 0){
		//blocked.
		return FALSE;
	}

	//FIRST A CHECK.
	if(meleeBlockTime != -1 && gpGlobals->time < meleeBlockTime){
		//we're still waiting for the block to pass? Deny.
		return FALSE;
	}

	SetConditionsMod( bits_COND_COULD_MELEE_ATTACK1 );

	if ( m_hEnemy != NULL )
	{
		pEnemy = m_hEnemy->MyMonsterPointer();
	}
	if ( !pEnemy )
	{
		return FALSE;
	}
	
	EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "THE DISTANCE BE %.2f", flDist));
	//allow any dot product, we're facing the enemy fast enough.
	if ( flDist <= 72 && 
	//if ( flDist <= 75 && flDot >= 0.7	&& 
		 pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		 pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON )
	{

		if(flDot >= 0.7){
			//good to go.
			return TRUE;
		}else{
			return FALSE;
		}

	}else{
		ClearConditionsMod( bits_COND_COULD_MELEE_ATTACK1 );
	}
	return FALSE;
}

BOOL CHAssault::CheckMeleeAttack2 ( float flDot, float flDist )
{
	return FALSE;
}


void CHAssault::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval )
{
	if(firing){
		return;
	}
	CSquadMonster::MoveExecute(pTargetEnt, vecDir, flInterval);
}

void CHAssault::ReportAIState( void )
{
	easyForcePrintLine("resfire: %.2f, %.2f",
		gpGlobals->time,
		residualFireTime
	);

	CSquadMonster::ReportAIState();
}


//=========================================================
// start task
//=========================================================
void CHAssault::StartTask ( Task_t *pTask ){
	TraceResult tr;
	Vector vecEnd;

	//easyForcePrintLine( "HASSAULT: START TASK: %d", pTask->iTask);
	//easyForcePrintLine( "HASSAULT: StartTask s:%s t:%d", getScheduleName(), pTask->iTask);

	const Vector shootOrigin = GetGunPositionAI();
	
	switch ( pTask->iTask )
	{
	case TASK_MELEE_ATTACK1:
		//MODDD SEE
		meleeAttackTimeMax = gpGlobals->time + ((76.0f - 23.0f)/30.0f);
		/*
		if(m_IdealActivity == ACT_MELEE_ATTACK1){
			//pev->frame = 0;
			//m_fSequenceFinished = FALSE;
			signalActivityUpdate = TRUE;  //force an animation reset as soon as possible.
		}
		*/
		
		CBaseMonster::StartTask( pTask );
	break;
	case TASK_HASSAULT_RESIDUAL_FIRE:
		residualFireTime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultResidualAttackTime);
		residualFireTimeBehindCheck = gpGlobals->time + 0.26;
		m_IdealActivity = ACT_RANGE_ATTACK1;

	break;
	case TASK_HASSAULT_START_SPIN:
	case TASK_HASSAULT_SPIN:
	{
		float targetFrameRate;
		
		if(spinuptime == -1){
			//aren't going to rely on a previous spin?  Then start spinning.
			UTIL_PlaySound( ENT(pev), getSpinUpDownChannel(), "hassault/hw_spinup.wav", 1, ATTN_NORM, 0, 100 );


			//OLD FORCE FREEZE FRAME LOCATION

			//animDuration / factor = requiredDuration
			//animDuration = requiredDuration * factor
			//factor = animDuration / requiredDuration

			targetFrameRate = EASY_CVAR_GET_DEBUGONLY(hassaultSpinupStartTime) / (16.0f/15.0f);

			//default time: (16/15) seconds. Scale to take as long as hassaultSpinupStartTime.
			this->SetSequenceByName("spinup", targetFrameRate);


			//only assign "spinuptime" if it hasn't been set yet.
			waittime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultWaitTime);
			spinuptime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultSpinupStartTime);
			spinuptimeIdleSoundDelay = gpGlobals->time + 1.151;
		}else if(spinuptime > gpGlobals->time){

			//not done spinning up? Keep staying to spinning then.
			int xx = 66;
		}else{

			//done spinning? Allowed to check this.
			if(spinuptimeremain != -1){
				//we're already spun up.  Reset the bored timer.

				waittime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultWaitTime);

				spinuptimeremain = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultSpinupRemainTime);
				TaskComplete();
				return;
			}

		}
		
		spinuptimeSet = TRUE;
		//Standin'! 
		m_movementActivity = ACT_IDLE;

		if(pTask->iTask == TASK_HASSAULT_START_SPIN){
			//we were just told to start, so end this task regardless.
			TaskComplete();
		}

	break;
	}
	case TASK_HASSAULT_CHECK_FIRE:

		if(m_hEnemy==NULL){
			ClearConditions(bits_COND_SPECIAL1);
			TaskFail();
			return;
		}
		
		if(m_pSchedule == slHAssault_forceFireAtTarget){

			//easyForcePrintLine("SOOOOO %d", HasConditions(bits_COND_SEE_ENEMY) );
			//if(HasConditions(bits_COND_SEE_ENEMY))
			{
				
				if(m_hEnemy != NULL){
					vecEnd = m_hEnemy->BodyTargetMod(shootOrigin);
				}else{
					vecEnd = m_vecEnemyLKP;
				}

				UTIL_TraceLine( shootOrigin, vecEnd, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
				//UTIL_drawLineFrame(vecSrc, vecEnd, 6, 255, 0, 0);
				if(tr.flFraction == 1.0){
					//hit... nothing?

				}else{
					
					CBaseEntity* tempEntTest = CBaseEntity::Instance(tr.pHit);
					
					if(tempEntTest != NULL && m_hEnemy->pev == tempEntTest->pev){
						//identical, this is ok.  Enemy is now in plain sight.  Stop attacking this descructible.
						//easyForcePrintLine("OOOHHHHH");
						TaskFail();
						return;
					}
				}

			}
		}

		if(!(EASY_CVAR_GET_DEBUGONLY(hassaultFriendlyFire) != 0 || NoFriendlyFireImp(shootOrigin, m_hEnemy->BodyTargetMod(shootOrigin)  ) )    ){
			SetConditions(bits_COND_SPECIAL1);
		}else{
			//MODDD - NOTE: is this necessary?
			ClearConditions(bits_COND_SPECIAL1);
		}
		TaskComplete();
	break;

	default: 
		CBaseMonster::StartTask( pTask );
	break;
	}

}


BOOL CHAssault::FValidateCover ( const Vector &vecCoverLocation )
{
	if ( !InSquad() )
	{
		return TRUE;
	}
	if (SquadMemberInRange( vecCoverLocation, 128 ))
	{
		// another squad member is too close to this piece of cover.
		return FALSE;
	}
	return TRUE;
	
	//far too badass for cover.
	//return FALSE;
}


//=========================================================
// ISoundMask - Overidden for human grunts because they
// hear the DANGER sound that is made by hand grenades and
// other dangerous items.
//=========================================================
int CHAssault::ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			bits_SOUND_DANGER	|
			//MODDD - new
			bits_SOUND_BAIT;
}


//=========================================================
// RunTask
//=========================================================
void CHAssault::RunTask ( Task_t *pTask )
{
	recentSchedule = m_pSchedule;

	firing = FALSE;
	//most things are certainly not "firing".

	//easyForcePrintLine("HASSAULT: RunTask: s:%s t:%d", getScheduleName(), pTask->iTask);
	//EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "HASSAULT ACT: %d MOVACT: %d SCHED: %s TASK: %d", m_IdealActivity, m_movementActivity, m_pSchedule->pName, pTask->iTask));

	switch ( pTask->iTask )
	{
	case TASK_MELEE_ATTACK1:

		if(spinuptimeremain != -1){
			//if spun-up, definitely staying that way in the heat of combat.
			waittime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultWaitTime);
			spinuptimeremain = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultSpinupRemainTime);
			//TaskComplete();
			//return;
		}

		//face the enemy throughout the attack.
		MakeIdealYaw( m_vecEnemyLKP );
		ChangeYaw( pev->yaw_speed );
		
		// aim for a litle earlier if we're still able to attack. No need to return all the way to resting position.
		if(m_fSequenceFinished || (HasConditions(bits_COND_CAN_MELEE_ATTACK1)  && pev->frame >= 220)  )
		{
			m_Activity = ACT_RESET;
			//m_IdealActivity = ACT_RESET;
			//MODDD - is this better?

			//EYY MAN WATCH OUT YOU DONT KNOW THAT THIS IS WANTED!!
			//m_IdealActivity = ACT_IDLE;
			//SetActivity(ACT_IDLE);
			//!!!
			//signalActivityUpdate = TRUE;  //in case we need to restart this animation next time.
			TaskComplete();
			return;
		}
		

		if(m_hEnemy != NULL){
			EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "MAMA MIA!!! %d %.2f", HasConditions(bits_COND_CAN_MELEE_ATTACK1), (m_hEnemy->pev->origin - pev->origin).Length()));
		}

		if(!HasConditionsEither(bits_COND_CAN_MELEE_ATTACK1)){

			if( HasConditionsModEither(bits_COND_COULD_MELEE_ATTACK1)){
				// then face them!  'Either' for COULD makes sense too, right?
				ChangeSchedule(slCombatFaceNoStump);
				return;
			}else{
				//keep moving?
			}

			//m_IdealActivity = ACT_RESET;
			if(m_hEnemy != NULL && (m_hEnemy->pev->origin - pev->origin).Length() > 79){
				TaskFail();
				//better?
				//m_IdealActivity = ACT_RESET;
				m_IdealActivity = ACT_IDLE;
				SetActivity(ACT_IDLE);
				ChangeSchedule(GetSchedule());

				return;
			}
		}

		//that's right... right?  don't call the parent?
		CSquadMonster::RunTask(pTask);

	break;
	case TASK_HASSAULT_WAIT_FOR_SPIN_FINISH:    //piggy back off of the script below.
	case TASK_HASSAULT_SPIN:

		spinuptimeremain = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultSpinupRemainTime);
		waittime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultWaitTime);
		//certainly not bored now.
		

		//easyForcePrintLine("Are you waiting on me? %.2f %.2f", spinuptime, gpGlobals->time);

		//yeeeyyy
		//"spinuptimeremain" being under gpGlobals->time means, we're already spun up from a previous time.
		if( spinuptime <= gpGlobals->time){
			
			spinuptime = -1;
		

			TaskComplete();
			return;
		}

		//just keep trying to look at the enemy, so do the same effect as "TASK_FACE_ENEMY" this whole time.
		MakeIdealYaw( m_vecEnemyLKP );
		ChangeYaw( pev->yaw_speed );


		if ( m_hTargetEnt != NULL )
		{
			//MakeIdealYaw ( m_hTargetEnt->pev->origin );
			SetTurnActivity(); 
		}
		//m_IdealActivity = ACT_STAND;
	break;


	case TASK_WAIT_FOR_MOVEMENT:

		if(firing){
			EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "PHALLICE1"));
			m_IdealActivity = ACT_IDLE;
			firing = FALSE;
		}

		if ( HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_ENEMY_OCCLUDED)  ){
			//can see the enemy just fine?   What are you waiting for?!
			EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "dont do that?"));
			//if we can't "range attack", then it's not good enough of a place to stop.
			if( HasConditions(bits_COND_CAN_RANGE_ATTACK1 ) ){
				EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "dont do that 2"));
				TaskFail();
				return;
			}else{
				if(HasConditionsMod(bits_COND_COULD_RANGE_ATTACK1)){
					EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "dont do that 3"));
					ChangeSchedule(slCombatFaceNoStump);
					return;
				}else{
					//keep moving?
				}

				//ChangeSchedule(slCombatFace);
				//return;
			}
		}
		CSquadMonster::RunTask( pTask );

	break;
	case TASK_RANGE_ATTACK1:
		if(m_hEnemy == NULL){
			TaskFail();
			return;
		}
		//yep.
		firing = TRUE;
		
		spinuptimeremain = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultSpinupRemainTime);
		waittime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultWaitTime);


		//CheckAttacks(m_hEnemy,  (pev->origin - m_hEnemy->pev->origin).Length() );

		//should we stop firing?
		if(!HasConditionsEither(bits_COND_CAN_RANGE_ATTACK1) ){
		//if(!HasConditionsSetThisFrame(bits_COND_CAN_RANGE_ATTACK1) ){

			//MODD - check for """could""" to just stop shooting but turn to look maybe?
			EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyForcePrintLine("OH snap my man"));
			TaskFail();
			return;
		}

		EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyForcePrintLine( "HASSAULT: range atta. FacingIdeal:%d enemy null?:%d targetent null?:%d", !FacingIdeal(), m_hEnemy!=NULL, m_hTargetEnt != NULL));
		
		if ( !FacingIdeal() )
		{
			//TaskComplete();
			//progress to turn!
			

			MakeIdealYaw( m_vecEnemyLKP );
			ChangeYaw( pev->yaw_speed );
			if ( m_hEnemy != NULL || m_hTargetEnt != NULL )
			{
				//MakeIdealYaw ( m_hTargetEnt->pev->origin );

				//mODDD NOTE - !!! Is this call a good idea here?!
				SetTurnActivity(); 
			}
		
		}
		
		//CSquadMonster::RunTask( pTask );

		

		MakeIdealYaw ( m_vecEnemyLKP );
		ChangeYaw ( pev->yaw_speed );

		/*
		// drop this part, let it keep looping.
		if ( m_fSequenceFinished )
		{
			m_Activity = ACT_RESET;
			TaskComplete();
		}
		*/


		if ( m_fSequenceFinished )
		{
			//Force a reset, cut out the middleman!
			//pev->frame = 0;
			//...wait doesn't this already naturally loop? what?
		}

	break;
	case TASK_HASSAULT_FACE_FORCEFIRE:

		if ( !FacingIdeal() )
		{
			MakeIdealYaw( forceFireTargetPosition );
			ChangeYaw( pev->yaw_speed );
			if ( m_hTargetEnt != NULL )
			{
				//MakeIdealYaw ( m_hTargetEnt->pev->origin );
				SetTurnActivity(); 
			}
		}else{
			TaskComplete();
			return;
		}

	break;
	case TASK_HASSAULT_FORCEFIRE:
		
		if ( !FacingIdeal() )
		{
			MakeIdealYaw( forceFireTargetPosition );
			ChangeYaw( pev->yaw_speed );
			//if ( m_hTargetEnt != NULL )
			//{
				//MakeIdealYaw ( m_hTargetEnt->pev->origin );
				SetTurnActivity(); 
			//}
			return;
		}else{
			//just go on.
			//TaskComplete();
			//return;
		}

		//if(forceFireTargetObject.Get()->free || forceFireGiveUp <= gpGlobals->time){
		if(forceFireTargetObject == NULL || forceFireGiveUp <= gpGlobals->time){
			//Destroyed that thing in the way?  Pick a new schedule!
			TaskFail();
			return;
		}
		//TaskComplete();
	break;

	case TASK_HASSAULT_RESIDUAL_FIRE:
		firing = TRUE;

		spinuptimeremain = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultSpinupRemainTime);
		waittime = gpGlobals->time + EASY_CVAR_GET_DEBUGONLY(hassaultWaitTime);

		EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "Residual fire."));

		if ( HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_ENEMY_OCCLUDED)  ){
			// can see the enemy just fine?   What are you waiting for?!

			//if we can't "range attack", then it's not good enough of a place to stop.
			if(  HasConditionsEither(bits_COND_CAN_RANGE_ATTACK1)   ){
				ChangeSchedule(slHAssault_fire);
				//TaskFail();
				return;
			}else{
				if(HasConditionsMod(bits_COND_COULD_RANGE_ATTACK1)){
					ChangeSchedule(slCombatFaceNoStump);
					return;
				}else{
					// keep moving?
				}
			}
		}

		// should we stop firing?
		if(residualFireTime <= gpGlobals->time){
			residualFireTime = -1;
			TaskComplete();
			/*
			/////////////////////////////////////
			this->SetSequenceByName("attack");
			this->pev->frame = 0;
			this->pev->framerate = 0;
			// is this a good idea? VERIFY!
			*/
			return;
		}

		//if it's been just a tiny amount of time after...
		if(residualFireTimeBehindCheck != -1 && residualFireTimeBehindCheck <= gpGlobals->time){
			residualFireTimeBehindCheck = -1;

			//do a straight-line check...
			TraceResult	tr;
			const Vector vecSrc = GetGunPositionAI();
			Vector vecEnd;
				
			if(m_hEnemy != NULL){
				
				EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "OH BOY! yay"));
				vecEnd = m_hEnemy->BodyTargetMod(vecSrc);
			}else{
				
				EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "OH BOY!  fuk!"));
				vecEnd = m_vecEnemyLKP;
			}

			UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ignore_glass, ENT(pev), &tr);
			if(tr.flFraction == 1.0){
				//hit... nothing?
				EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "Hit nothing!"));
			}else{
				
				EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "AW heck YEAHH %s", STRING(tr.pHit->v.classname) ));
				//if( FClassnameIs(&tr.pHit->v, "func_breakable") || FClassnameIs(&tr.pHit->v, "func_pushable")  ){


				if(tr.pHit != NULL){

					CBaseEntity* tempEntTest = CBaseEntity::Instance(tr.pHit);

					if( tempEntTest != NULL && tempEntTest->isDestructibleInanimate() ){
						//shoot at the breakable!
					
						CBaseEntity* derp = Instance(tr.pHit);
						if(derp != NULL){
							forceFireTargetPosition = derp->Center();
							//forceFireTargetObject = tr.pHit;
						
							forceFireTargetObject = tempEntTest;

							forceFireGiveUp = gpGlobals->time + 6;
							ChangeSchedule(slHAssault_forceFireAtTarget);
						}

					}
				}//END OF tr.pHit NULL check

			}
		}//END OF residualFireTimeBehindCheck

	break;
	case TASK_FACE_ENEMY:

		/*
		if(HasConditions(bits_COND_SEE_ENEMY) && m_hEnemy != NULL){
			//is that ok?
			//m_vecEnemyLKP = m_hEnemy->pev->origin + m_hEnemy->EyePosition();
			m_vecEnemyLKP = m_hEnemy->pev->origin;
		}
		*/
		EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "HASSAULT: TASK_FACE_ENEMY ??? %d %d", HasConditions(bits_COND_SEE_ENEMY), m_hEnemy!=NULL));


		debugDrawVect = m_vecEnemyLKP;		
			
		MakeIdealYaw( m_vecEnemyLKP );
		ChangeYaw( pev->yaw_speed );


		if ( FacingIdeal() )
		{
			//TaskComplete();
			//the RunTask parent call below will figure this out.
		}else{

			if(m_IdealActivity == m_movementActivity && (m_IdealActivity != ACT_TURN_RIGHT || m_IdealActivity != ACT_TURN_LEFT) ){
				//if moving, stop to look.
				SetTurnActivity(); 
			}
		}
		CSquadMonster::RunTask( pTask );  //why... why not chec kthis?
	break;
	default: 
		CSquadMonster::RunTask( pTask );
	break;
	}
}

extern Schedule_t slError[];

Schedule_t* CHAssault::GetSchedule(){

	nonStumpableCombatLook = FALSE; //by default.

	//MODDD - safety.
	if(pev->deadflag != DEAD_NO){
		return GetScheduleOfType( SCHED_DIE );
	}

	/*
	if(recentSchedule == NULL){
		EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyForcePrintLine( "hassault recentSchedule: none?"));
	}else{
		EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyForcePrintLine( "hassault recentSchedule: %s", recentSchedule->pName));
	}
	*/
	if(recentSchedule == NULL){
		EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyForcePrintLine( "hassault recentSchedule: none?"));
	}else{
		EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyForcePrintLine( "hassault recentSchedule: %s", recentSchedule->pName));
	}
	if(recentRecentSchedule == NULL){
		EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyForcePrintLine( "hassault recentRecentSchedule: none?"));
	}else{
		EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyForcePrintLine( "hassault recentRecentSchedule: %s", recentRecentSchedule->pName));
	}
	
	//MODD  - experimental
	SCHEDULE_TYPE baitSched = getHeardBaitSoundSchedule();

	if(baitSched != SCHED_NONE){
		return GetScheduleOfType ( baitSched );
	}

	if(recentSchedule == slHAssault_fire && forceBlockResidual == FALSE){
		recentSchedule = NULL;
		//never twice.
		forceBlockResidual = TRUE;
		return GetScheduleOfType(SCHED_HASSAULT_RESIDUAL_FIRE);
	}

	if(recentSchedule == slHAssault_residualFire ||
		recentSchedule == slHAssault_forceFireAtTarget ||
		recentSchedule == slCombatFaceNoStump ||//     what ?!
		recentSchedule == slHAssault_spin ||
		recentSchedule == slHAssault_fire
		
	){
		//hacky fix. anything following residual fires is non-stumpable.

		//EVEN HACKIER: if the recentSchedule was slCombatFaceNoStump before, don't allow it again this time.
		//if( !(recentRecentSchedule == slCombatFaceNoStump && recentSchedule == slCombatFaceNoStump) ){
		nonStumpableCombatLook = TRUE;
		//}
	}

	recentRecentSchedule = recentSchedule;

	if(HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_ENEMY_OCCLUDED) ){
		//residual reset.
		forceBlockResidual = FALSE;
	}
	
	switch	( m_MonsterState )
	{
	case MONSTERSTATE_PRONE:
		{
			return GetScheduleOfType( SCHED_BARNACLE_VICTIM_GRAB );
			break;
		}
	case MONSTERSTATE_NONE:
		{
			ALERT ( at_aiconsole, "MONSTERSTATE IS NONE!\n" );
			break;
		}
	case MONSTERSTATE_IDLE:
		{
			if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_ALERT_FACE );
			}
			else if ( FRouteClear() )
			{
				// no valid route!
				return GetScheduleOfType( SCHED_IDLE_STAND );
			}
			else
			{
				// valid route. Get moving
				return GetScheduleOfType( SCHED_IDLE_WALK );
			}
			break;
		}
	case MONSTERSTATE_ALERT:
		{
			if ( HasConditions( bits_COND_ENEMY_DEAD ) && LookupActivity( ACT_VICTORY_DANCE ) != ACTIVITY_NOT_AVAILABLE )
			{
				return GetScheduleOfType ( SCHED_VICTORY_DANCE );
			}

			if ( HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE) )
			{
				if ( fabs( FlYawDiff() ) < (1.0 - m_flFieldOfView) * 60 ) // roughly in the correct direction
				{
					return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ORIGIN );
				}
				else
				{
					return GetScheduleOfType( SCHED_ALERT_SMALL_FLINCH );
				}
			}

			else if ( HasConditions ( bits_COND_HEAR_SOUND ) )
			{
				return GetScheduleOfType( SCHED_ALERT_FACE );
			}
			else
			{
				return GetScheduleOfType( SCHED_ALERT_STAND );
			}
			break;
		}
	case MONSTERSTATE_COMBAT:
		{
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// clear the current (dead) enemy and try to find another.
				m_hEnemy = NULL;

				if ( GetEnemy() )
				{
					ClearConditions( bits_COND_ENEMY_DEAD );
					return GetSchedule();
				}
				else
				{
					SetState( MONSTERSTATE_ALERT );
					return GetSchedule();
				}
			}

			if ( HasConditions(bits_COND_NEW_ENEMY) )
			{
				if ( InSquad() )
				{
					MySquadLeader()->m_fEnemyEluded = FALSE;

					if ( !IsLeader() )
					{
						//no, I don't go for cover.
						//return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
					}
					else
					{
						//HGrunt's "alerted" script to go with the hand signal.
						//The hassault already has an alert sound that will play, can replace it with this hgrunt sentence instead of desired
						//just for this (squad leader, not eluded, etc.)
						/*
						if (FOkToSpeak())// && RANDOM_LONG(0,1))
						{
							if ((m_hEnemy != NULL) && m_hEnemy->IsPlayer())
								// player
								SENTENCEG_PlayRndSz( ENT(pev), "HG_ALERT", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
							else if ((m_hEnemy != NULL) &&
									(m_hEnemy->Classify() != CLASS_PLAYER_ALLY) &&
									(m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE) &&
									(m_hEnemy->Classify() != CLASS_MACHINE))
								// monster
								SENTENCEG_PlayRndSz( ENT(pev), "HG_MONST", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);

							JustSpoke();
						}
						*/

						//if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
						if ( m_hEnemy != NULL && !HasConditions ( bits_COND_ENEMY_OCCLUDED ) && gpGlobals->time > signal2Cooldown )
						{
							// don't do this signal again too often.
							signal2Cooldown = gpGlobals->time + 10;
							return GetScheduleOfType ( SCHED_HASSAULT_SUPPRESS );
						}
						else
						{
							// just let the usual schedule play out.   SCHED_CHASE_ENEMY?
							//return GetScheduleOfType ( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
						}
					}
				}
				return GetScheduleOfType ( SCHED_WAKE_ANGRY );
			}
			//MODDD - other condition.  If "noFlinchOnHard" is on and the skill is hard, don't flinch from getting hit.
			else if (HasConditions(bits_COND_LIGHT_DAMAGE) && !HasMemory( bits_MEMORY_FLINCHED) && !(EASY_CVAR_GET_DEBUGONLY(noFlinchOnHard)==1 && g_iSkillLevel==SKILL_HARD)  )
			{
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}
			else if ( !HasConditions(bits_COND_SEE_ENEMY) )
			{
				//MODDD - yes?
				if ( HasConditions ( bits_COND_HEAR_SOUND ) )
				{
					//return GetScheduleOfType( COMBAT_ALERT_FACE_NOSTUMP );

					//nonStumpableCombatLook = TRUE;
					//return GetScheduleOfType( SCHED_COMBAT_FACE );

					CSound *pSoundTemp;
					pSoundTemp = PBestSound();

					//For now to be safe...
					if(pSoundTemp && pSoundTemp->m_iType & bits_SOUND_PLAYER ){

						//-1 + 1.2 = 0.2, so -1 to 0.2 (a little over +-90 deg from the back)
						if(UTIL_IsFacingAway(this->pev, pSoundTemp->m_vecOrigin, 1.2)){
							return &slCombatFaceSound[0];
							
						}else{
							//keep on' marchin.
							this->ClearConditions(bits_COND_HEAR_SOUND);
							return GetSchedule();
						}

						
					}
				}

				// we can't see the enemy
				if ( !HasConditions(bits_COND_ENEMY_OCCLUDED) )
				{
					// enemy is unseen, but not occluded!
					// turn to face enemy

					if(m_hEnemy == NULL){
						//??? is this safe?
						SetState( MONSTERSTATE_ALERT );
						return GetSchedule();
					}

					//MODDD - is that ok?
					// Maybe not, kinda gives them that 'eyes in the back of the head' power.
					//m_vecEnemyLKP = m_hEnemy->pev->origin;
					//setEnemyLKP(m_hEnemy->pev->origin);
					
					this->nonStumpableCombatLook = FALSE;

					if(!FacingIdeal()){
						// enemy is unseen, but not occluded!
						// turn to face enemy
						return GetScheduleOfType(SCHED_COMBAT_FACE);
					}else{
						//We're facing the LKP already. Then we have to go to that point and declare we're stumped there if we still see nothing.
						return GetScheduleOfType(SCHED_CHASE_ENEMY);
					}

				}
				else
				{
					// chase!
					//...confirm with "spinuptimeremain" though.
					EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "ducks??"));

					if(waittime == -1){
						//we're bored, go look.
						return GetScheduleOfType( SCHED_CHASE_ENEMY );
					}else{
						if( HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_CAN_RANGE_ATTACK1 ) ){
							//can't ranged attack, but can see them?  Attempt to get closer...
							return GetScheduleOfType( SCHED_CHASE_ENEMY );
						}
						EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "lets not do that."));
						m_IdealActivity = ACT_IDLE;
						return GetScheduleOfType( SCHED_COMBAT_FACE );
					}

					
				}
			}
			else  
			{
				//MODDD - yes?
				if ( HasConditions ( bits_COND_HEAR_SOUND ) )
				{
					//return GetScheduleOfType( COMBAT_ALERT_FACE_NOSTUMP );
					return slCombatFaceNoStump;
				}
				
				//SLIGHTLY DIFFERENT ORDER.  priotize melee attacks.
				if ( HasConditionsEither(bits_COND_CAN_MELEE_ATTACK1) )
				{
					
					//return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
					return GetScheduleOfType( SCHED_HASSAULT_MELEE1 );
				}



				// we can see the enemy
				if ( HasConditions(bits_COND_CAN_RANGE_ATTACK1) )
				{
					if ( InSquad() )
					{
						// if the enemy has eluded the squad and a squad member has just located the enemy
						// and the enemy does not see the squad member, issue a call to the squad to waste a
						// little time and give the player a chance to turn.
						if ( MySquadLeader()->m_fEnemyEluded && !HasConditions ( bits_COND_ENEMY_FACING_ME ) && gpGlobals->time > signal1Cooldown )
						{
							MySquadLeader()->m_fEnemyEluded = FALSE;
							EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "SCHED HASSAULT SPIN FOUND ENEMY"));
							signal1Cooldown = gpGlobals->time + 10;
							return GetScheduleOfType(SCHED_HASSAULT_FOUND_ENEMY);
						}
					}


					//above did not apply? Plain spin up.
					//return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
					return GetScheduleOfType(SCHED_HASSAULT_SPIN);
				}


				if ( HasConditions(bits_COND_CAN_RANGE_ATTACK2) )
				{
					return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
				}
				/*
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK1) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
				}
				*/
				if ( HasConditions(bits_COND_CAN_MELEE_ATTACK2) )
				{
					return GetScheduleOfType( SCHED_MELEE_ATTACK2 );
				}
				if ( !HasConditions(bits_COND_CAN_RANGE_ATTACK1 | bits_COND_CAN_MELEE_ATTACK1) )
				{
					// if we can see enemy but can't use either attack type, we must need to get closer to enemy
					EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "ducks2"));
					//NOPE!  Hassault will wait for the spin to finish...
					
					//return GetScheduleOfType( SCHED_CHASE_ENEMY );



					// WAIT!  If you can see the enemy, why the hell aren't you looking?
					// this space already implies we can see the enemy, just not looking directly at em' I guess.
					if (!FacingIdeal() && !HasConditions(bits_COND_ENEMY_OCCLUDED) && !HasConditions(bits_COND_CAN_RANGE_ATTACK1) && (pev->origin - m_hEnemy->pev->origin).Length() < m_flDistTooFar){
						// look at them!
						return GetScheduleOfType(SCHED_COMBAT_FACE);
					}else if(waittime == -1){
						//we're bored, go look.
						return GetScheduleOfType( SCHED_CHASE_ENEMY );
					}else{
						if( HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_CAN_RANGE_ATTACK1 ) ){
							//can't ranged attack, but can see them?  Attempt to get closer...
							return GetScheduleOfType( SCHED_CHASE_ENEMY );
						}
						EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "PHALLICE3"));

						m_IdealActivity = ACT_IDLE;
						return GetScheduleOfType( SCHED_COMBAT_FACE );
					}
				}
				else if ( !FacingIdeal() )
				{
					//turn
					return GetScheduleOfType( SCHED_COMBAT_FACE );
				}
				else
				{
					ALERT ( at_aiconsole, "No suitable combat schedule!\n" );
				}
			}
			break;
		}
	case MONSTERSTATE_DEAD:
		{
			return GetScheduleOfType( SCHED_DIE );
			break;
		}
	case MONSTERSTATE_SCRIPT:
		{
			//
			//ASSERT( m_pCine != NULL );

			if(m_pCine == NULL){
				EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "WARNING: m_pCine IS NULL!"));
			}

			if ( !m_pCine )
			{
				ALERT( at_aiconsole, "Script failed for %s\n", STRING(pev->classname) );
				CineCleanup();
				return GetScheduleOfType( SCHED_IDLE_STAND );
			}

			return GetScheduleOfType( SCHED_AISCRIPT );
		}
	default:
		{
			ALERT ( at_aiconsole, "Invalid State for GetSchedule!\n" );
			break;
		}
	}

	return &slError[ 0 ];
}



Schedule_t* CHAssault::GetScheduleOfType(int Type){

	//EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "HASSAULT GET SCHED TYPE: %d ", Type));
	EASY_CVAR_PRINTIF_PRE(hassaultPrintout, ( "HASSAULT GET SCHED TYPE: %d ", Type));

	switch(Type){
		case SCHED_HASSAULT_FORCEFIRE:
			return &slHAssault_forceFireAtTarget[0];
		break;
		case SCHED_HASSAULT_MELEE1:
			return &slHAssault_melee1[0];
		break;

		case SCHED_HASSAULT_RESIDUAL_FIRE:
			return &slHAssault_residualFire[0];
		break;
		case SCHED_HASSAULT_GENERIC_FAIL:
			return &slHAssaultGenericFail[0];

		break;

		case SCHED_HASSAULT_SPIN:
			{
			
			spinuptimeSet = FALSE;
			//MODDD - TEST: is this okay?
			//...no, being -1 is okay.   This is in order to call "spin", after all.

			const float cheaterFloat = gpGlobals->time;

			if( (spinuptime!=-1 && spinuptime > gpGlobals->time) || (spinuptimeremain == -1 || spinuptimeremain <= gpGlobals->time)  ){
				
				EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "SCHED HASSAULT SPIN"));
				return &slHAssault_spin[0];
			}else{

				if(recentSchedule != slHAssault_fire){
					//if we started firing after doing something else, disallow melee attacks for a second.
					meleeBlockTime = gpGlobals->time + 1;
				}
				EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "SCHED HASSAULT FIRE"));
				return &slHAssault_fire[0];
				
			}
			
			break;
			}

		case SCHED_HASSAULT_FOUND_ENEMY:
			return &slHAssault_foundEnemy[0];
		break;
		case SCHED_HASSAULT_SUPPRESS:
			return &slHAssault_suppress[0];
		break;
		case SCHED_CHASE_ENEMY:
			spinuptimeSet = FALSE;
			EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "SCHED CHASE_ENEMY"));
			return &slHAssault_follow[0];
		break;
		case SCHED_HASSAULT_FIRE:
			EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "SCHED HASSAULT_FIRE"));

			if(recentSchedule != slHAssault_fire){
				//if we started firing after doing something else, disallow melee attacks for a second.
				meleeBlockTime = gpGlobals->time + 1;
			}
			return &slHAssault_fire[0];
		break;
		case SCHED_HASSAULT_FIRE_OVER:
			//...?  TASK_HASSAULT_SPINDOWN
			EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "FAILURE HASSAULT_FIRE_OVER FOREVER"));

			//DO STUFF HEAL REPAIR!

			return &slHAssaultFireOver[0];
		break;
		case SCHED_COMBAT_FACE:
			//easyForcePrintLine("SCHED_COMBAT_FACE???? nonStump:%d : recentsched:%s", nonStumpableCombatLook, recentSchedule!=NULL?this->recentSchedule->pName:"NULL");
			
			//MODDD - not having so much luck with nostump. Try always stumpable.
			//return &slCombatFaceSound[0];

			if(nonStumpableCombatLook){
				return &slCombatFaceNoStump[0];
			}else{
				return &slCombatFace[0];
			}
			
			//return &slCombatFace[0];
		break;
	}
	return CSquadMonster::GetScheduleOfType(Type);
}


//Sounds with short durations tend to continue after they are called once, as though it is assumed they are supposed to loop.
//So they must be explicitly stopped.
//Why can't sounds looping automatically be a setting of its own in script? No idea.
void CHAssault::onDelete(void){
	UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav");
	UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spindown.wav");
	UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spinup.wav");
}


GENERATE_KILLED_IMPLEMENTATION(CHAssault){
	//spinuptimeremain = -1;
	//spinuptime = -1;

	if(ShouldGibMonster( iGib )){
		//going to gib? stop all!

		//force it to stop just in case...
		//stopIdleSpinSoundForever = TRUE;
		UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav");
		UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spindown.wav");
		UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spinup.wav");
		if(EASY_CVAR_GET_DEBUGONLY(hassaultFireSound) >= 2){
			resetChainFireSound();
		}
		spinuptimeremain = -1;
		spinuptime = -1;

	}else{

		//force it to stop just in case...
		//stopIdleSpinSoundForever = TRUE;
		UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav");

		if(EASY_CVAR_GET_DEBUGONLY(hassaultFireSound) >= 2){
			resetChainFireSound();
		}
	}

	GENERATE_KILLED_PARENT_CALL(CSquadMonster);
}


Vector giveZ(const Vector2D& arg, const float& myZ){
	return Vector(arg.x, arg.y, myZ);
}


void CHAssault::MonsterThink ( void )
//void CHAssault::HAssaultThink( void ) //Save & Restore only work properly if this is MonsterThink
{
	//MODDD - NOTICE.   Nothing in this think method, as of yet, seems to depend on the monster being alive or not.
	//If something should be placed here that does (or in a lot of places), have a dead check first.

	//easyForcePrintLine("ID:%d SEQ:%d SCHED:%s TASK:%d", monsterID, this->pev->sequence, getScheduleName(), getTaskNumber());
	//easyForcePrintLine("ugh YOU. sched:%s task:%d fr:%.2f f:%.2f s:%d act:%d iact:%d", getScheduleName(), getTaskNumber(), pev->framerate, pev->frame, pev->sequence, m_Activity, m_IdealActivity);

	if(rageTimer != -1){
		//movementBaseFramerate
		//currently ACT_CREEPING_WALK, the only movement anim. no run one.
		if(pev->sequence == SEQ_HASSAULT_CREEPING_WALK){
			if(gpGlobals->time <= rageTimer){
				//range: 1.8 - 1.3
				float angryFactor = ((rageTimer - gpGlobals->time) / 8.0f ) * 0.5 + 1.3;


				pev->framerate = movementBaseFramerate * angryFactor;
			}else{
				//it's over.  Restore in case of any changes.
				pev->framerate = movementBaseFramerate;
				rageTimer = -1;
			}
		}else{
			//just stop.
			rageTimer = -1;
		}
	}//END OF rageTimer check
	else{
		//normal framerate.
	}

	//is that okay.
	if(EASY_CVAR_GET_DEBUGONLY(hassaultDrawLKP) == 1){
		UTIL_drawLineFrameBoxAround(debugDrawVect, 9, 30, 255, 0, 0);
	}else if(EASY_CVAR_GET_DEBUGONLY(hassaultDrawLKP) == 2){
		UTIL_drawLineFrameBoxAround(m_vecEnemyLKP, 9, 30, 255, 0, 0);
	}
	//easyPrintLine("HASSAULT:::whut %s %d  %d", this->getScheduleName(), this->getTaskNumber(), this->HasConditions(bits_COND_ENEMY_DEAD));
	
	if(EASY_CVAR_GET_CLIENTSENDOFF_BROADCAST_DEBUGONLY(thatWasntPunch) == 1 && (this->m_fSequenceFinished || pev->frame >= 245)){

		switch(RANDOM_LONG(0, 26)){

			case 0:this->SetSequenceByName("idle");break;
			case 1:this->SetSequenceByName("idle");break;case 2:this->SetSequenceByName("idle1");break;
			case 3:this->SetSequenceByName("idle1");break;
			case 4:this->SetSequenceByName("turn_left");break;case 5:this->SetSequenceByName("turn_left");break;
			case 6:this->SetSequenceByName("turn_right");break;
			case 7:this->SetSequenceByName("turn_right");break;
			case 8:this->SetSequenceByName("melee");break;
			case 9:this->SetSequenceByName("melee");break;
			case 10:this->SetSequenceByName("melee1");break;
			case 12:this->SetSequenceByName("melee1");break;

			case 13:this->SetSequenceByName("small_pain");break;
			case 14:this->SetSequenceByName("small_pain");break;
			case 15:this->SetSequenceByName("small_pain");break;
			case 16:this->SetSequenceByName("small_pain");break;
			case 17:this->SetSequenceByName("small_pain2");break;
			case 18:this->SetSequenceByName("small_pain2");break;
			case 19:this->SetSequenceByName("small_pain2");break;
			case 20:this->SetSequenceByName("small_pain2");break;
			case 21:this->SetSequenceByName("barnacled1");break;
			case 22:this->SetSequenceByName("barnacled2");break;
			case 23:this->SetSequenceByName("barnacled3");break;
			case 24:this->SetSequenceByName("barnacled4");break;
			case 25:this->SetSequenceByName("die_backwards"); break;
			case 26:this->SetSequenceByName("throwgrenade"); break;
		}
	}
	//EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "MOVEMENT WAIT COND: %d %d %d" , HasConditions(bits_COND_SEE_ENEMY), !HasConditions(bits_COND_ENEMY_OCCLUDED), HasConditions(HasConditionsEither(bits_COND_CAN_RANGE_ATTACK1) ) ));
	
	//UTIL_drawLineFrameBoxAround(pev->origin, 3, 326, 255, 255, 0);
	
	//bits_COND_SEE_ENEMY |
	//if(m_hEnemy != NULL && pev->sequence == 0 && pev->frame == 0 && pev->framerate == 0 && HasConditions( bits_COND_CAN_RANGE_ATTACK1) ){

	//point is when spun up, don't do the idle animation completely. stay frozen in place.
	
	//HACKY: way to see if the player is in the attack anim but spinning. If so, stay frozen at frame 0 of that animation.
	if(m_Activity == ACT_IDLE && pev->sequence == SEQ_HASSAULT_SMALL_PAIN2){
		//easyForcePrintLine("OH NO WHATS HOT");

		if(
			//(waittime != -1 && waittime > gpGlobals->time) &&
			(
			(spinuptimeremain != -1 && spinuptimeremain > gpGlobals->time) ||
		  (spinuptime != -1 && spinuptime > gpGlobals->time)
		  )
		  
		){
			//can ranged attack and facing the enemy, frozen in this sequence? We're probably aiming at them. Adjust your pitch or it could look silly.
		
				pev->sequence = SEQ_HASSAULT_SMALL_PAIN2;
				this->pev->frame = 0;
				this->pev->framerate = 0;

				//if(m_Activity == ACT_IDLE){
					//don't look like you're casually swinging your gun around, you're about to start shooting!
					//this->SetSequenceByName("attack");
					//this->pev->frame = 0;
					//this->pev->framerate = 0;
				//}
				
				Vector vecShootOrigin;
				Vector vecShootDir;
				AimAtEnemy(vecShootOrigin, vecShootDir);

				//if(m_hEnemy != NULL){
//
				//}else{
				//	//Have to set the blending
				//	SafeSetBlending( 0, 0 );
				//}
		}else{
			//not spinning up or spun up? Just animate like you should.
			//if(pev->sequence == SEQ_HASSAULT_SMALL_PAIN2){
				pev->framerate = 1;
				this->signalActivityUpdate = TRUE;
				SetActivity(ACT_IDLE);
			//}
		}
	}//END OF ACT_IDLE check

	if(waittime != -1 && waittime <= gpGlobals->time){
		waittime = -1;
	}

	if(spinuptimeremain != -1){
		//UTIL_PlaySound( ENT(pev), CHAN_WEAPON, "hassault/hw_spin.wav", 1, ATTN_NORM, 0, 100 );
		
		//could do  " || m_pSchedule == slDie ",  not as reliable for sound though.
		if(spinuptimeremain <= gpGlobals->time || m_pSchedule == slDie){
				//we're bored.  Play unspin.   Know this means we can try to scout where the enemy is.
				UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav");
				UTIL_PlaySound( ENT(pev), getSpinUpDownChannel(), "hassault/hw_spindown.wav", 1, ATTN_NORM, 0, 100 );
				spinuptimeremain = -1;
				spinuptime = -1;
				
				//...apparently we are sticking to calling this ACT_WALK now.
				if(m_IdealActivity == ACT_WALK){
					//Spinup finished while walking? We may be able to run now, do a check.
					if(EASY_CVAR_GET_DEBUGONLY(hassaultSpinMovement) != 2){
						this->signalActivityUpdate = TRUE;
						SetActivity(ACT_WALK); //keep it as ACT_WALK regardless or else we'll get hell pretty soon.
					}
				}
		}else{
			//if in the middle of spinning up, forget about the idle spinning.
			if(spinuptimeIdleSoundDelay == -1 || spinuptimeIdleSoundDelay <= gpGlobals->time && EASY_CVAR_GET_DEBUGONLY(hassaultFireSound) != 3){

				//still spinning? play idle spinning sound.
				if(EASY_CVAR_GET_DEBUGONLY(hassaultIdleSpinSound) == 0){
					
				}else if(EASY_CVAR_GET_DEBUGONLY(hassaultIdleSpinSound) == 1){
					if(idleSpinSoundDelay == -1 || idleSpinSoundDelay <= gpGlobals->time){
						idleSpinSoundDelay = gpGlobals->time + 0.214;
					//if(idleSpinSound == FALSE){
						//Uh, wow.  This sound automatically loops, all on its own.  Would not guess this in 1 million years.
						//How in the Hell...             Yea, guess how long it took to figure that $#!% out.
						//Unsure if this can be stopped or interrupted, so still pumping it out as usual.  The "Stop" further below than this is effective.
						UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav");
						UTIL_PlaySound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav", 1, ATTN_NORM, 0, 100 );
					}
					//}
				}else if(EASY_CVAR_GET_DEBUGONLY(hassaultIdleSpinSound) == 2){
					UTIL_PlaySound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav", 1, ATTN_NORM, 0, 100 );
				}else if(EASY_CVAR_GET_DEBUGONLY(hassaultIdleSpinSound) == 3){
					
					if(chainFireSoundDelay <= gpGlobals->time){
						if(idleSpinSoundDelay == -1 || idleSpinSoundDelay <= gpGlobals->time){
							idleSpinSoundDelay = gpGlobals->time + 0.214;
							UTIL_StopSound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav");
							UTIL_PlaySound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav", 1, ATTN_NORM, 0, 100 );
						}
					}
				}else if(EASY_CVAR_GET_DEBUGONLY(hassaultIdleSpinSound) == 4){
					if(chainFireSoundDelay <= gpGlobals->time){
						UTIL_PlaySound(ENT(pev), getIdleSpinChannel(), "hassault/hw_spin.wav", 1, ATTN_NORM, 0, 100 );
					}
				}
			}
		}
	}else{
		attemptStopIdleSpinSound(TRUE);
	}

	if(spinuptimeremain <= gpGlobals->time){
		//hw_gun4.wav?
	}

	CSquadMonster::MonsterThink();
	//easyForcePrintLine("HASSAULT%d seq:%d fra:%.2f", monsterID, pev->sequence, pev->frame);
}


int CHAssault::SquadRecruit( int searchRadius, int maxMembers ){
	return CSquadMonster::SquadRecruit(searchRadius, maxMembers);
}


BOOL CHAssault::usesAdvancedAnimSystem(void){
	return TRUE;
}

int CHAssault::LookupActivityHard(int activity){
	pev->framerate = 1;
	resetEventQueue();

	m_flFramerateSuggestion = 1;
	/*
	m_flFramerateSuggestion = -1;
	//pev->frame = 6;
	return LookupSequence("get_bug");
	*/

	EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "SMOOTH AS %d %d %d", HasConditions(bits_COND_CAN_MELEE_ATTACK1), m_Activity, m_IdealActivity ));
	int iRandChoice = 0;
	int iRandWeightChoice = 0;
	char* animChoiceString = NULL;
	int* weightsAbs = NULL;
	//pev->framerate = 1;
	int maxRandWeight = 30;
	int iSelectedActivity = activity;  //by default the same. Change to affect what animation is picked below.

	//easyForcePrintLine("OH wow son!!! %d %.2f %.2f", m_IdealActivity, spinuptimeremain, gpGlobals->time);
	
	/*
	if(iSelectedActivity == ACT_RUN && spinuptimeremain != -1 && gpGlobals->time <= spinuptimeremain){
		//Let the hassaultSpinMovement tell us which movement anim to use.
		switch( (int)EASY_CVAR_GET_DEBUGONLY(hassaultSpinMovement) ){
			case 0:
				//nothing, stay standing.
				iSelectedActivity = ACT_IDLE;
			break;
			case 1:
				iSelectedActivity = ACT_WALK;
			break;
			case 2:
				iSelectedActivity = ACT_RUN;
			break;
		}//END OF switch
	}
	*/

	BOOL stillSpinning = (spinuptimeremain != -1 && gpGlobals->time <= spinuptimeremain);

	// still spinning and unable to walk? Don't.
	if(iSelectedActivity == ACT_WALK){
		if(stillSpinning){
			// Let the hassaultSpinMovement tell us which movement anim to use.
			switch( (int)EASY_CVAR_GET_DEBUGONLY(hassaultSpinMovement) ){
				case 0:
					//nothing, stay standing.
					iSelectedActivity = ACT_IDLE;
				break;
				case 1:
					iSelectedActivity = ACT_WALK;
				break;
				case 2:
					iSelectedActivity = ACT_RUN;
				break;
			}//END OF switch
		}else{
			// not spinning? go faster all the time.
			iSelectedActivity = ACT_RUN;
		}
	}


	// let's do m_IdealActivity??
	switch(iSelectedActivity){
		case ACT_RANGE_ATTACK2:
			return LookupSequence("throwgrenade");
		break;
		case ACT_IDLE:
			if(
				//(waittime != -1 && waittime > gpGlobals->time) &&
				(
				(spinuptimeremain != -1 && spinuptimeremain > gpGlobals->time) ||
			  (spinuptime != -1 && spinuptime > gpGlobals->time)
			  )
			){
				// spinning up or maintaining spin? Just return the firing anim, the logic will know to freeze the anim at this frame.
				// ...don't trust it, just freeze it now.
				pev->framerate = 0;
				m_flFramerateSuggestion = 0;
				return LookupSequence("attack"); //firing sequence.
			}else{
				//nothing unusual.
				return CBaseAnimating::LookupActivity(iSelectedActivity);
			}
		break;
		
		case ACT_RANGE_ATTACK1:
			if(previousAnimationActivity != ACT_RANGE_ATTACK1){
				previousAnimationActivity = ACT_RANGE_ATTACK1;
				//easyForcePrintLine("ILL lets not say that %d %.2f", previousAnimationActivity, gpGlobals->time);
				fireDelay = gpGlobals->time + 0.07;
			}

			previousAnimationActivity = iSelectedActivity;
			return CBaseAnimating::LookupActivity(iSelectedActivity);
		break;
		case ACT_WALK:
			if(g_iSkillLevel <= SKILL_EASY){
				m_flFramerateSuggestion = 1.07;
			}else if(g_iSkillLevel == SKILL_MEDIUM){
				m_flFramerateSuggestion = 1.13;
			}else{ //if(g_iSkillLevel == SKILL_HARD){
				m_flFramerateSuggestion = 1.24;
			}
			movementBaseFramerate = m_flFramerateSuggestion;
			// the default "power_walk" is fine to use.
			previousAnimationActivity = iSelectedActivity;
			return CBaseAnimating::LookupActivity(iSelectedActivity);
		break;
		case ACT_RUN:
		{
			if(g_iSkillLevel <= SKILL_EASY){
				m_flFramerateSuggestion = 0.75;
			}else if(g_iSkillLevel == SKILL_MEDIUM){
				m_flFramerateSuggestion = 0.82;
			}else{ //if(g_iSkillLevel == SKILL_HARD){
				m_flFramerateSuggestion = 0.90;
			}
			// the default "power_walk" is fine to use.
			previousAnimationActivity = iSelectedActivity;

			// WHY DONT YOU WORK???!
			int testAnim = CBaseAnimating::LookupActivity(iSelectedActivity);
			if(testAnim == -1){
				// just make the crawl faster.

				m_flFramerateSuggestion *= 2.5;
				// all the ways!!!

				this->m_flFrameRate = m_flFramerateSuggestion;
				this->pev->framerate = m_flFrameRate;
				
				movementBaseFramerate = m_flFramerateSuggestion;
				return CBaseAnimating::LookupActivity(ACT_WALK);
			}else{
				// it is fine, we found a ACT_RUN anim from the model. Use it.
				movementBaseFramerate = m_flFramerateSuggestion;
				return testAnim;
			}

			break;
		}
		case ACT_MELEE_ATTACK1:
			EASY_CVAR_PRINTIF_PRE(hassaultPrintout, easyPrintLine( "YEEE"));

			
			m_flFramerateSuggestion = EASY_CVAR_GET_DEBUGONLY(hassaultMeleeAnimSpeedMulti);

			//?
			pev->framerate = EASY_CVAR_GET_DEBUGONLY(hassaultMeleeAnimSpeedMulti);

			//pev->renderfx |= STOPINTR;
			//pev->framerate = 1.2;
			
			/*
			this->animFrameStartSuggestion = 2;
			this->animFrameCutoffSuggestion = 230;  //these are out of 255. 255 is 100%, at the end.
			*/

			this->animationFPSSuggestion = 30;
			//does this even do anything?
			//this->animationFramesSuggestion = 76;

			this->animEventQueuePush(5.0f / 30.0f, 0);
			this->animEventQueuePush(12.0f / 30.0f, 0);
			this->animEventQueuePush(36.7f / 30.0f, 0);
			this->animEventQueuePush(55.0f / 30.0f, 1);


			//this->animEventQueuePush(20.0f / 30.0f, 0);
			//this->animEventQueuePush(31.0f / 30.0f, 0);
			//this->animEventQueuePush(43.0f / 30.0f, 0);
			previousAnimationActivity = iSelectedActivity;
			return LookupSequence("melee1");
		break;
		case ACT_DIEBACKWARD:

			this->animEventQueuePush(10.0f / 30.0f, 3);
			previousAnimationActivity = iSelectedActivity;
			return LookupSequence("die_backwards");
		break;
		case ACT_DIESIMPLE:
			
			this->animEventQueuePush(5.0f / 30.0f, 2);
			this->animEventQueuePush(14.4f / 30.0f, 3);
			previousAnimationActivity = iSelectedActivity;
			return LookupSequence("die_crumple");
		break;
		case ACT_DIEVIOLENT:
		case ACT_DIE_HEADSHOT:

			this->animEventQueuePush(12.4f / 30.0f, 3);
			previousAnimationActivity = iSelectedActivity;
			return LookupSequence("die_violent");
		break;

	}

	previousAnimationActivity = iSelectedActivity;
	//not handled by above?  try the real deal.
	return CBaseAnimating::LookupActivity(iSelectedActivity);
}


int CHAssault::tryActivitySubstitute(int activity){
	//no need for default, just falls back to the normal activity lookup.
	switch(activity){
		
		case ACT_RANGE_ATTACK1:
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_RANGE_ATTACK2:
			return LookupSequence("throwgrenade");
		break;
		case ACT_WALK:
			return CBaseAnimating::LookupActivity(activity);
		break;
		case ACT_RUN:

			//we don't appear to have an ACT_RUN anymore.
			return CBaseAnimating::LookupActivity(ACT_WALK);
		break;
		case ACT_MELEE_ATTACK1:
			return LookupSequence("melee1");
		break;
		case ACT_DIEBACKWARD:
			return LookupSequence("die_backwards");
		break;
		case ACT_DIESIMPLE:
			return LookupSequence("die_crumple");
		break;
		case ACT_DIEVIOLENT:
		case ACT_DIE_HEADSHOT:
			return LookupSequence("die_violent");
		break;
	}
	
	//not handled by above?  No animations.
	//MODDD TODO - would it be safer for all monsters to call "PARENTCLASS::LookupActivity(activity);" instead of giving up this easily? Verify.
	return CBaseAnimating::LookupActivity(activity);
}


BOOL CHAssault::canResetBlend0(void){
	return TRUE;
}

//MODDD TODO - check. Does this get constntly called while the anim is frozen? May want to check this out.
BOOL CHAssault::onResetBlend0(void){
	/*
	//easyForcePrintLine("HOW IT GO   %d", (m_hEnemy!=NULL));
	if (m_hEnemy == NULL)
	{
		return FALSE;
	}

	//NOTICE: may be a good idea to do this first.  ShootAtEnemy may use the global forward vector generated by this.
	UTIL_MakeVectors(pev->angles);

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemyMod( vecShootOrigin );

	//UTIL_MakeVectors ( pev->angles );

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	//easyForcePrintLine("angDir.x %.2f", angDir.x);
	
	SafeSetBlending( 0, angDir.x );
	*/

	Vector vecShootOrigin;
	Vector vecShootDir;
	AimAtEnemy(vecShootOrigin, vecShootDir);
	
	return TRUE;
}

