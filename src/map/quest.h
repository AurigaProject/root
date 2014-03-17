/*
 * Copyright (C) 2002-2007  Auriga
 *
 * This file is part of Auriga.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef _QUEST_H_
#define _QUEST_H_

// �N�G�X�g�f�[�^�x�[�X
struct quest_db {
	int nameid;
	unsigned int limit;
	struct {
		short id;
		short count;
	} mob[3];
};
extern struct quest_db quest_db[MAX_QUEST_DB];

int quest_search_db(int quest_id);
int quest_search_index(struct map_session_data *sd, int quest_id);
struct quest_data *quest_get_data(struct map_session_data *sd, int quest_id);
int quest_addlist(struct map_session_data *sd, int quest_id);
int quest_updatelist(struct map_session_data *sd, int old_id, int new_id);
int quest_dellist(struct map_session_data *sd, int quest_id);
int quest_killcount(struct map_session_data *sd, int mob_id);

void quest_reload(void);
int do_init_quest(void);

#endif	// _QUEST_H_