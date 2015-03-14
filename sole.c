// sole     - Small OutLine Editor  sole007.c
// license  : GPL www.gnu.org
// date     : Oct-Nov 1999
// author   : Lucas Gonzalez
// email    : lucas@comtf.es
// url      : www.gulic.org/copensar/index.html

////////////////////////////////////////////////////////////

///// TERMS

// OL   : outline, aka Tree
// root : first item, main title
// sub  : son, immediate descendant
// dad  : father
// cur  : current
// id   : input from disk (aka read, load)
// od   : output to disk (aka write, save)
// ik   : input from keyboard
// os   : output to screen (aka display)

////////////////////////////////////////////////////////////


///// INCLUDE as little as possible

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>

////////////////////////////////////////////////////////////

///// DEFINE

#define OLSIZE 255
#define LINELEN 65
#define MAXSUBS 50
#define SCRTOP 3
#define SCRBOT 20
#define LEGALCHARS "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ,.-?!0123456789"
#define MAXFILENAME 8+1+3
#define ASCNUL     (256+3)
#define ASCBEL        7
#define ASCBS         8
#define ASCTAB        9
#define ASCLF       0xA
#define ASCFF       0xC
#define ASCCR       0xD
#define ASCESC     0x1B
#define ASCDEL     0x1F
#define ASCSPACE   0x20

#define HOMEKEY     (71)
#define ENDKEY      (79)
// #define UPARROW     (72)
// #define RIGHTARROW  (77)
// #define DOWNARROW   (80)
// #define LEFTARROW   (75)
#define UPARROW     'U'
#define RIGHTARROW  'K'
#define DOWNARROW   'J'
#define LEFTARROW   'H'
#define BACKTAB     (15)
#define PGUPKEY     (73)
#define PGDNKEY     (81)
#define INSERTKEY   (82)
#define DELETEKEY   (83)
#define CTLPRTSC    (256+114)
#define CTLLARROW   (256+115)
#define CTLRARROW   (256+116)
#define CTLEND      (256+117)
#define CTLPGDN     (256+118)
#define CTLHOME     (256+119)
#define CTLPGUP     (256+132)
#define F1KEY       (256+59)
#define F11KEY      (256+84)
#define F21KEY      (256+94)
#define F31KEY      (256+104)
#define ALT1KEY     (256+120)

////////////////////////////////////////////////////////////

///// VARIABLES

struct Node {
    char text[LINELEN];
    int level;          // root is 0, sons are 1, grandsons are 2
    int dad;            // for sons it's 1 (root), for root it's 0
    int nsons;          // number of sons
    int sons[MAXSUBS];  // node number of each son
    // metaitems, such as author, date ...
} node[OLSIZE];

int nnodes = 0, cur, title, curson, topson;  //curson = current son
FILE *fileout;

////////////////////////////////////////////////////////////

///// CONIO

void clrscr()
{
 clear();
}

void clreol()
{
 clrtoeol();
}

int gotoxy(int x, int y)
{
 move(y - 1, x - 1);
 return 0;
}

int cprintf(char *fmt, ...)
{
 va_list arg; 
 va_start(arg, fmt);

 int i = vwprintw(stdscr, fmt, arg);

 va_end(arg);

 refresh();
 return i;
}

////////////////////////////////////////////////////////////

///// MAIN

int main(void)
{
 char ch;
 init();
 for(;;) {
    ch=menu();
    switch(ch){
        int d; // for double check

        case 'R' : idTree();  break;
        case 'W' : odTree();  break;
        case 'N' : newTree(); break;

        case LEFTARROW  : goDad();   break;
        case RIGHTARROW : goSon();   break;
        case UPARROW    : goOlder(); break;
        case DOWNARROW  : goYoung(); break;

        case 'A' : addNode(); break;
        case 'D' : delNode(); break;
        case 'L' : posTree(); break; // locate, move subtree
        case 'C' : cpyTree(); break; // copy
        case 'E' : editnode(); break;
        case 'X' : editroot(); break;

        case 'Y' : showhlp(); break; //yell!
        case 'Q' : d=dblchk(); if (d) exit(0);
        } // switch
    } // for
final();
} // main

////////////////////////////////////////////////////////////

char *getfname(char *t, int x, int y);

init()
{
 int ch;
 initscr();
 clrscr();
 printw("sole is a Small OutLine Editor, written in C by a non programmer.");
 printw("\nIt is distributed under the GNU General Public Licence.");
 printw("\nCurrently it doesn't do much, really - you can always help.");
 printw("\nAuthor  : lucas@comtf.es");
 printw("\nWebpage : http://www.gulic.org/copensar/index.html");
 printw("\n\nPress any key to start.\n");
 refresh();
 ch=getch();
}

