/***[exec.c]******************************************************[TAB=4]****\
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
/* $Id: exec.c,v 1.7 1996/05/30 14:47:36 rasmus Exp $ */
#include <php.h>
#include <parse.h>
#include <ctype.h>
#if APACHE
#include "http_protocol.h"
#endif

/*
 * If type==0, only last line of output is returned
 * If type==1, all lines will be printed and last lined returned
 * If type==2, all lines will be saved to array var on stack
 *
 * name is the name of an array variable in which to store each line
 * of output
 *
 * retname is the name of a variable in which to store the return status
 * of the exec'ed command
 */
void Exec(unsigned char *name, unsigned char *retname, int type) {
	FILE *fp;
	Stack *s;
	char buf[4096];
	int t,l,ret;

	s = Pop();
	if(!s) {
		Error("Stack error in exec");
		return;
	}	
	fp = popen(s->strval,"r");
	if(!fp) {
		Error("Unable to fork [%s]",s->strval);
		return;
	}
	buf[0]='\0';	
	if(type==1) {
		php_header(0,NULL);
	}		
	while(fgets(buf,4095,fp)) {
		if(type==1) PUTS(buf);
		else if(type==2) {
			l = strlen(buf);
			t = l;
			while(isspace(buf[--l])); 
			if(l<t) buf[l+1]='\0';	
			Push(AddSlashes(buf,0),STRING);
			SetVar(name,1,0);	
		}
	}
	l = strlen(buf);
	t = l;
	while(l && isspace(buf[--l])); 
	if(l<t) buf[l+1]='\0';	
	Push(AddSlashes(buf,0),STRING);
	ret = pclose(fp);
	if(retname) {
		sprintf(buf,"%d",ret);
		Push(buf,LNUMBER);
		SetVar(retname,0,0);
	}
}

int php_ind(char *s, char c) {
    register int x;

    for(x=0;s[x];x++)
        if(s[x] == c) return x;

    return -1;
}

void EscapeShellCmd(void) {
    register int x,y,l;
	Stack *s;
	char *cmd;

	s = Pop();
	if(!s) {
		Error("Stack error in EscapeShellCmd");
		return;
	}	
    l=strlen(s->strval);
	cmd = emalloc(1,2*l);
	strcpy(cmd,s->strval);
    for(x=0;cmd[x];x++) {
        if(php_ind("&;`'\"|*?~<>^()[]{}$\\\x0A",cmd[x]) != -1){
            for(y=l+1;y>x;y--)
                cmd[y] = cmd[y-1];
            l++; /* length has been increased */
            cmd[x] = '\\';
            x++; /* skip the character */
        }
    }
	Push(cmd,s->type);
}
