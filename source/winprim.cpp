/*
    stdwin window primitives
    written by tim budd, january 1989
*/

# include "stdwin.h"
/* undefine guido's NOARGS so it doesn't conflict with mine */
# undef NOARGS
# include <stdio.h>
# include "env.h"
# include "objmemory.h"
# include "names.h"

extern object booleanSyms[booleanTrue], booleanSyms[booleanFalse];
extern boolean parseok;
extern int initial;

/* report a fatal system error */
noreturn sysError(s1, s2)
char *s1, *s2;
{   char buffer[1024];

    if (initial) {
        ignore fprintf(stderr,"%s\n%s\n", s1, s2);
        }
    else {
        ignore sprintf(buffer,"%s %s", s1, s2);
        wperror(buffer);
        }
    ignore abort();
}

/* report a fatal system error */
noreturn sysWarn(s1, s2)
char *s1, *s2;
{   char buffer[1024];

    if (initial) {
        ignore fprintf(stderr,"%s\n%s\n", s1, s2);
        }
    else {
        ignore sprintf(buffer,"%s %s", s1, s2);
        wperror(buffer);
        }
}

compilWarn(selector, str1, str2)
char *selector, *str1, *str2;
{   char buffer[1024];

    if (initial) {
        ignore fprintf(stderr,"compiler warning: Method %s : %s %s\n", 
            selector, str1, str2);
        }
    else {
        ignore sprintf(buffer,"warn: %s %s", str1, str2);
        wmessage(buffer);
        }
}

compilError(selector, str1, str2)
char *selector, *str1, *str2;
{   char buffer[1024];

    if (initial) {
        ignore fprintf(stderr,"compiler error: Method %s : %s %s\n", 
            selector, str1, str2);
        }
    else {
        ignore sprintf(buffer,"error: %s %s", str1, str2);
        wmessage(buffer);
        }
    parseok = false;
}

noreturn dspMethod(cp, mp)
char *cp, *mp;
{
    /*ignore fprintf(stderr,"%s %s\n", cp, mp);*/
}

givepause()
{   char buffer[80];

    if (initial) {
        ignore fprintf(stderr,"push return to continue\n");
        ignore gets(buffer);
        }
    else
        wmessage("wait to continue");
}

static object newPoint(x, y)
int x, y;
{   object newObj;

    newObj = MemoryManager::Instance()->allocObject(2);
    objectRef(newObj).setClass(globalSymbol("Point"));
    objectRef(newObj).basicAtPut(1, newInteger(x));
    objectRef(newObj).basicAtPut(2, newInteger(y));
    return newObj;
}

/* windows and text edit buffers are maintained in
   a single structure */
# define WINDOWMAX 15
static struct {
    WINDOW *w;
    TEXTEDIT *tp;
    } ws[WINDOWMAX];

/* menus are maintained in a similar structure */
# define MENUMAX 20
static MENU *mu[MENUMAX];

/* current event record */
static EVENT evrec;

static int findWindow(w)
WINDOW *w;
{   int i;
    for (i = 0; i < WINDOWMAX; i++)
        if (w == ws[i].w) return(i);
    sysError("can't find window","");
    return(0);
}

static void drawproc(w, left, top, right, bottom)
WINDOW *w;
int left, top, right, bottom;
{   int i;

    i = findWindow(w);
    if (ws[i].tp) tedraw(ws[i].tp);
}

object sysPrimitive(primitiveNumber, arguments)
int primitiveNumber;
object *arguments;
{   int i, j, k;
    int p1, p2, p3, p4;
    char *c;
    WINDOW *w;
    object returnedObject;

    returnedObject = nilobj;