showhlp()
{
 char ch;
 clrscr();
 printw("Press one of the available keys to navigate, load, edit, save,");
 printw("\nand so on. The letters in lower case are not implemented yet.");
 printw("\n\nPress any key to go back to the current outline, if any.\n");
 refresh();
 ch=getch();
 osTree();
}

dblchk()
{
}

salir(){}  //means "go out" in Spanish

final()
{
 endwin();
}

///// MENU
int menu()
{
 char ch;
 char TxtMnu1[] = "[R]ead[W]rite[N]ew [<^>v] [E]dit[X]root [A]dd[d]el";
 char TxtMnu2[] = "[l]ocate[c]opy [Y]ell [Q]uit\n";

 do {
    gotoxy(1,23); clreol(); gotoxy(1,23);
    cprintf("%s%s ",TxtMnu1,TxtMnu2);
    ch=toupper(getch());
   } while (
            (!chinstr(ch,"RWEXADLCYQN"))
         && (ch!=UPARROW)
         && (ch!=DOWNARROW)
         && (ch!=LEFTARROW)
         && (ch!=RIGHTARROW)
         );
 cprintf("\n");
 return ch;
} // menu

chinstr(ch,s)
char ch,*s;
{
 while (*s)     {
       if(ch==*s) return ch;
       else s++;
 }
 return 0;
}

/////////////////////////////////////////////////////

idTree()
{
//it'd be simpler to read if we define a father int variable
//father = node[cur].dad
//and maybe other variables
    FILE *in;
    int ch;
    long n;
    int nr, pre, i;
    char *filename;

    cur = 1; nnodes = 0; pre = 0;
    node[cur].level = 0;

    filename=(char *)malloc(MAXFILENAME + 1);
    filename=getfname("Infile:",1,24);
    /*filename="";*/
    if (filename=="") {
       strcpy(filename,"demo.ol");
    }

    if ((in=fopen(filename,"r"))!=NULL){
        while (fgets(node[cur].text,LINELEN,in)!=NULL){
        node[cur].text[strlen(node[cur].text)-1]='\0'; //last char is CR
        node[cur].level = strspn(node[cur].text, " "); //" " or "*"
        strcpy(node[cur].text, &node[cur].text[node[cur].level]);
        if (cur==0) node[cur].level=0; //done 5 and 3 lines ago?
        nnodes ++;
        node[cur].nsons = 0;
        if (node[pre].level<node[cur].level) {
           node[cur].dad = pre;
           node[pre].nsons = 1;
           node[pre].sons[1] = cur;
        }
        if (node[pre].level==node[cur].level) {
           node[cur].dad=node[pre].dad;
           node[node[cur].dad].nsons++;
           node[node[cur].dad].sons[node[node[cur].dad].nsons]=cur;
        }
        if (node[pre].level>node[cur].level) {
           for (i=pre-1;i > -1 ; i--){
               if (node[i].level == node[cur].level) {
               node[cur].dad = node[i].dad;
               node[node[cur].dad].nsons++;
               node[node[cur].dad].sons[node[node[cur].dad].nsons]=cur;
               break; //get out of the if statement
               } // if
           } // for
        }
        pre = cur;
        cur ++; //check if cur > OLSIZE
                //inform of incomplete file-read
                //and suggest options: abort(=new or old) or incomplete
        }
    } else {
        cprintf("CAN'T READ");
    };
cur --; //correct excesive step
//show what I read, just testing:
/*
clrscr();
for (i=1;i<nnodes+1;i++){
    cur = i;
    printf("<Node =%3i>",cur);
    printf("<Level=%3i>",node[cur].level);
    printf("<Dad  =%3i>",node[cur].dad);
    printf("<NSons=%3i>",node[cur].nsons);
    printf("<Text =%s>",node[cur].text);
    printf("\n");
    }
ch=getch();
*/

title=1;
cur=node[1].sons[1];
curson=1;
topson = 1;
osTree();

return 0;
}

verNivel(){}

itNombreFichero()
{
/*   Devuelve nombre_fichero que es variable global apunta a char
SUB itNombreFichero
shared OLname$
locate 24,1
OLname$=""
input "leer fichero llamado [demo.ol]";OLname$
if OLname$="" then OLname$="demo.ol"
END SUB 'it nombre fichero
*/
}

///////////////////////////////////////////////

osTree1()       //this version is outdated - I use osTree()
                //Can be used for writing to file, I think.
{
int i;

clrscr();
gotoxy(1,1);
cprintf("TITULO : %s",node[node[cur].dad].text);
for (i=1;i<node[node[cur].dad].nsons+1;i++) {
     gotoxy(1,i+2);
     cprintf("%3i:   %s",i,node[node[node[cur].dad].sons[i]].text);
     gotoxy(5,i+2);
     if (node[node[node[cur].dad].sons[i]].nsons>0) cprintf("*");
     gotoxy(6,i+2);
     if (i==curson) cprintf(">");
     }
}

