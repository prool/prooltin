/* Prool foolish translator (English to Russian interpreter) and others Prool's functions

   Prool here: http://prool.kharkov.org http://mud.kharkov.org https://github.com/prool/virtustan

   E-mail proolix BIGLAZYDOG gmail . com

*/
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <sys/ioctl.h>
#include <iconv.h>

#include "tintin.h"
#include "prool.h"

#define MAXBUF 4096
#define MAXWORD 200
#define MAXWORDLEN 60

#define TOTAL_LOG_FILENAME "prooltin.log"

int tron;
int total_log;
int coder;
long int start_time;
long int startwatchtime;
int prool_loop_counter;
long int watchdog;

char English [MAXWORD] [MAXWORDLEN];
char Russian [MAXWORD] [MAXWORDLEN];

char buffer [MAXBUF];
char recoded_str [MAXBUF];
char clipboard [MAXBUF];

void prool_ident(void)
{
printf("Mod by Prool. 2014-2018. https://github.com/prool/prooltin http://prool.kharkov.org proolix@gmail.com\n");
}

#define MAX_SOCK_BUF 1024

void koi_to_utf8(char *str_i, char *str_o)
{
	iconv_t cd;
	size_t len_i, len_o = MAX_SOCK_BUF * 6;
	size_t i;

	if ((cd = iconv_open("UTF-8","KOI8-R")) == (iconv_t) - 1)
	{
		printf("koi_to_utf8: iconv_open error\n");
		return;
	}
	len_i = strlen(str_i);
	if ((i = iconv(cd, &str_i, &len_i, &str_o, &len_o)) == (size_t) - 1)
	{
		printf("koi_to_utf8: iconv error\n");
		return;
	}
	*str_o = 0;
	if (iconv_close(cd) == -1)
	{
		printf("koi_to_utf8: iconv_close error\n");
		return;
	}
}

void utf8_to_koi(char *str_i, char *str_o)
{
	iconv_t cd;
	size_t len_i, len_o = MAX_SOCK_BUF * 6;
	size_t i;

	if ((cd = iconv_open("KOI8-R", "UTF-8")) == (iconv_t) - 1)
	{
		printf("utf8_to_koi: iconv_open error\n");
		return;
	}
	len_i = strlen(str_i);
	if ((i=iconv(cd, &str_i, &len_i, &str_o, &len_o)) == (size_t) - 1)
	{
		printf("utf8_to_koi: iconv error\n");
		// return;
	}
	if (iconv_close(cd) == -1)
	{
		printf("utf8_to_koi: iconv_close error\n");
		return;
	}
}

DO_COMMAND(do_proolcoder_switch)
{
if (coder) coder=0;
else coder=1;
return ses;
}

void do_coder(char *s_i, char *s_o)
{
koi_to_utf8(s_i,s_o);
}

char *prooltran(char *si0)
{char *pp, *p0;
int ii;
char *si;

si=si0;

if (coder)
	{
	do_coder(si,recoded_str);
	si=recoded_str;
	}

if (total_log) prool_log(si);

if (tron==0) return si;

p0=buffer;
strcpy(buffer,si);

for(ii=0;ii<MAXWORD;ii++)
{// цикл по всем словам
if (English[ii][0]==0) continue;
	while(1) // многократная замена одного слова
	{
	pp=strstr(buffer,English[ii]);
	if (pp==0) break;
	if (pp!=p0) {memcpy(clipboard,buffer,(pp-p0)); clipboard[pp-p0]=0;}
	else clipboard[0]=0;
	//printf("clipboard=%s\n",clipboard);
	strcat(clipboard,Russian[ii]);
	//printf("clipboard=%s\n",clipboard);
	strcat(clipboard,buffer+(pp-p0)+strlen(English[ii]));
	//printf("clipboard=%s\n",clipboard);
	strcpy(buffer,clipboard);
	}
}

/*
if (pp)
	{
	if (pp!=p0) {memcpy(buffer,si,(pp-p0)); buffer[pp-p0]=0;}
	else buffer[0]=0;
	strcat(buffer,"Пруль");
	strcat(buffer,si+(pp-p0)+strlen("prool"));
	}
else	strcpy(buffer,si);
*/

//printf("buffer=%s\n", buffer);
return buffer;
}

char *ptime(void) // Возвращаемое значение: ссылка на текстовую строку с текущим временем
	{
	char *tmstr;
	time_t mytime;

	mytime = time(0);

	tmstr = (char *) asctime(localtime(&mytime));
	*(tmstr + strlen(tmstr) - 1) = '\0';

	return tmstr+4;

	}