    switch(primitiveNumber) {
    case 160:   /* window open */
        i = intValue(arguments[0]); /* win number */
        if (ws[i].w) break; /* already open */
        c = objectRef(arguments[1]).charPtr();  /* title */
        j = intValue(arguments[2]); /* text or not */
        if (j) {
            ws[i].w = w = wopen(c, drawproc);
            wgetwinsize(w, &j, &k);
            ws[i].tp = tecreate(w, 0, 0, j, k);
            }
        else {
            ws[i].w = wopen(c, NULL);
            ws[i].tp = 0;
            }   
        break;
        
    case 161:   /* variety of simple actions */
        i = intValue(arguments[0]); /* win number */
        if (! (w = ws[i].w)) break; /* return if no open */
        j = intValue(arguments[1]); /* action */
        switch(j) {
            case 1: ws[i].w = NULL; wclose(w); break;
            case 2: wbegindrawing(w); break;
            case 3: wenddrawing(w); break; 
            case 4: wsetactive(w); break;
            case 5: if (ws[i].tp) tedraw(ws[i].tp); break;
            case 6: wgetwinsize(w, &i, &j); 
                returnedObject = newPoint(i, j); break;
        }
        break;

    case 162:   /* one int arg actions */
        i = intValue(arguments[0]); /* win number */
        if (! (w = ws[i].w)) break; /* return if no open */
        i = intValue(arguments[1]); /* action */
        j = intValue(arguments[2]); /* x */
        switch(i) {
            case 1:  /* set timer */
                wsettimer(w, j); break;
            case 2: /* menu attach */
                wmenuattach(w, mu[j]); break;
            case 3: /* menu detach */
                wmenudetach(w, mu[j]); break;
        }
        break;

    case 163:   /* two int arg actions */
        i = intValue(arguments[0]); /* win number */
        if (! (w = ws[i].w)) break; /* return if no open */
        i = intValue(arguments[1]); /* action */
        j = intValue(arguments[2]); /* x */
        k = intValue(arguments[3]); /* y */
        switch(i) {
            case 2: wsetdocsize(w, j, k); break;
            case 3: wsetorigin(w, j, k); break;
        }
        break;

    case 164:   /* title */
        i = intValue(arguments[0]); /* win number */
        if (! (w = ws[i].w)) break; /* return if no open */
        c = objectRef(arguments[1]).charPtr();
        wsettitle(w, c);
        break;

    case 165:   /* get text */
        i = intValue(arguments[0]);
        if (ws[i].tp)
        returnedObject = newStString(tegettext(ws[i].tp));
        break;

    case 166:   /* replace text */
        i = intValue(arguments[0]);
        if (ws[i].tp) {
            tereplace(ws[i].tp, objectRef(arguments[1]).charPtr());
            /* add newline */
            tereplace(ws[i].tp, "\n");
            }
        break;

    case 170: getevent: /* get next event */
        wgetevent(&evrec);
        i = findWindow(evrec.window);
        if (ws[i].tp && teevent(ws[i].tp, &evrec))
            goto getevent;
/*fprintf(stderr,"returning event type %d %d\n", evrec.type, evrec.u.where.clicks);*/
        returnedObject = newInteger(evrec.type);
        break;

    case 171:   /* integer event info */
        i = intValue(arguments[0]);
        switch(i) {
        case 1:     /* event window */
            j = findWindow(evrec.window); break;
        case 2:     /* event menu */
            j = evrec.u.m.id; break;
        case 3:     /* menu item */
            j = evrec.u.m.item + 1; break;
        case 4:     /* char typed */
            j = evrec.u.character; break;
        case 5:     /* mouse y */
            j = evrec.u.where.v; break;
        case 6:     /* mouse button */
            j = evrec.u.where.button; break;
        case 7:     /* mouse click number */
            j = evrec.u.where.clicks;
        case 8:     /* char typed */
            j = evrec.u.character;
        case 9:     /* command typed */
            j = evrec.u.command;
        }
        returnedObject = newInteger(j);
        break;

    case 172:   /* more general event info */
        i = intValue(arguments[0]);
        switch(i) {
        case 1:     /* mouse down point */
            returnedObject = newPoint(evrec.u.where.h, 
                evrec.u.where.v); 
            break;
        }
        break;

    case 180:   /* new menu */
        i = intValue(arguments[0]); /* win number */
        c = objectRef(arguments[1]).charPtr(); /* title */
        mu[i] = wmenucreate(i, c);
        break;

    case 181:   /* menu item */
        i = intValue(arguments[0]); /* menu number */
        c = objectRef(arguments[1]).charPtr(); /* title */
        if (isInteger(arguments[2]))
            j = intValue(arguments[2]);
        else j = -1;
        wmenuadditem(mu[i], c, j);
        break;

    case 182:   /* check menu items */
        i = intValue(arguments[0]); /* menu number */
        j = intValue(arguments[1]); /* item number */
        k = intValue(arguments[2]); /* action */
        p1 = intValue(arguments[3]); /* flag */
        switch(k) {
            case 1: /* enable/disable */
                wmenuenable(mu[i],j-1,p1); break;
            case 2: /* check/no check */
                wmenucheck(mu[i],j-1,p1); break;
        }
        break;

    case 190:   /* print text graphics */
        i = intValue(arguments[0]); /* x */
        j = intValue(arguments[1]); /* y */
        c = objectRef(arguments[2]).charPtr(); /* text */
        wdrawtext(i, j, c, strlen(c));
        break;

    case 192:   /* points */
        i = intValue(arguments[0]); /* action */
        p1 = intValue(arguments[1]);
        p2 = intValue(arguments[2]);
        switch(i) {
            case 1:  /* draw line */
                wdrawline(p1, p2); break;
        }
        break;

    case 193:   /* circles and the like */
        i = intValue(arguments[0]); /* action */
        p1 = intValue(arguments[1]);
        p2 = intValue(arguments[2]);
        p3 = intValue(arguments[3]);
        switch(i) {
            case 1:     /* draw circle */
                wdrawcircle(p1,p2,p3); break;
            case 2:     /* draw char */
                wdrawchar(p1,p2,p3); break;
        }
        break;

    case 194:   /* rectangles */
        i = intValue(arguments[0]); /* action */
        p1 = intValue(arguments[1]);
        p2 = intValue(arguments[2]);
        p3 = intValue(arguments[3]);
        p4 = intValue(arguments[4]);
        switch(i) {
            case 1:     /* draw box */
                wdrawbox(p1,p2,p3,p4); break;
            case 2:     /* paint */
                wpaint(p1,p2,p3,p4); break;
            case 3:     /* erase */
                werase(p1,p2,p3,p4); break;
            case 4:     /* invert */
                winvert(p1,p2,p3,p4); break;
        }
        break;

    case 195:   /* shading */
        i = intValue(arguments[0]); /* action */
        p1 = intValue(arguments[1]);
        p2 = intValue(arguments[2]);
        p3 = intValue(arguments[3]);
        p4 = intValue(arguments[4]);
        j = intValue(arguments[5]);
        switch(i) {
            case 1:     /* shading */
                wshade(p1,p2,p3,p4,j); break;
            }
        break;

    case 200:   /* issue a message */
        c = objectRef(arguments[0]).charPtr();
        wmessage(c);
        break;

    case 201:   /* ask a question */
        { char replybuffer[120];
        strcpy(replybuffer, objectRef(arguments[1]).charPtr());
        if (waskstr(objectRef(arguments[0]).charPtr(), replybuffer, 120))
            returnedObject = newStString(replybuffer);
        }
        break;
    
    case 202:   /* asky a binary question */
        i = waskync(objectRef(arguments[0]).charPtr(), intValue(arguments[1]));
        if (i == 1) returnedObject = booleanSyms[booleanTrue];
        else if (i == 0) returnedObject = booleanSyms[booleanFalse];
        break;

    case 203:   /* ask for a file */
        { char replybuffer[120];
        strcpy(replybuffer, objectRef(arguments[1]).charPtr());
        if (waskfile(objectRef(arguments[0]).charPtr(), replybuffer, 120, intValue(arguments[2])))
            returnedObject = newStString(replybuffer);
        }
        break;

    case 204:   /* error message */
        wperror(objectRef(arguments[0]).charPtr());
        break;
        
    case 205:   /* beep */
        wfleep();
        break;

    default:
    fprintf(stderr,"primitive not implmented yet %d\n",
        primitiveNumber);
    sysError("primitive not done","");
    }

    return returnedObject;
}
