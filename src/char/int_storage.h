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

#ifndef _INT_STORAGE_H_
#define _INT_STORAGE_H_

extern struct dbt *gstorage_db;

int inter_storage_parse_frommap(int fd);
int inter_storage_init(void);

#ifdef TXT_ONLY
	#include "storagedb_txt.h"
#else
	#include "storagedb_sql.h"
#endif

#endif /* _INT_STORAGE_H_ */
