/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
***********
$Id$
*/


#include "ngspice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "dvec.h"
#include "fteparse.h"
#include "cpitf.h"

#include <terminal.h>

#include "completion.h"
#include "variable.h"

/* Set some standard variables and aliases, etc, and init the ccom stuff.  
   Called by fcn main() */

void
ft_cpinit(void)
{
    wordlist *wl;
    wordlist wl1, wl2, wl3;
    bool found = FALSE, t = TRUE;
    char buf[BSIZE_SP], **x, *s, *r,*copys;
    struct comm *c;
    int i;
    FILE *fp;
    static char *predefs[] = {
        "yes",      "1",
        "TRUE",     "1",
        "no",       "0",
        "FALSE",    "0",
        "pi",       "3.14159265358979323846",
        "e",        "2.71828182844590452353",
        "c",        "2.997925e8",
        "i",        "0,1",
        "kelvin",   "-273.15",
        "echarge",  "1.60219e-19",
        "boltz",    "1.38062e-23",
        "planck",   "6.62620e-34"
    } ;
    static char *udfs[] = {
        "max(x,y)", "(x gt y) * x + (x le y) * y",
        "min(x,y)", "(x lt y) * x + (x ge y) * y",
        "vdb(x)",   "db(v(x))",
        "vdb(x,y)", "db(v(x) - v(y))",
        "vi(x)",    "im(v(x))",
        "vi(x,y)",  "im(v(x) - v(y))",
        "vm(x)",    "mag(v(x))",
        "vm(x,y)",  "mag(v(x) - v(y))",
        "vg(x)",    "group_delay(v(x))", //A.Rroldan 10/06/05 group delay new function
        "gd(x)",    "group_delay(v(x))", //A.Rroldan 10/06/05 group delay new function        
        "vp(x)",    "ph(v(x))",
        "vp(x,y)",  "ph(v(x) - v(y))",
        "vr(x)",    "re(v(x))",
        "vr(x,y)",  "re(v(x) - v(y))"
    } ;

    /* if TIOCSTI is defined (not available in MS Windows:
       Make escape the break character.
       So the user can type ahead...
       fcn defined in complete.c. */
    cp_ccon(TRUE);
    /* Initialize io, cp_chars[], variable "history" in init.c. */
    cp_init();

    /* If command completion is available (global variable cp_nocc
       set in main.c by command line option -q)   */ 
    if (!cp_nocc) {
        /* Add commands... */
        for (c = cp_coms; c->co_func; c++) {
            if (c->co_spiceonly && ft_nutmeg)
                continue;
            cp_addcomm(c->co_comname, c->co_cctypes[0],
                    c->co_cctypes[1], c->co_cctypes[2],
                    c->co_cctypes[3]);
            cp_addkword(CT_COMMANDS, c->co_comname);
        }
        /* And keywords... These are the ones that are constant... */
        if (!ft_nutmeg) {
            cp_addkword(CT_LISTINGARGS, "deck");
            cp_addkword(CT_LISTINGARGS, "logical");
            cp_addkword(CT_LISTINGARGS, "physical");
            cp_addkword(CT_LISTINGARGS, "expand");

            cp_addkword(CT_STOPARGS, "when");
            cp_addkword(CT_STOPARGS, "after");
        }

        cp_addkword(CT_PLOT, "new");

        cp_addkword(CT_PLOTKEYWORDS, "xlimit");
        cp_addkword(CT_PLOTKEYWORDS, "ylimit");
        cp_addkword(CT_PLOTKEYWORDS, "vs");
        cp_addkword(CT_PLOTKEYWORDS, "xindices");
        cp_addkword(CT_PLOTKEYWORDS, "xcompress");
        cp_addkword(CT_PLOTKEYWORDS, "xdelta");
        cp_addkword(CT_PLOTKEYWORDS, "ydelta");
        cp_addkword(CT_PLOTKEYWORDS, "lingrid");
        cp_addkword(CT_PLOTKEYWORDS, "loglog");
        cp_addkword(CT_PLOTKEYWORDS, "linear");
        cp_addkword(CT_PLOTKEYWORDS, "xlog");
        cp_addkword(CT_PLOTKEYWORDS, "ylog");
        cp_addkword(CT_PLOTKEYWORDS, "polar");
        cp_addkword(CT_PLOTKEYWORDS, "smith");
        cp_addkword(CT_PLOTKEYWORDS, "smithgrid");
        cp_addkword(CT_PLOTKEYWORDS, "nointerp");
        cp_addkword(CT_PLOTKEYWORDS, "title");
        cp_addkword(CT_PLOTKEYWORDS, "xlabel");
        cp_addkword(CT_PLOTKEYWORDS, "ylabel");
        cp_addkword(CT_PLOTKEYWORDS, "linplot");
        cp_addkword(CT_PLOTKEYWORDS, "combplot");
        cp_addkword(CT_PLOTKEYWORDS, "pointplot");

        cp_addkword(CT_RUSEARGS, "time");
        cp_addkword(CT_RUSEARGS, "space");
        cp_addkword(CT_RUSEARGS, "faults");
        cp_addkword(CT_RUSEARGS, "elapsed");
        cp_addkword(CT_RUSEARGS, "totiter");
        cp_addkword(CT_RUSEARGS, "traniter");
        cp_addkword(CT_RUSEARGS, "tranpoints");
        cp_addkword(CT_RUSEARGS, "accept");
        cp_addkword(CT_RUSEARGS, "rejected");
        cp_addkword(CT_RUSEARGS, "time");
        cp_addkword(CT_RUSEARGS, "trantime");
        cp_addkword(CT_RUSEARGS, "lutime");
        cp_addkword(CT_RUSEARGS, "solvetime");
        cp_addkword(CT_RUSEARGS, "transolvetime");
        cp_addkword(CT_RUSEARGS, "loadtime");
        cp_addkword(CT_RUSEARGS, "all");

        cp_addkword(CT_VECTOR, "all");

        for (x = ft_setkwords; *x; x++)
            cp_addkword(CT_VARIABLES, *x);
        for (i = 0; ; i++) {
            if (!(s = ft_typenames(i)))
                break;
            cp_addkword(CT_TYPENAMES, s);
        }
    }

    cp_vset("program", VT_STRING, cp_program);

    /* Make the prompt use only the last component of the path... */

    if (DIR_TERM) {
	for (s = cp_program; s && *s; s++)
	    ;
	s--;
	while ((s > cp_program) && (*s != DIR_TERM))
	    s--;
	if (*s == DIR_TERM)
	    s++;
	(void) strcpy(buf, s);
	for (s = buf; *s && (*s != '.'); s++)
	    ;
	*s = '\0';
	(void) strcat(buf, " ! -> ");
    } else
	(void) sprintf(buf, "%s ! -> ", cp_program);

    cp_vset("prompt", VT_STRING, buf);
    cp_vset("noglob", VT_BOOL, (char *) &t);
    cp_vset("brief", VT_BOOL, (char *) &t);

    /* Make vectors from values in predefs[] for the current plot. 
     Define functions from entries in udfs[] (like user defined functions).
     */
    wl1.wl_next = &wl2;
    wl1.wl_prev = NULL;
    wl2.wl_next = NULL;
    wl2.wl_prev = &wl1;
    wl1.wl_word = "if";
    wl2.wl_word = "1";
    cp_setalias("begin", &wl1);
    wl1.wl_next = NULL;
    wl1.wl_word = "end";
    cp_setalias("endif", &wl1);
    cp_setalias("endwhile", &wl1);
    cp_setalias("endforeach", &wl1);
    cp_setalias("endrepeat", &wl1);
    cp_setalias("enddowhile", &wl1);
    wl1.wl_word = "help";
    cp_setalias("?", &wl1);

    wl1.wl_next = &wl2;
    wl2.wl_next = &wl3;
    wl2.wl_prev = &wl1;
    wl3.wl_prev = &wl2;
    wl3.wl_next = NULL;
    wl2.wl_word = "=";
    for (i = 0; i < sizeof (predefs) / sizeof (char *); i += 2) {
        wl1.wl_word = predefs[i];
        wl3.wl_word = predefs[i + 1];
        com_let(&wl1);
    }

    wl2.wl_next = NULL;
    for (i = 0; i < sizeof (udfs) / sizeof (char *); i += 2) {
        wl1.wl_word = udfs[i];
        wl2.wl_word = udfs[i + 1];
        com_define(&wl1);
    }

    /* Reset this for the front end. */
    cp_hash = '*';

    /* NGSPICEDATADIR has been set to path "$dprefix/share/ngspice" in configure.in, 
       Spice_Lib_Dir has been set to NGSPICEDATADIR in conf.c, 
       may be overridden by environmental variable SPICE_LIB_DIR in ivars().
       Lib_Path has been set to Spice_Lib_Dir adding /scripts in ivars() */
    if (Lib_Path && *Lib_Path) {
       /* set variable 'sourcepath' */ 
       (void) sprintf(buf, "sourcepath = ( %s %s )", DIR_CWD, Lib_Path);
        wl = cp_doglob(cp_lexer(buf));
        cp_striplist(wl);
        com_set(wl);
        wl_free(wl);
        
        /* Now source the standard startup file spinit or tclspinit. */

        /* jump over leading spaces */
        for (copys=s=cp_tildexpand(Lib_Path); copys && *copys; ) {
            while (isspace(*s))
                s++;
            /* copy s into buf until space is seen, r is the actual position */
            for (r = buf; *s && !isspace(*s); r++, s++)
                *r = *s;
            tfree(copys);
	         /* add a path separator to buf at actual position */
            (void) strcpy(r, DIR_PATHSEP);
#ifdef TCL_MODULE
            /* add "tclspinit" to buf after actual position */
            (void) strcat(r, "tclspinit");
#else
            /* add "spinit" to buf after actual position */
            (void) strcat(r, "spinit");
#endif
            if ((fp = fopen(buf, "r"))) {
                cp_interactive = FALSE;
                inp_spsource(fp, TRUE, buf);
                cp_interactive = TRUE;
                found = TRUE;
                break;
#if defined (HAS_WINDOWS) || defined (__MINGW32__) || defined (_MSC_VER)
            /* search in local directory where ngspice.exe resides */
            } else if ((fp = fopen("./spinit", "r"))) {
                cp_interactive = FALSE;
                inp_spsource(fp, TRUE, buf);
                cp_interactive = TRUE;
                found = TRUE;
                break;
#endif                
            } else if (ft_controldb)
                fprintf(cp_err, "Note: can't open \"%s\".\n", buf);
        }
        if (!found)
            fprintf(cp_err, "Note: can't find init file.\n");
    }

    tcap_init( );
    return;
}

