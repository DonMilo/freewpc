/*
 * Copyright 2009 by Brian Dominy <brian@oddchange.com>
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

#include <freewpc.h>
#include <eb.h>

#define NUM_GANGWAY_AWARDS 6

__local__ U8 gangway_count;

U8 gangway_eb_lit_this_ball;

const score_id_t gangway_scores[] = {
	SC_75K, SC_100K, SC_150K, SC_200K, SC_250K, SC_250K,
};
const struct generic_ladder gangway_score_rule = {
	NUM_GANGWAY_AWARDS,
	gangway_scores,
	&gangway_count,
};


void gangway_collect_deff (void)
{
	seg_alloc_clean ();
	seg_write_row_center (0, "GANGWAY");
	sprintf_score (score_deff_get ());
	seg_write_row_center (1, sprintf_buffer);
	seg_sched_transition (&seg_trans_center_out);
	seg_show ();
	task_sleep_sec (2);
	deff_exit ();
}

void gangway_eb_lit_deff (void)
{
	seg_alloc_clean ();
	seg_write_row_center (0, "GANGWAY");
	seg_write_row_center (1, "EXTRA BALL LIT");
	seg_sched_transition (&seg_trans_center_out);
	seg_show ();
	task_sleep_sec (2);
	deff_exit ();
}

bool gangway_available_p (void)
{
	return !multiball_mode_running_p ();
}

void gangway_loop_lit (void)
{
	sample_start (SND_LOOP, SL_1S);
}

void gangway_loop_collected (void)
{
	if (gangway_count == 5 && !gangway_eb_lit_this_ball)
	{
		gangway_eb_lit_this_ball = TRUE;
		light_hard_extra_ball ();
		deff_start (DEFF_GANGWAY_EB_LIT);
	}
	else
	{
		sample_start (SND_WHEEEE, SL_1S);
		lamp_tristate_on (lamplist_index (LAMPLIST_GANGWAYS, gangway_count));
		generic_ladder_score_and_advance (&gangway_score_rule);
		deff_start (DEFF_GANGWAY_COLLECT);
		lamp_tristate_flash (lamplist_index (LAMPLIST_GANGWAYS, gangway_count));
	}
}


static inline void gangway_shot (task_gid_t gid, task_gid_t other_gid)
{
	score (SC_50K);
	if (gangway_available_p ())
	{
		if (timer_find_gid (gid))
		{
			timer_kill_gid (gid);
			gangway_loop_collected ();
		}
		else
		{
			timer_kill_gid (other_gid);
			timer_start_free (gid, TIME_5S);
			gangway_loop_lit ();
		}
	}
}


static inline void gangway_light (task_gid_t gid, task_gid_t other_gid)
{
	if (gangway_available_p ())
	{
		timer_kill_gid (other_gid);
		timer_restart_free (gid, TIME_5S);
		gangway_loop_lit ();
	}
}

void gangway_init (U8 spots)
{
	lamplist_apply (LAMPLIST_GANGWAYS, lamp_off);
	lamplist_apply (LAMPLIST_GANGWAYS, lamp_flash_off);
	gangway_count = spots;
	while (spots > 0)
	{
		lamplist_build_increment (LAMPLIST_GANGWAYS, lamp_matrix);
		spots--;
	}
	lamp_tristate_flash (lamplist_index (LAMPLIST_GANGWAYS, gangway_count));
}


CALLSET_ENTRY (gangway, left_loop_shot)
{
	gangway_shot (GID_LEFT_GANGWAY_LIT, GID_RIGHT_GANGWAY_LIT);
}

CALLSET_ENTRY (gangway, right_loop_shot)
{
	gangway_shot (GID_RIGHT_GANGWAY_LIT, GID_LEFT_GANGWAY_LIT);
}

CALLSET_ENTRY (gangway, sw_left_inlane)
{
	gangway_light (GID_RIGHT_GANGWAY_LIT, GID_LEFT_GANGWAY_LIT);
}

CALLSET_ENTRY (gangway, sw_inner_right_inlane)
{
	gangway_light (GID_LEFT_GANGWAY_LIT, GID_RIGHT_GANGWAY_LIT);
}

CALLSET_ENTRY (gangway, lamp_update)
{
	lamp_on_if (LM_FLIPPER_LANES, gangway_available_p ());
	lamp_flash_if (LM_LEFT_GANGWAY, timer_find_gid (GID_LEFT_GANGWAY_LIT));
	lamp_flash_if (LM_RIGHT_GANGWAY, timer_find_gid (GID_RIGHT_GANGWAY_LIT));
}

CALLSET_ENTRY (gangway, start_player)
{
	gangway_init (2);
}

CALLSET_ENTRY (gangway, start_ball)
{
	if (gangway_count >= 5)
	{
		gangway_init (0);
	}
	gangway_eb_lit_this_ball = FALSE;
}

