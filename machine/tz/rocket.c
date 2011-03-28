/*
 * Copyright 2010 by Brian Dominy <brian@oddchange.com>
 *
 * This file is part of FreeWPC.
 *
 * FreeWPC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * FreeWPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FreeWPC; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

//TODO Make this play nice with Skill shot animation
/* CALLSET_SECTION (rocket, __machine2__) */
#include <freewpc.h>

extern bool skill_shot_enabled;

CALLSET_ENTRY (rocket, ball_search)
{
	if (switch_poll_logical (SW_ROCKET_KICKER))
	{
		sol_request (SOL_ROCKET_KICKER);
	}
}

/* Rocket animation contributed by highrise */
void rocket_deff (void)
{
	/* Show loading frames and wait for kick */
	U16 fno;
	for (fno = IMG_ROCKET_LOAD_START; fno <= IMG_ROCKET_LOAD_END; fno += 2)
	{
		//dmd_alloc_pair ();
		dmd_map_overlay ();
		dmd_text_outline ();
		dmd_alloc_pair ();
		frame_draw (fno);
		//dmd_overlay_onto_color ();
		dmd_overlay_outline ();
		dmd_show2 ();
		task_sleep (TIME_33MS);
	//	dmd_map_overlay ();
	}
	task_sleep (TIME_100MS);
	/* Rocket takes 500ms before kick 
	 * load animation takes 400ms */
	/* Launch rocket */
	for (fno = IMG_NEWROCKET_START; fno <= IMG_NEWROCKET_END; fno += 2)
	{
		dmd_alloc_pair_clean ();
	//	dmd_map_overlay ();
	//	dmd_text_outline ();
	//	dmd_alloc_pair ();
		
		frame_draw (fno);
	//	dmd_overlay_outline ();
		dmd_show2 ();
		task_sleep (TIME_33MS);
	}
	deff_exit ();
}

static void rocket_kick_sound (void)
{
	event_can_follow (rocket, flipper, TIME_100MS);
	sound_send (SND_ROCKET_KICK_DONE);
	flasher_pulse (FLASH_UR_FLIPPER);
	task_exit ();
}

CALLSET_ENTRY (rocket, dev_rocket_enter)
{
		if (skill_shot_enabled)
		{
			callset_invoke (skill_missed);
		}
}

/* Give the player 1M if he hits the right flipper on
 * the rocket launch */
CALLSET_ENTRY (rocket, sw_right_button)
{
	if (event_did_follow (rocket, flipper))
	{
		sound_send (SND_CUCKOO);
		score (SC_1M);
	}
}

CALLSET_ENTRY (rocket, dev_rocket_kick_attempt)
{
	if (in_live_game)
	{
		/* Small sleep to make sure skill code gets a chance to
		 * execute */
		task_sleep (TIME_33MS);
		if (!multi_ball_play ())
			leff_start (LEFF_ROCKET);
		sound_send (SND_ROCKET_KICK_REVVING);
		deff_start (DEFF_ROCKET);
		task_sleep (TIME_500MS);
		task_create_gid (0, rocket_kick_sound);
	}
}
