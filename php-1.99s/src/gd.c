/***[gd.c]********************************************************[TAB=4]****\
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
/* $Id: gd.c,v 1.6 1996/05/16 15:29:21 rasmus Exp $ */
/* gd 1.2 is copyright 1994, 1995, Quest Protein Database Center, 
   Cold Spring Harbor Labs. */

/* Note that there is no code from the gd package in this file */

#include <php.h>
#include <parse.h>
#if HAVE_LIBGD
#include <gd.h>
#include <gdfontt.h>  /* 1 Tiny font */
#include <gdfonts.h>  /* 2 Small font */
#include <gdfontmb.h> /* 3 Medium bold font */
#include <gdfontl.h>  /* 4 Large font */
#include <gdfontg.h>  /* 5 Giant font */

#define PolyMaxPoints 256

typedef struct ImageList {
    gdImagePtr img;
    int ind;
    struct ImageList *next;
} ImageList;

static ImageList *image_top=NULL;
static int gd_ind=0;

void php_init_gd(void) {
	image_top=NULL;
	gd_ind=0;
}

int add_image(gdImagePtr img) {
	ImageList *new;
 
	new = image_top;
	if(!new) {
		new = (ImageList *)emalloc(0,sizeof(ImageList));
		image_top = new;
	} else {
		while(new->next) new=new->next;
		new->next = (ImageList *)emalloc(0,sizeof(ImageList));
		new = new->next;
	}
	new->img    = img;
	new->ind    = gd_ind++;
	new->next   = NULL;
	return(gd_ind-1);
}

gdImagePtr get_image(int count) {
	ImageList *new;
 
	new = image_top;
	while(new) {
#if DEBUG
		Debug("Scanning ImageList: ind = %d\n",new->ind);
#endif
		if(new->ind == count) return(new->img);
		new=new->next;
	}
	return(NULL);
}

void del_image(int count) {
	ImageList *new, *prev, *next;
 
	prev=NULL;
	new = image_top;
	while(new) {
		next = new->next;
		if(new->ind == count) {
			gdImageDestroy(new->img);
			if(prev) prev->next = next;
			else image_top = next;
			break;
		}
		prev=new;
		new=next;
	}
}
#endif

void ImageCreate(void) {
#if HAVE_LIBGD
	Stack *s;
	int dx, dy, ind;
	gdImagePtr im;
	char temp[8];

	s = Pop();
	if(!s) {
		Error("Stack error in imagecreate");
		return;
	}
	dy = s->intval;

	s = Pop();
	if(!s) {
		Error("Stack error in imagecreate");
		return;
	}
	dx = s->intval;

	im = gdImageCreate(dx,dy);
	ind = add_image(im);		

	sprintf(temp,"%d",ind);
	Push(temp,LNUMBER);
#else
	Pop();
	Pop();
	Error("No GD support available");
	Push("-1",LNUMBER);
#endif
}

void ImageCreateFromGif(void) {
#if HAVE_LIBGD
	Stack *s;
	int ind;
	gdImagePtr im;
	char temp[8];
	char *fn=NULL;
	FILE *fp;

	s = Pop();
	if(!s) {
		Error("Stack error in imagecreatefromgif");
		return;
	}
	if(s->strval) {
		fn = (char *) estrdup(1,s->strval);
	} else {
		Error("Invalid filename in imagecreatefromgif");
		return;
	}

	fp = fopen(fn,"r");
	if(!fp) {
		Error("Unable to open %s to read gif file",fn);
		return;
	}

	
	im = gdImageCreateFromGif(fp);

	fflush(fp);
	fclose(fp);

	ind = add_image(im);

	sprintf(temp,"%d",ind);
	Push(temp,LNUMBER);
#else
	Pop();
	Error("No GD support available");
	Push("-1",LNUMBER);
#endif
}

void ImageDestroy(void) {
#if HAVE_LIBGD
	Stack *s;
	int ind;

	s = Pop();
	if(!s) {
		Error("Stack error in imagedestroy");
		return;
	}
	ind = s->intval;
	del_image(ind);
#else
	Pop();
	Error("No GD support available");
#endif
}

void ImageColorAllocate(void) {
#if HAVE_LIBGD
	Stack *s;
	int ind;
	int col;
	int r, g, b;
	char temp[8];
	gdImagePtr im;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecolorallocate");
        return;
    }
	b = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecolorallocate");
        return;
    }
	g = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecolorallocate");
        return;
    }
	r = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecolorallocate");
        return;
    }
	ind = s->intval;

	im = get_image(ind);	
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}
	col = gdImageColorAllocate(im, r, g, b);
	sprintf(temp,"%d",col);
	Push(temp,LNUMBER);
#else
	Pop();
	Pop();
	Pop();
	Pop();
	Error("No GD support available");
	Push("-1",LNUMBER);
#endif
}