/* Decide whether a condition is TRUE or not. */

bool
cp_istrue(wordlist *wl)
{
    int i;
    struct dvec *v;
    struct pnode *pn;

/* fprintf(stderr, "isTRUE: "); wl_print(wl, stderr); fprintf(stderr, "\n"); */
    /* First do all the csh-type stuff here... */
    wl = wl_copy(wl);
    wl = cp_variablesubst(wl);
    wl = cp_bquote(wl);
    cp_striplist(wl);

    pn = ft_getpnames(wl, TRUE);
    wl_free(wl);
    v = ft_evaluate(pn);

    /* It makes no sense to say while (all), but what the heck... */
    while (v) {
        if (isreal(v)) {
            for (i = 0; i < v->v_length; i++)
                if (v->v_realdata[i] != 0.0) {
		   free_pnode(pn);
                   return (TRUE);
		}
        } else {
            for (i = 0; i < v->v_length; i++)
                if ((realpart(&v->v_compdata[i]) != 0.0) ||
                    (imagpart(&v->v_compdata[i]) != 0.0)) {
		    free_pnode(pn);
                    return (TRUE);
		}
        }
        v = v->v_link2;
    }
   free_pnode(pn);
   return (FALSE);
}

/* This gets called before every command is executed... 
   from fcns do_command() or do_block() in control.c */

