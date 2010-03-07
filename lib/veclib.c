#ifndef lint
static char *Module_id = "$Id: veclib.c,v 2.6 2003/02/20 00:56:58 alex Exp $";
#endif

#define	CNULL	(char *)0

static char *next_word();
static int is_sep();

/* FUNCTION vector()                                                */
/* Convert a null-terminated string into a vector of words. Words   */
/* are strings separated by "sepchars". The string is broken up "in */
/* place", and separators are replaced by null characters. The      */
/* vector is constructed in "wstart", which must be at least size   */
/* 'maxent'. The vector is null-terminated, so 'maxent' must        */
/* include space for the terminating null entry.                    */

/* RETURNS the number of words in the vector, including the         */
/* terminating null.                                                */

int
vector(buf,wstart,sepchars,singlesep,maxent)
char *buf,**wstart,*sepchars;
int singlesep,maxent;
{
	char *bptr = buf;
	int pos = 0;

	if(!wstart)
		return(0);
	wstart[0] = bptr;
	if(!buf || (*buf == '\0'))
		return(0);
	bptr = buf;

	/* first word													*/
	wstart[0] = bptr;

	/* next_word() replaces the next separator with a null char     */
	/* and returns the start of the next word (which may be null    */
	/* if there are no more words.                                  */
	while(bptr && (bptr = next_word(wstart[pos],sepchars,singlesep)))
	{
		wstart[++pos] = bptr;
		if(pos >= (maxent - 1))
			break;
	}
	wstart[++pos] = CNULL;
	return(pos);
}


/* FUNCTION next_word()                                             */
/* Finds the beginning of the second "word" pointed to by its       */
/* argument.  A word ends at the first ocurrence of any character   */
/* in sepchars, which is replaced in the string by a null.  The     */
/* start of the next word is the next character not in sepchars.    */

/* RETURNS a pointer to the start of the second word, or CNULL      */
/* if no second word. If 'singlesep' is non-zero, adjacent          */
/* separators surround null "words"; otherwise, contiguous strings  */
/* of sepchars delineate words.                                     */

char *
next_word(string,sepchars,singlesep)
char *string,*sepchars;
int singlesep;
{
	char *x;

	if(string == CNULL)
		return(string);
	x = string;

	while(*x && !is_sep(*x,sepchars))
		++x;

	if(!*x)	/* end of line                                          */
		return(CNULL);

    /* replace separator after first word with null                 */
	*x++ = '\0';

	if(!singlesep)
	{
		/* find and return pointer to the first non-sep char        */
		while(is_sep(*x,sepchars))
			++x;
        if(!*x) /* end of line                                      */
			return(CNULL);
	}
	/* If the current char is a separator (singlesep) it will be    */
	/* replaced by a null next time through.                        */

	return(x);
}

#ifndef is_sep
/* FUNCTION is_sep                                                  */
/* Examines character passed as argument and RETURNS 1 if the       */
/* character is in the string defined by "sepchars", otherwise      */
/* returns 0.  (0 is returned if character is null).                */

int
is_sep(c,sepchars)
char c,*sepchars;
{
	char *tmp = sepchars;

	if(c)
	{
		while(*tmp)
		{
			if(*tmp == c)
				return(1);
			else
				++tmp;
		}
	}
	return(0);
}
#endif

#include <stdio.h>

/* FUNCTION prvector() prints to 'toptr' the null-terminated vector */
/* passed as argument. For debugging only. Prints at most 'nwords'  */
/* words.                                                           */

void
prvector(ioptr,vec,nwords)
FILE * ioptr;
char **vec;
int nwords;
{
	int i = 0;

	fprintf(ioptr,"%d words:\n",nwords);
	if(vec)
	{
		while(*vec && (i < nwords))
			fprintf(ioptr,"\t[%d] \"%s\"\n",i++,*vec++);
	}
	else
		fprintf(ioptr,"no words (nwords = %d\n",nwords);
}