void prool_log(char *message)
{
FILE *fp;

fp=fopen(TOTAL_LOG_FILENAME,"a");
if (fp==NULL) {printf("prooltin: can't open log\n"); total_log=0; return;}
fprintf(fp,"%i %s %s\r\n",getpid(),ptime(),message);
fclose(fp);
}

void uptime_(void)
{long uptime; char *tmstr;
uptime=time(0)-start_time;
if (uptime<=60)printf("Uptime %li sec", uptime);
else
	{
	uptime=uptime/60;
	if (uptime<=60) printf ("Uptime %li min", uptime);
	else
		{
		uptime=uptime/60;
		if (uptime<=24) printf("Uptime %li hour", uptime);
		else printf("Uptime %li days", uptime/24);
		}
	}
	tmstr = asctime(localtime(&start_time));
	*(tmstr + strlen(tmstr) - 1) = '\0';
printf(" since %s\n", tmstr);
}

void prooltranslate_init(int greeting)
{FILE *fp;
char buf [MAXBUF];
char buf2 [MAXBUF];
char *cc;
int i,j;

for (i=0;i<MAXWORD;i++)
	{
	English[0][0] = 0;
	Russian[0][0] = 0;
	}

fp=fopen("slovarb.csv","r");
if (fp==NULL) {if (greeting!=FALSE)printf("Can't open Slovarb\n"); return;}
j=0;
while(!feof(fp))
	{
	buf[0]=0;
	fgets(buf,MAXBUF,fp);
	cc=strchr(buf,'\n');
	if (cc) *cc=0;
	if (buf[0])
		{
		if (greeting!=FALSE)printf("'%s' ", buf);
		cc=strchr(buf,',');
		if (cc==0) continue;
		strcpy(buf2,cc+1);
		*cc=0;
		if (greeting!=FALSE)printf("1 '%s' [%i] 2 '%s' [%i]\n", buf, strlen(buf), buf2, strlen(buf2));
		if ((strlen(buf)>=MAXWORDLEN) || (strlen(buf2)>=MAXWORDLEN))
			{
			printf("Word length overflow. Max len=%i\n",MAXWORDLEN);
			prool_log("Word length overflow");
			break;
			}
		strncpy(English[j],buf,MAXWORDLEN);
		strncpy(Russian[j],buf2,MAXWORDLEN);
		if (++j>=MAXWORD)
			{
			printf("Dictionary overflow!\n");
			prool_log("Dictionary overflow!");
			break;
			}
		}
	}

for (i=0;i<MAXWORD;i++)
	{
	if (English[i][0]==0) break;
	if (greeting!=FALSE)printf("%i) %s %s ", i, English[i], Russian[i]);
	}

fclose(fp);
}

DO_COMMAND(do_prool)
{
struct winsize w;

prool_ident();

printf("\nCompile date %s %s\nCurrent date %s\n\nprool's remarks:\n\
Command for MSSP:\n#config {debug telnet} on\n\
\n\
Prool command\n\
#prool - prool help\n\
#tron - enable foolish translator\n\
#troff - disable foolish translator\n\
#totalon - enable total logging\n\
#totaloff - disable total logging\n\
#listdic - list of dictionary\n\
#writedic - write dictionary to file\n\
#addword english,russian - add word pair to dic\n\
#delword english - del word from dic\n\
#proolwatchdogtimer [n] - set watchdog to n seconds\n\
#proolcoder - set text recoding koi8-r (server) <-> UTF-8 (client)\n\
\n\
Experimental cmds:\n\
#mouseon and #mouseoff\n\
",__DATE__,__TIME__,ptime());

printf("Translator = %i\n", tron);
printf("Total log = %i \"%s\"\n", total_log, TOTAL_LOG_FILENAME);
printf("PID = %i\n", getpid());
printf("prool loop counter = %i\n", prool_loop_counter);
printf("watchdog = %li\n", watchdog);
printf("coder = %i\n", coder);

printf("arg='%s'\n", arg);

ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
printf("Window size: lines %i, columns %i\n",w.ws_row,w.ws_col);

return ses;
}

DO_COMMAND(do_totalon)
{
total_log=1;
prool_log("Total log enabled");
printf("Total log enabled\n");
return ses;
}

DO_COMMAND(do_totaloff)
{
prool_log("Total log disabled");
printf("Total log disabled\n");
total_log=0;
return ses;
}

DO_COMMAND(do_tron)
{
tron=1;
printf("Translator enabled\n");
return ses;
}

DO_COMMAND(do_troff)
{
tron=0;
printf("Translator disabled\n");
return ses;
}