void ImageGif(int args) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	char *fn=NULL;
	FILE *fp;
	
	if(args==2) {
		s = Pop();
    	if(!s) {
        	Error("Stack error in imagegif");
        	return;
    	}
		if(s->strval) {
			fn = (char *) estrdup(1,s->strval);
		} else {
			Error("Invalid filename in imagegif");
			return;
		}
	}

	s = Pop();
    if(!s) {
        Error("Stack error in imagegif");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}
	if(args==2) {
		fp = fopen(fn,"w");
		if(!fp) {
			Error("Unable to open %s to create gif file",fn);
			return;
		}
		gdImageGif(im,fp);
		fflush(fp);
		fclose(fp);
	} else {
		gdImageGif(im,stdout);
		fflush(stdout);
	}
#else
	Pop();
	Error("No GD support available");
#endif
}

void ImageSetPixel(void) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	int col, y, x;

	s = Pop();
    if(!s) {
        Error("Stack error in imagesetpixel");
        return;
    }
	col = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagesetpixel");
        return;
    }
	y = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagesetpixel");
        return;
    }
	x = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagesetpixel");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}

	gdImageSetPixel(im,x,y,col);
#else
	Pop();
	Pop();
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

void ImageLine(void) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	int col, y2, x2, y1, x1;

	s = Pop();
    if(!s) {
        Error("Stack error in imageline");
        return;
    }
	col = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imageline");
        return;
    }
	y2 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imageline");
        return;
    }
	x2 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imageline");
        return;
    }
	y1 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imageline");
        return;
    }
	x1 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imageline");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}

	gdImageLine(im,x1,y1,x2,y2,col);
#else
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

void ImageRectangle(void) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	int col, y2, x2, y1, x1;

	s = Pop();
    if(!s) {
        Error("Stack error in imagerectangle");
        return;
    }
	col = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagerectangle");
        return;
    }
	y2 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagerectangle");
        return;
    }
	x2 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagerectangle");
        return;
    }
	y1 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagerectangle");
        return;
    }
	x1 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagerectangle");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}

	gdImageRectangle(im,x1,y1,x2,y2,col);
#else
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

void ImageFilledRectangle(void) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	int col, y2, x2, y1, x1;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefilledrectangle");
        return;
    }
	col = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefilledrectangle");
        return;
    }
	y2 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefilledrectangle");
        return;
    }
	x2 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefilledrectangle");
        return;
    }
	y1 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefilledrectangle");
        return;
    }
	x1 = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefilledrectangle");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}

	gdImageFilledRectangle(im,x1,y1,x2,y2,col);
#else
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

void ImageArc(void) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	int col, e, st, h, w, cy, cx;

	s = Pop();
    if(!s) {
        Error("Stack error in imagearc");
        return;
    }
	col = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagearc");
        return;
    }
	e = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagearc");
        return;
    }
	st = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagearc");
        return;
    }
	h = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagearc");
        return;
    }
	w = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagearc");
        return;
    }
	cy = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagearc");
        return;
    }
	cx = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagearc");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}

	gdImageArc(im,cx,cy,w,h,st,e,col);
#else
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

void ImageFillToBorder(void) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	int col, border, y, x;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefilltoborder");
        return;
    }
	col = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefilltoborder");
        return;
    }
	border = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefilltoborder");
        return;
    }
	y = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefilltoborder");
        return;
    }
	x = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagesetpixel");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}

	gdImageFillToBorder(im,x,y,border,col);
#else
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

void ImageFill(void) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	int col, y, x;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefill");
        return;
    }
	col = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefill");
        return;
    }
	y = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefill");
        return;
    }
	x = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagefill");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}

	gdImageFill(im,x,y,col);
#else
	Pop();
	Pop();
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

void ImageColorTransparent(void) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	int col;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecolortransparent");
        return;
    }
	col = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecolortransparent");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}

	gdImageColorTransparent(im,col);
#else
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

void ImageInterlace(void) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	int interlace;

	s = Pop();
    if(!s) {
        Error("Stack error in imageinterlace");
        return;
    }
	interlace = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imageinterlace");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}

	gdImageInterlace(im,interlace);
#else
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

/* arg = 0  normal polygon
   arg = 1  filled polygon */
void ImagePolygon(int arg) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	gdPoint points[PolyMaxPoints];	
	int i, col, ptotal;
	VarTree *var;

	s = Pop();
    if(!s) {
        Error("Stack error in imagepolygon");
        return;
    }
	col = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagepolygon");
        return;
    }
	ptotal = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagepolygon");
        return;
    }
	var = s->var;
	if(!var) {
		Error("You must pass an array to the imagepolygon function");
		return;
	}
	if(var->count < 6) {
		Error("You must have at least 3 points in your array");
		return;
	}
	if(var->count < ptotal*2) {
		Error("Trying to use %d points in array with only %d points",ptotal,var->count/2);
		return;
	}

	s = Pop();
    if(!s) {
        Error("Stack error in imagepolygon");
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}
	for(i=0;i<ptotal;i++) {
		if(!var) break;
		points[i].x = var->intval;
		var = var->next;
		if(!var) break;
		points[i].y = var->intval;
		var = var->next;
	}
	if(arg==0) gdImagePolygon(im,points,ptotal,col);
	else if(arg==1) gdImageFilledPolygon(im,points,ptotal,col);
#else
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