osTree()
{
//needs topson, curson and cur
int soni, line;

clrscr();
gotoxy(1,SCRTOP);
cprintf("%s",node[node[cur].dad].text);
line = SCRTOP + 2;  //2 should become a defined constant
soni = topson;
title = node[cur].dad;
do {
   gotoxy(1,line);
   cprintf("%3i:   %s",soni,node[node[title].sons[soni]].text);
   gotoxy(5,line);
   if (node[node[title].sons[soni]].nsons>0) cprintf("*");
   gotoxy(6,line);
   if (soni==curson) cprintf(">");
   line ++;
   soni ++;
} while ((line<SCRBOT)
      && (soni<(topson+SCRBOT-SCRTOP))
      && (soni<node[title].nsons+1));
}

////////////////////////////////////////////
////////// NAVIGATE: ///////////////////////

goOlder()
{
 if (curson>1) {
    curson--;
    if (curson<topson) topson=curson;
    cur=node[title].sons[curson];
    osTree();
 }
}

goYoung()
{
 if (node[title].nsons>curson) {
    curson++;
    if (curson>(SCRBOT-SCRTOP-2)) topson++;
    cur=node[title].sons[curson];
    osTree();
 }
}

goDad()
{
  if (title!=1) {      //if title is not root already
    int i,norder = 1;
    cur = title;
    title = node[cur].dad;
    for(i=0;i<node[title].nsons+1;i++) {
      if (node[title].sons[i]==cur) norder = i;
    }
    curson = norder;  //I should define a prevson - a stack with
                      //a to-surface direction and an in-depth direction
                      //to be able to remember a pathway
    topson = 1;
    osTree();
  } // else MOVE TO TITLE? -- I must change osTree(), I think
}

goSon()
{
 if (node[cur].nsons>0) {
    title = cur;
    cur = node[title].sons[1];
    curson = 1;
    topson = 1;
    osTree();
 }
}


///////////////////////////////////////////////////

addNode()
{
 char ch;
 char TxtMnu[] = "[B]efore [A]fter [S]on [C]ancel\n";

 do {
    gotoxy(1,23); clreol(); gotoxy(1,23);
    cprintf("Where to add: %s ",TxtMnu);
    ch=toupper(getch());
 } while (!chinstr(ch,"BASC"));
 cprintf("\n");
 switch(ch){
     case 'B' : addBrother(+0); break;
     case 'A' : addBrother(+1); break;
     case 'S' : addSon(); break;
     case 'C' : break;
     }
} // menu

addBrother(int where)
{
  int i,norder;
  if((node[title].nsons<MAXSUBS) && (nnodes<OLSIZE)) {
     node[title].nsons++;
     for(i=0;i<(node[title].nsons+1);i++) {
       if (node[title].sons[i]==cur) norder = i;
     }
     curson=norder+where;
     for(i=node[title].nsons;i>curson;i--){
        node[title].sons[i]=node[title].sons[i-1];
     }
     nnodes++;
     cur=nnodes;
     node[title].sons[curson]=cur;
     node[cur].level=node[title].level + 1;
     node[cur].nsons=0;
     node[cur].dad=title;
     strcpy(node[cur].text,"NEW");
     osTree();
     editnode();
  }
}

addSon()
{
  int i,norder;
  if((node[title].nsons<MAXSUBS) && (nnodes<OLSIZE)) {
     title=cur;
     node[title].nsons++;
     nnodes++;
     cur=nnodes;
     node[title].sons[node[title].nsons]=cur;
     for(i=0;i<(node[title].nsons+1);i++) {
       if (node[title].sons[i]==cur) norder = i;
     }
     curson=norder;
     node[cur].level = node[title].level + 1;
     node[cur].dad=title;
     strcpy(node[cur].text,"NEW");
     osTree();
     editnode();
  }
}

delNode(){}

newTree(){
    int ch;
    long n;
    int nr, i;

    node[1].level=0;
    strcpy(node[1].text,"TITLE"); //write function to edit root!
    node[1].nsons=1;
    node[1].sons[1]=2;
    node[1].dad=0;

    node[2].level=1;
    strcpy(node[2].text,"SON");
    node[2].nsons=0;
    node[2].dad=1;

    nnodes = 2; title=1; cur=2;
    curson=1; topson = 1; osTree();

    return 0;
}

cpyTree(){}

posTree(){}

/////////////////////////////////////////////

odTree()
{
    char *filename;

    int odcur=1; //or root of subTree

    filename=(char *)malloc(MAXFILENAME + 1);
    filename=getfname("Outfile:",1,24);
    if (filename=="") {
       strcpy(filename,"demoout.ol");
    }

    if ((fileout=fopen(filename,"w"))!=NULL){
       odNodeNSons(odcur);
    }
    fclose(fileout);
}

