/***[post.c]******************************************************[TAB=4]****\
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
/* $Id: post.c,v 1.6 1996/05/16 15:29:28 rasmus Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <php.h>
#include <parse.h>
#if APACHE
#include "http_protocol.h"
#endif

#define ishex(x) (((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || \
                  ((x) >= 'A' && (x) <= 'F'))

int htoi(unsigned char *s) {
        int     value;
        char    c;

        c = s[0];
        if(isupper(c)) c = tolower(c);
        value=(c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

        c = s[1];
        if(isupper(c)) c = tolower(c);
        value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

        return(value);
}

void parse_url(char *data) {
    char *dest = data;

    while(*data) {
        if(*data=='+') *dest=' ';
        else if(*data== '%' && ishex(*(data+1)) && ishex(*(data+2))) {
            *dest = (unsigned char) htoi(data + 1);
            data+=2;
        } else *dest = *data;
        data++;
        dest++;
    }
    *dest = '\0';
}

/*
 * getpost()
 *
 * This reads the post form data into a string.
 * Remember to free this pointer when done with it.
 */
char *getpost(void) {
	static char *buf=NULL;
	int bytes, length, cnt=0;
#if FILE_UPLOAD
	int file_upload=0;
	char *mb;
	char boundary[100];
#endif

#if APACHE	
/*	buf = php_rqst->content_type; */
	buf = table_get(php_rqst->subprocess_env,"CONTENT_TYPE");
#else
	buf = getenv("CONTENT_TYPE");
#endif
	if(!buf) {
		Error("POST Error: content-type missing");
		return(NULL);
	}
#if FILE_UPLOAD
	if(strncasecmp(buf,"application/x-www-form-urlencoded",33) && strncasecmp(buf,"multipart/form-data",19)) {
#else
	if(strncasecmp(buf,"application/x-www-form-urlencoded",33)) {
#endif
		Error("Unsupported content-type: %s",buf);
		return(NULL);
	}
#if FILE_UPLOAD
	if(!strncasecmp(buf,"multipart/form-data",19)) {
		file_upload=1;
		mb = strchr(buf,'=');
		if(mb) strcpy(boundary,mb+1);	
		else {
			Error("File Upload Error: No MIME boundary found");
			Error("There should have been a \"boundary=something\" in the Content-Type string");
			Error("The Content-Type string was: \"%s\"",buf);
			return(NULL);
		}
	}
#endif

#if APACHE
	buf = table_get(php_rqst->subprocess_env,"CONTENT_LENGTH");
#else
	buf = getenv("CONTENT_LENGTH");
#endif
	if(buf==NULL) {
		Error("POST Error: No Content Length");
		return(NULL);
	}
	
	length = atoi(buf);
	buf = (char *)emalloc(1,(length+1)*sizeof(char));
	if(!buf) {
		Error("Unable to allocate memory in getpost()");
		return(NULL);
	}
#if DEBUG
	Debug("Allocated %d bytes for post buffer\n",length);
#endif
	do {
#if APACHE
		bytes = read_client_block(php_rqst, buf + cnt, length - cnt);
#else
		bytes = fread(buf + cnt, 1, length - cnt, stdin);
#endif
		cnt += bytes;
	} while(bytes && cnt<length);
#if FILE_UPLOAD
	if(file_upload) {
		mime_split(buf,cnt,boundary);
		return(NULL);
	} 
#endif

	buf[cnt]='\0';
	return(buf);
}

/*
 * This function returns 0 if no fields of a submitted form contain
 * any data and 1 otherwise.
 */
int CheckResult(char *res) {
	char *s, *t;
	int done=0;
 
	if(!res) return(0);
	if(strlen(res)<2) return(0);
	t=res;
	while(!done) {
		s=strchr(t,'=');
		if(!s) return(0);
		t=strchr(s,'&');
		if(!t) {
			if(strlen(s)>1) return(1);
			else return(0);
		} else {
			if(t-s>1) return(1);
			t++;
		}
	}
	return(0); /* never reached */
}

/*
 * arg = 0  Post Data
 * arg = 1  Get Data
 */
void TreatData(int arg) {
	char *res, *s, *t, *tt, *u=NULL;
	char *ind, *tmp, *ret;
	char o='\0';
	int itype;
	int inc = 0;
	VarTree *v;

	if(arg==0) res = getpost();
	else {
#if APACHE
		s = php_rqst->args;
#else
		s = getenv("QUERY_STRING");
#endif
		res=s;
		if(s && *s) {
			res = (char *)estrdup(1,s);
		}
		inc = -1;
	}
	if(!(res && *res)) return;
#if DEBUG
	Debug("TreatData: [%s]\n",res);
#endif
	s = strtok(res,"&");
	while(s) {
		t = strchr(s,'=');
		if(t) {
			*t='\0';
			tt = strchr(s,'+');
			while(tt) {
				s = tt+1;
				tt=strchr(s,'+');
			}	
			parse_url(s);
			itype = CheckIdentType(s);
			if(itype==2) {
				ind=GetIdentIndex(s);
				tmp = estrdup(1,ind);
				Push((ret=AddSlashes(tmp,1)),STRING);
			}
			if(itype) {
				u = strchr(s,'[');
				if(u) *u='\0';
			}
			/* 
			 * This check makes sure that a variable which has been
			 * defined through the POST method is not redefined with a
			 * GET method variable.  Allowing this would make it
			 * impossible to write secure PHP Scripts.
			 */
			if(arg==1) {
				v = GetVar(s,NULL,0);
				if(v && v->flag != -1) {
					if(itype==2) Pop();
					if(tt) *tt=o;
					continue;
				}
			}
			parse_url(t+1);
			tmp = estrdup(1,t+1);
			Push((ret=AddSlashes(tmp,1)),CheckType(t+1));
			SetVar(s,itype,inc);
			if(tt) *tt=o;
		}
		s = strtok(NULL,"&");
	}
}	