void
cp_periodic(void)
{
    ft_setflag = FALSE;
    ft_intrpt = FALSE;
    ft_ckspace();   /* check for max. data size (resource.c) */
    ft_checkkids(); /* check for jobs (only if OK_ASPICE is defined, apsice.c) */
    vec_gc();       /* remove vectors which do not have permanent flag set (vectors.c) */
    return;
}

void
cp_doquit(void)
{
    com_quit(NULL);
    return;
}

/* This is how we deal with emulation of commands by scripts... If the script
 * is found, then set the variables argc and argv and call the script.  Note
 * that this also allows us to type a filename to load a spice deck...
 */

bool
cp_oddcomm(char *s, wordlist *wl)
{
    FILE *fp;
    char buf[BSIZE_SP];
    wordlist ww;

    if ((fp = inp_pathopen(s, "r"))) {
        (void) fclose(fp);
        (void) sprintf(buf, "argc = %d argv = ( ", wl_length(wl));
        while (wl) {
            (void) strcat(buf, wl->wl_word);
            (void) strcat(buf, " ");
            wl = wl->wl_next;
        }
        (void) strcat(buf, ")");
        com_set(cp_lexer(buf));
        inp_source(s);
        cp_remvar("argc");
        cp_remvar("argv");
        return (TRUE);
    } else if (wl && eq(wl->wl_word, "=")) {
        ww.wl_next = wl;
        wl->wl_prev = &ww;
        ww.wl_prev = NULL;
        ww.wl_word = s;
        com_let(&ww);
        return (TRUE);
    } else
        return (FALSE);
}