odNodeNSons(int odcur)
{
  int i,odcur2;
  char spaces[LINELEN] = "\0";
        //add leading spaces according to level
        for (i=0;i<node[odcur].level;i++) strcat(spaces," ");
        //write to disk that spaced line
        fprintf(fileout,"%s%s\n",spaces,node[odcur].text);
        //and its descendants, if any
        if (node[odcur].nsons>0){
           for(i=1;i<(node[odcur].nsons+1);i++){
             odcur2=node[odcur].sons[i];
             odNodeNSons(odcur2);
           }
        }
}

/////////////// edit node
editnode()
{
// string:1         posinb              strlen(buf)
//        |         |                   |
// buf=  <this is a Long, long, long line           >
//        |         |                   |          |
// screen:minx      |                   |          |
//                  curx                |          |
//                                      lastx      |
//                                                 maxx=minx+LINELEN
    int ch,y,minx,maxx,curx,lastx,posinbuf;
    char buf[LINELEN];

    strcpy(buf,node[cur].text);
    y=SCRTOP+(curson-topson)+2;
    minx=8; maxx=minx+LINELEN;
    curx=minx; lastx=minx+strlen(buf)-1;
    posinbuf=0;

    gotoxy(curx,y);
    cprintf("%s",buf);
    gotoxy(curx,y);
    cprintf("%c",buf[0]);
    do
    {
        ch = getch();
        switch(ch){
            case LEFTARROW  :
                 if (curx>minx) {
                    curx--; posinbuf--;
                    gotoxy(curx,y);
                    };
                 break;
            case RIGHTARROW :
                 if (curx<(lastx+1)) {
                    curx++; posinbuf++;
                    gotoxy(curx,y);
                    };
                 break;
            case ASCESC : break;
            case ASCCR  : break;
            default :
                 //assuming "overwrite mode" - the editor is *S*imple, too!
                 if (chinstr(ch,LEGALCHARS)) {
                    buf[posinbuf]=ch;
                    gotoxy(curx,y);
                    cprintf("%c",ch);
                    //then do as in right arrow
                    if (curx<(lastx+1)) {
                       curx++; posinbuf++;
                       gotoxy(curx,y);
                    //except the line may grow
                    } else if (lastx<maxx) {
                       lastx++; curx++; posinbuf++;
                       buf[posinbuf+1]='\0';
                       gotoxy(curx,y);
                    };
                    };
                 break;
            //case insert, delete, backdelete
        }
    } while ((ch!=ASCESC)&&(ch!=ASCCR));
    if (ch==ASCCR) strcpy(node[cur].text,buf);
    gotoxy(minx,y); cprintf("%s",node[cur].text);
}

int it_key()
{
	int c;
	c=getch();
	if (c==0){
		c=getch();
		c=c+256;
	}
}

char *getfname(char *t, int x, int y)
{
    char *buffer;

    buffer=(char*) malloc(MAXFILENAME + 1);
    gotoxy(x,y);
    cprintf("%s", t);
    scanf("%s",buffer);
    return buffer;
}

editroot()
{
    int ch,y,minx,maxx,curx,lastx,posinbuf;
    char buf[LINELEN];

    clrscr();
    strcpy(buf,node[1].text);
    y=2;
    minx=8; maxx=minx+LINELEN;
    curx=minx; lastx=minx+strlen(buf)-1;
    posinbuf=0;

    gotoxy(curx,y);
    cprintf("%s",buf);
    gotoxy(curx,y);
    cprintf("%c",buf[0]);
    do
    {
        ch = getch();
        switch(ch){
            case LEFTARROW  :
                 if (curx>minx) {
                    curx--; posinbuf--;
                    gotoxy(curx,y);
                    };
                 break;
            case RIGHTARROW :
                 if (curx<(lastx+1)) {
                    curx++; posinbuf++;
                    gotoxy(curx,y);
                    };
                 break;
            case ASCESC : break;
            case ASCCR  : break;
            default :
                 //assuming "overwrite mode" - the editor is *S*imple, too!
                 if (chinstr(ch,LEGALCHARS)) {
                    buf[posinbuf]=ch;
                    gotoxy(curx,y);
                    cprintf("%c",ch);
                    //then do as in right arrow
                    if (curx<(lastx+1)) {
                       curx++; posinbuf++;
                       gotoxy(curx,y);
                    //except the line may grow
                    } else if (lastx<maxx) {
                       lastx++; curx++; posinbuf++;
                       buf[posinbuf+1]='\0';
                       gotoxy(curx,y);
                    };
                    };
                 break;
            //case insert, delete, backdelete
        }
    } while ((ch!=ASCESC)&&(ch!=ASCCR));
    if (ch==ASCCR) strcpy(node[1].text,buf);
    osTree();
}

//end of sole008.c
