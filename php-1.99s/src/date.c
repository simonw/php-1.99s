/***[date.c]******************************************************[TAB=4]****\
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
/* $Id: date.c,v 1.6 1996/05/16 15:29:18 rasmus Exp $ */
#include <stdlib.h>
#if TM_IN_SYS_TIME
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <string.h>
#include <php.h>
#include <parse.h>

static char *Months[] = {
	"Jan","Feb","Mar","Apr","May","June","July",
	"Aug","Sept","Oct","Nov","Dec"
};

static char *Days[] = {
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};

/*
 * Date
 * 
 * Function prints out the date and/or time based on the passed
 * format argument.  If type is 0, the local time is used, and if
 * type is 1, Greenwich Mean time is used.
 *
 * Any character in the format string not part of the following
 * will be output without modification:
 *
 *   year     month      day      hour   minute  second
 * Y - 1995  M - Sept  D - Sun   H - 22  i - 05  s - 08
 * y - 95    m - 09    d - 10    h - 10  
 *                     z - (day of year 0-365) 
 *                     U - seconds since Unix Epoch (unix time format)
 */
void Date(int arg, int type) {
	Stack *st;
	struct tm *tm1;
	char *s;
	char *out;
	char temp[32];
	time_t t;
	char *format;
	int y;

	t = time(NULL);
	if(arg) {
		st = Pop();
		if(!st) {
			Error("Stack error in %s expression",type?"gmdate":"date");
			return;
		}
		t = st->intval;
	}			
	st = Pop();
	if(!st) {
		Error("Stack error in %s expression",type?"gmdate":"date");
		return;
	}
	if(st->strval) format = st->strval;
	else {
		Error("No format string specified");
		return;
	}
	out = emalloc(1,sizeof(char) * 10 * (strlen(format)+1));
	*out = '\0';

	if(type==0) tm1 = localtime(&t);
	else tm1 = gmtime(&t);

	s = format;
	while(*s) {
		switch(*s) {
			case 'y':
				y = tm1->tm_year;
				if(y>=100) y-=100;
				sprintf(temp,"%02d",y);
				strcat(out,temp);
				break;
			case 'z':
				sprintf(temp,"%03d",tm1->tm_yday);
				strcat(out,temp);
				break;
			case 'U':
				sprintf(temp,"%ld",(long)t);
				strcat(out,temp);
				break;
			case 'Y':
				sprintf(temp,"%d",1900+tm1->tm_year);
				strcat(out,temp);
				break;
			case 'M':
				strcat(out,Months[tm1->tm_mon]);
				break;	
			case 'm':
				sprintf(temp,"%02d",tm1->tm_mon+1);
				strcat(out,temp);
				break;
			case 'D':
				strcat(out,Days[tm1->tm_wday]);
				break;
			case 'd':
				sprintf(temp,"%02d",tm1->tm_mday);
				strcat(out,temp);
				break;
			case 'H':
				sprintf(temp,"%02d",tm1->tm_hour);
				strcat(out,temp);
				break;
			case 'h':
				sprintf(temp,"%02d",tm1->tm_hour%12);
				strcat(out,temp);
				break;
			case 'i':
				sprintf(temp,"%02d",tm1->tm_min);
				strcat(out,temp);
				break;
			case 's':
				sprintf(temp,"%02d",tm1->tm_sec);
				strcat(out,temp);
				break;
			default:
				sprintf(temp,"%c",*s);
				strcat(out,temp);
				break;
		}
		s++;
	}
	Push(out,STRING);
}

void UnixTime(void) {
	char temp[32];

	sprintf(temp,"%ld",(long)time(NULL));
	Push(temp,LNUMBER);
}

/* arguments: hour minute second month day year */
void MkTime(int args) {
	struct tm tm1;
	struct tm *tm2;
	time_t t;
	int j;
	Stack *s;
	char temp[32];
	int vals[10];

	s = Pop();
	j=0;
	while(args) {
		vals[j++] = s->intval;
		s=Pop();
		args--;
	}	
	t = time(NULL);
	tm2 = localtime(&t);	
	memcpy(&tm1, tm2, sizeof(struct tm));
	tm1.tm_isdst = -1;
	switch(j) {
	case 6:	
		tm1.tm_hour = vals[5];
		tm1.tm_min = vals[4];
		tm1.tm_sec = vals[3];
		tm1.tm_mon = vals[2]-1;
		tm1.tm_mday = vals[1];
		if(vals[0] > 1000) vals[0]-=1900;
		tm1.tm_year = vals[0];
		break;
	case 5:
		tm1.tm_hour = vals[4];
		tm1.tm_min = vals[3];
		tm1.tm_sec = vals[2];
		tm1.tm_mon = vals[1]-1;
		tm1.tm_mday = vals[0];
		break;
	case 4:
		tm1.tm_hour = vals[3];
		tm1.tm_min = vals[2];
		tm1.tm_sec = vals[1];
		tm1.tm_mon = vals[0]-1;
		break;
	case 3:
		tm1.tm_hour = vals[2];
		tm1.tm_min = vals[1];
		tm1.tm_sec = vals[0];
		break;
	case 2:
		tm1.tm_hour = vals[1];
		tm1.tm_min = vals[0];
		break;
	case 1:
		tm1.tm_hour = vals[0];
		break;
	case 0:
		break;
	}
	if(tm1.tm_hour > 24 || tm1.tm_hour < 0) {
		Error("Hour argument to mktime is invalid");
	}
	if(tm1.tm_min > 60 || tm1.tm_min < 0) {
		Error("Minute argument to mktime is invalid");
	}
	if(tm1.tm_sec > 61 || tm1.tm_min < 0) {
		Error("Second argument to mktime is invalid");
	}
	if(tm1.tm_mon > 11 || tm1.tm_min < 0) {
		Error("Month argument to mktime is invalid");
	}
	if(tm1.tm_mday > 31 || tm1.tm_min < 0) {
		Error("Day of month argument to mktime is invalid");
	}
	if(tm1.tm_year > 138 || tm1.tm_year < 70) {
		Error("Calendar times before 00:00:00 UTC, January 1, 1970 or after 03:14:07 UTS, January 19, 2038 cannot be represented by mktime");
	}
	t = mktime(&tm1);
	sprintf(temp,"%ld\n",t);
	Push(temp,LNUMBER);
}