DO_COMMAND(do_addword)
{
char buf[MAXBUF];
char buf2[MAXBUF];
char *cc;
int i;

//printf("arg=%s\n", arg);

strncpy(buf,arg,MAXBUF);

cc=strchr(buf,',');
if (cc==0) {printf("ERROR: No comma\n"); return ses;}
strcpy(buf2,cc+1);
*cc=0;
printf("addword 1 '%s' 2 '%s'\n", buf, buf2);

if ((strlen(buf)>=MAXWORDLEN) || (strlen(buf2)>=MAXWORDLEN))
			{
			printf("addword: Word length overflow\n");
			prool_log("addword: Word length overflow");
			return ses;
			}

for(i=0;i<MAXWORD;i++)
	{
	if (English[i][0]==0)
		{
		strcpy(English[i],buf);
		strcpy(Russian[i],buf2);
		return ses;
		}
	}

printf("ERROR: addword: word overflow\n");
prool_log("ERROR: addword: word overflow");

return ses;
}

DO_COMMAND(do_delword)
{int i;
	if (*arg==0) {
		printf("usage: #delword word\n");
		return ses;
	}

	for (i=0;i<MAXWORD;i++)
	{
		if (!strcmp(English[i],arg))
		{
			English[i][0]=0;
			printf("Found and delete\n");
			return ses;
		}
	}
printf("Word '%s' not found\n", arg);
return ses;
}

DO_COMMAND(do_listdic)
{
	int i,count;

	count=0;
	for (i=0;i<MAXWORD;i++) {
		if (English[i][0]) {printf("%s,%s\n",English[i],Russian[i]); count++;}
	}

	printf("Total words %i\n", count);

	return ses;
}

DO_COMMAND(do_writedic)
{
	int i,count;
	FILE *fp;

	count=0;
	fp=fopen("slovarb2.csv","w");
	if (fp==NULL) {printf("writedic: can't open file\n"); return ses;}
	for (i=0;i<MAXWORD;i++) {
		if (English[i][0]) {fprintf(fp,"%s,%s\n",English[i],Russian[i]); count++;}
	}
	fclose(fp);

	printf("Total words %i\n", count);

	return ses;
}

DO_COMMAND(do_proolwatchdogtimer)
{
watchdog=atoi(arg);

printf("watchdog set to %li sec\n", watchdog);
prool_log("proolwatchdog set");

if (watchdog) startwatchtime=time(0);

return ses;
}

int prool_loop(void)
{
long int i;
prool_loop_counter++;
if (watchdog)
	{
	i=time(0);
	if ((i-startwatchtime)>watchdog)
		{
		printf("proolwatchdog bzzzzz!!!\n");
		prool_log("proolwatchdog bzzzzz!!!");
		watchdog=0;
		printf("tintin++ quit\n");
		prool_log("tintin++ quit");
		do_zap(0,0);
		printf("zapped\n");
		}
	}
return 0;
}

#define ESC 033
#define ESC_STR "\033"

void enable_mouse(void)
{
#if 1 // mouse code from midnight commander
//    case MOUSE_XTERM_NORMAL_TRACKING:
        /* save old highlight mouse tracking */
        printf (ESC_STR "[?1001s");

        /* enable mouse tracking */
        printf (ESC_STR "[?1000h");

        /* enable SGR extended mouse reporting */
//        printf (ESC_STR "[?1006h");

        fflush (stdout);

//    case MOUSE_XTERM_BUTTON_EVENT_TRACKING:
        /* save old highlight mouse tracking */
        printf (ESC_STR "[?1001s");

        /* enable mouse tracking */
        printf (ESC_STR "[?1002h");

        /* enable SGR extended mouse reporting */
//        printf (ESC_STR "[?1006h");

        fflush (stdout);
#endif
}

void disable_mouse(void)
{

#if 1 // mouse code from midnight commander

// case MOUSE_XTERM_NORMAL_TRACKING:
        /* disable SGR extended mouse reporting */
        printf (ESC_STR "[?1006l");

        /* disable mouse tracking */
        printf (ESC_STR "[?1000l");

        /* restore old highlight mouse tracking */
        printf (ESC_STR "[?1001r");

        fflush (stdout);
//    case MOUSE_XTERM_BUTTON_EVENT_TRACKING:
        /* disable SGR extended mouse reporting */
        printf (ESC_STR "[?1006l");

        /* disable mouse tracking */
        printf (ESC_STR "[?1002l");

        /* restore old highlight mouse tracking */
        printf (ESC_STR "[?1001r");

        fflush (stdout);
#endif
}

DO_COMMAND(do_mouseon)
{
	enable_mouse();
	printf("Mouse on\n");
return ses;
}

DO_COMMAND(do_mouseoff)
{
	disable_mouse();
	printf("Mouse off\n");
return ses;
}
