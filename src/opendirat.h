/* Multi Monitor Plugin for Xfce
 *  Copyright (C) 2007 Yuuki Harano
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef OPENDIRAT_H__INCLUDED
#define OPENDIRAT_H__INCLUDED

#include "../config.h"
#include <stdio.h>
#include <dirent.h>

DIR *opendirat(int dirfd, const char *name);
FILE *fopenat(int dirfd, const char *name);
int open_dir(const char *name);
int openat_dir(int dirfd, const char *name);

#endif	/* ifndef OPENDIRAT_H__INCLUDED */