/***[dir.c]*******************************************************[TAB=4]****\
*                                                                            *
* PHP/FI                                                                     *
*                                                                            *
* Copyright 1995,1996 Rasmus Lerdorf                                         *
*                                                                            *
*  This program is free software; you can redistribute it and/or modify      *
*  it under the terms of the GNU General Public License as published by      *
*  the Free Software Foundation; either version 2 of the License, or         *
*  (at your option) any later version.                                       *
*                                                                            *
*  This program is distributed in the hope that it will be useful,           *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*  GNU General Public License for more details.                              *
*                                                                            *
*  You should have received a copy of the GNU General Public License         *
*  along with this program; if not, write to the Free Software               *
*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                 *
*                                                                            *
\****************************************************************************/
/* $Id: dir.c,v 1.3 1996/05/16 15:29:19 rasmus Exp $ */
#include <php.h>
#include <dirent.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <parse.h>

static DIR *dirp=NULL;

void php_init_dir(void) {
	dirp=NULL;
}

void OpenDir(void) {
	Stack *s;

    s = Pop();
    if(!s) {
        Error("Stack error in opendir");
        return;
    }
	if(dirp) closedir(dirp);
	dirp = opendir(s->strval);
	if(!dirp) {
		Error("%d:%s",errno,strerror(errno));
	}
} 

void CloseDir(void) {
	if(dirp) closedir(dirp);
	dirp=NULL;
} 

void ChDir(void) {
	Stack *s;
	int ret;

    s = Pop();
    if(!s) {
        Error("Stack error in chdir");
        return;
    }
	ret = chdir(s->strval);
	if(ret<0) {
		Error("%d:%s",errno,strerror(errno));
	}
	Push("-1",LNUMBER);
}

void RewindDir(void) {
	if(dirp) rewinddir(dirp);
}

void ReadDir(void) {
	static struct dirent *direntp;

	if(!dirp) {
		Error("No current directory pointer - call opendir() first");
		return;
	}
	direntp = readdir(dirp);
	if(direntp) Push(direntp->d_name,STRING);
	else Push("",STRING);
}