/*
 * arg = 0  ImageChar
 * arg = 1  ImageCharUp
 * arg = 2  ImageString
 * arg = 3  ImageStringUp
 */
void ImageChar(int arg) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im;
	int ch=0, col, x, y, size;
	char *string=NULL;
	char fun[16];

	switch(arg) {
	case 0: strcpy(fun,"imagechar");
			break;
	case 1: strcpy(fun,"imagecharup");
			break;
	case 2: strcpy(fun,"imagestring");
			break;
	case 3: strcpy(fun,"imagestringup");
			break;
	}
	s = Pop();
    if(!s) {
        Error("Stack error in %s",fun);
        return;
    }
	col = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in %s",fun);
        return;
    }
	if(arg<2) ch = *(s->strval);
	else string = (char *) estrdup(1,s->strval);

	s = Pop();
    if(!s) {
        Error("Stack error in %s",fun);
        return;
    }
	y = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in %s",fun);
        return;
    }
	x = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in %s",fun);
        return;
    }
	size = s->intval;

	s = Pop();
    if(!s) {
       	Error("Stack error in %s",fun);
        return;
    }
	im = get_image(s->intval);
	if(!im) {
		Error("Unable to find image pointer");
		return;
	}
	
	switch(size) {
	case 1:
		switch(arg) {
		case 0:
			gdImageChar(im,gdFontTiny,x,y,ch,col);
			break;
		case 1:
			gdImageCharUp(im,gdFontTiny,x,y,ch,col);
			break;
		case 2:
			gdImageString(im,gdFontTiny,x,y,string,col);
			break;
		case 3:
			gdImageStringUp(im,gdFontTiny,x,y,string,col);
			break;
		}
		break;
	case 2:
		switch(arg) {
		case 0:
			gdImageChar(im,gdFontSmall,x,y,ch,col);
			break;
		case 1:
			gdImageCharUp(im,gdFontSmall,x,y,ch,col);
			break;
		case 2:
			gdImageString(im,gdFontSmall,x,y,string,col);
			break;
		case 3:
			gdImageStringUp(im,gdFontSmall,x,y,string,col);
			break;
		}
		break;
	case 3:
		switch(arg) {
		case 0:
			gdImageChar(im,gdFontMediumBold,x,y,ch,col);
			break;
		case 1:
			gdImageCharUp(im,gdFontMediumBold,x,y,ch,col);
			break;
		case 2:
			gdImageString(im,gdFontMediumBold,x,y,string,col);
			break;
		case 3:
			gdImageStringUp(im,gdFontMediumBold,x,y,string,col);
			break;
		}
		break;
	case 4:
		switch(arg) {
		case 0:
			gdImageChar(im,gdFontLarge,x,y,ch,col);
			break;
		case 1:
			gdImageCharUp(im,gdFontLarge,x,y,ch,col);
			break;
		case 2:
			gdImageString(im,gdFontLarge,x,y,string,col);
			break;
		case 3:
			gdImageStringUp(im,gdFontLarge,x,y,string,col);
			break;
		}
		break;
	case 5:
		switch(arg) {
		case 0:
			gdImageChar(im,gdFontGiant,x,y,ch,col);
			break;
		case 1:
			gdImageCharUp(im,gdFontGiant,x,y,ch,col);
			break;
		case 2:
			gdImageString(im,gdFontGiant,x,y,string,col);
			break;
		case 3:
			gdImageStringUp(im,gdFontGiant,x,y,string,col);
			break;
		}
		break;
	default:
		if(size<1) gdImageChar(im,gdFontTiny,x,y,ch,col);
		else gdImageChar(im,gdFontGiant,x,y,ch,col);
		break;
	}
#else
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	

void ImageCopyResized(void) {
#if HAVE_LIBGD
	Stack *s;
	gdImagePtr im_dst;
	gdImagePtr im_src;
	int srcH, srcW, dstH, dstW, srcY, srcX, dstY, dstX;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecopyresized");
        return;
    }
	srcH = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecopyresized");
        return;
    }
	srcW = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecopyresized");
        return;
    }
	dstH = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecopyresized");
        return;
    }
	dstW = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecopyresized");
        return;
    }
	srcY = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecopyresized");
        return;
    }
	srcX = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecopyresized");
        return;
    }
	dstY = s->intval;
	
	s = Pop();
    if(!s) {
        Error("Stack error in imagecopyresized");
        return;
    }
	dstX = s->intval;

	s = Pop();
    if(!s) {
        Error("Stack error in imagecopyresized");
        return;
    }
	im_src = get_image(s->intval);
	if(!im_src) {
		Error("Unable to find image pointer");
		return;
	}

	s = Pop();
    if(!s) {
        Error("Stack error in imagecopyresized");
        return;
    }
	im_dst = get_image(s->intval);
	if(!im_dst) {
		Error("Unable to find image pointer");
		return;
	}

	gdImageCopyResized(im_dst,im_src,dstX,dstY,srcX,srcY,
	  dstW,dstH,srcW,srcH);
#else
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Pop();
	Error("No GD support available");
#endif
}	
