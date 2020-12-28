/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */
#include <X11/Xft/Xft.h>
/* custom */
#include <fcntl.h>
#include <sys/prctl.h>

#include "drw.h"
#include "util.h"

/* macros */
#define BUTTONMASK                      (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)                 (mask & ~(numlockmask|LockMask) & \
                                         (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)            (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                                       * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISVISIBLE(C)                    ((C->tags & C->mon->tagset[C->mon->seltags]) && !C->ishidden)
#define LENGTH(X)                       (sizeof X / sizeof X[0])
#define MOUSEMASK                       (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                        ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)                       ((X)->h + 2 * (X)->bw)
#define TAGMASK                         ((1 << LENGTH(tags)) - 1)
#define TEXTW(X)                        (drw_fontset_getwidth(drw, (X)) + lrpad)
#define TTEXTW(X)                       (drw_fontset_getwidth(drw, (X)))
#define ATT(M)                          (M->pertag->attidxs[M->pertag->curtag][M->pertag->selatts[M->pertag->curtag]])
#define SPLUS(M)                        (M->pertag->splus[M->pertag->curtag])

#define STATUSLENGTH                    256
#define ROOTNAMELENGTH                  320 /* fake signal + status */
#define DSBLOCKSLOCKFILE                "/tmp/dsblocks.pid"
#define DELIMITERENDCHAR                10

#define SYSTEM_TRAY_REQUEST_DOCK        0

/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY          0
#define XEMBED_WINDOW_ACTIVATE          1
#define XEMBED_FOCUS_IN                 4
#define XEMBED_MODALITY_ON              10

#define XEMBED_MAPPED                   (1 << 0)
#define XEMBED_WINDOW_ACTIVATE          1
#define XEMBED_WINDOW_DEACTIVATE        2

#define VERSION_MAJOR                   0
#define VERSION_MINOR                   0
#define XEMBED_EMBEDDED_VERSION         ((VERSION_MAJOR << 16) | VERSION_MINOR)

/* enums */
enum { CurNormal, CurHand, CurResize, CurMove, CurLast }; /* cursor */
enum { NetSupported, NetWMName, NetWMState, NetWMCheck,
       NetSystemTray, NetSystemTrayOP, NetSystemTrayOrientation,
       NetSystemTrayOrientationHorz, NetWMFullscreen, NetActiveWindow,
       NetWMWindowType, NetWMWindowTypeDialog, NetDesktopNames, NetWMDesktop,
       NetClientList, NetLast }; /* EWMH atoms */
enum { Manager, Xembed, XembedInfo, XLast }; /* Xembed atoms */
enum { WMProtocols, WMDelete, WMState, WMTakeFocus, WMLast }; /* default atoms */
enum { ClkTagBar, ClkTabBar, ClkLtSymbol, ClkStatusText, ClkWinTitle,
       ClkClientWin, ClkRootWin, ClkLast }; /* clicks */

typedef union {
	int i;
	unsigned int ui;
	float f;
	const void *v;
} Arg;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client {
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int sfx, sfy, sfw, sfh; /* stored float geometry, used on mode revert */
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh;
	int bw, oldbw;
	unsigned int tags;
        int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen, ishidden;
	int scratchkey;
	Client *next;
	Client *snext;
	Monitor *mon;
	Window win;
};

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	const char * sig;
	void (*func)(const Arg *);
} Signal;

typedef struct {
        const char *symbol;
        void (*attach)(Client *);
} Attach;

typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
        const Attach *attach;
} Layout;

typedef struct Pertag Pertag;
struct Monitor {
	char ltsymbol[16];
	float mfact;
        int ntiles;
	int nmaster;
	int num;
	int by;               /* bar geometry */
	int ty;               /* tab bar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int topbar;
	int toptab;
        int statushandcursor;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Window barwin;
	Window tabwin;
	const Layout *lt[2];
	Pertag *pertag;
};

typedef struct Systray Systray;
struct Systray {
	Window win;
	Client *icons;
};

/* function declarations */
static void applycurtagsettings(void);
static void applyrules(Client *c); /* defined in config.h */
static int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);
static void arrange(Monitor *m);
static void arrangemon(Monitor *m);
static void attach(Client *c);
static void attachabove(Client *c);
static void attachaside(Client *c);
static void attachbelow(Client *c);
static void attachbottom(Client *c);
static void attachstack(Client *c);
static void buttonpress(XEvent *e);
static void checkotherwm(void);
static void cleanup(void);
static void cleanupmon(Monitor *mon);
static void clientmessage(XEvent *e);
static void configure(Client *c);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static Monitor *createmon(void);
static void deck(Monitor *m);
static void destroynotify(XEvent *e);
static void detach(Client *c);
static void detachstack(Client *c);
//static Monitor *dirtomon(int dir);
static void drawbar(Monitor *m);
static void drawbars(void);
static void drawtab(Monitor *m);
static void drawtabhelper(Monitor *m, int onlystack);
static void drawtabs(void);
//static void enternotify(XEvent *e);
static void expose(XEvent *e);
static void focus(Client *c);
static void focusalt(Client *c);
static void focusclient(Client *c, unsigned int tag);
static void focusin(XEvent *e);
static void focuslast(const Arg *arg);
//static void focusmon(const Arg *arg);
//static void focusstack(const Arg *arg);
static void focuswin(const Arg* arg);
static Atom getatomprop(Client *c, Atom prop);
static int getrootptr(int *x, int *y);
static int getwinptr(Window win, int *x, int *y);
static long getstate(Window w);
static unsigned int getsystraywidth();
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);
static void grabbuttons(Client *c, int focused);
static void grabkeys(void);
static void incnmaster(const Arg *arg);
static void keypress(XEvent *e);
static void killclient(const Arg *arg);
static void manage(Window w, XWindowAttributes *wa);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void monocle(Monitor *m);
static void motionnotify(XEvent *e);
static void swaptags(const Arg *arg);
static void movemouse(const Arg *arg);
static void moveprevtag(const Arg *arg);
static Client *nexttiled(Client *c);
static void pop(Client *);
static void propertynotify(XEvent *e);
static void quit(const Arg *arg);
static Monitor *recttomon(int x, int y, int w, int h);
static void removesystrayicon(Client *i);
static void resettagifempty(unsigned int tag);
static void resetsplus(const Arg *arg);
static void resize(Client *c, int x, int y, int w, int h, int interact);
static void resizebarwin(Monitor *m);
static void resizeclient(Client *c, int x, int y, int w, int h);
static void resizemouse(const Arg *arg);
static void resizerequest(XEvent *e);
static void restack(Monitor *m, int dbr);
static void restorestatus(void);
static void run(void);
static void scan(void);
static void scratchhide(const Arg *arg);
static void scratchhidehelper(void);
static void scratchshow(const Arg *arg);
static void scratchshowhelper(int key);
static void scratchtoggle(const Arg *arg);
static int sendevent(Window w, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);
static void sendmon(Client *c, Monitor *m);
static void setattach(const Arg *arg);
static void setattorprev(const Arg *arg);
static void setclientstate(Client *c, long state);
static void setdesktopnames(void);
static void setfocus(Client *c);
static void setfullscreen(Client *c, int fullscreen);
static void setlayout(const Arg *arg);
static void setltorprev(const Arg *arg);
static void setmfact(const Arg *arg);
static void setsplus(const Arg *arg);
static void setup(void);
static void seturgent(Client *c, int urg);
static void shifttag(const Arg *arg);
static void shiftview(const Arg *arg);
static void showhide(Client *c);
static void sigchld(int unused);
static void sigdsblocks(const Arg *arg);
static void spawn(const Arg *arg);
static void tabmode(const Arg *arg);
static void tag(const Arg *arg);
static void tagandview(const Arg *arg);
//static void tagmon(const Arg *arg);
static void tile(Monitor *m);
static void tiledeck(Monitor *m, int deck);
static void togglebar(const Arg *arg);
static void togglefloating(const Arg *arg);
static void toggletag(const Arg *arg);
static void toggleview(const Arg *arg);
static void togglewin(const Arg *arg);
static void unfocus(Client *c);
static void unmanage(Client *c, int destroyed);
static void unmapnotify(XEvent *e);
static void updatebarpos(Monitor *m);
static void updatebars(void);
static void updateclientdesktop(Client *c);
static void updateclientlist(void);
static void updatedsblockssig(int x);
static int updategeom(void);
static void updatentiles(Monitor *m);
static void updatenumlockmask(void);
static void updateselmon(Monitor *m);
static void updatesizehints(Client *c);
static void updatestatus(void);
static void updatesystray(void);
static void updatesystrayicongeom(Client *i, int w, int h);
static void updatesystrayiconstate(Client *i, XPropertyEvent *ev);
static void updatetitle(Client *c);
static void updatewindowtype(Client *c);
static void updatewmhints(Client *c);
static void view(const Arg *arg);
static Client *wintoclient(Window w);
static Monitor *wintomon(Window w);
static Client *wintosystrayicon(Window w);
static int xerror(Display *dpy, XErrorEvent *ee);
static int xerrordummy(Display *dpy, XErrorEvent *ee);
static int xerrorstart(Display *dpy, XErrorEvent *ee);
static void zoom(const Arg *arg);

/* variables */
static const char broken[] = "";
static char stextc[STATUSLENGTH];
static char stexts[STATUSLENGTH];
static int screen;
static int sw, sh;           /* X display screen geometry width, height */
static int bh, blw, ble;     /* bar geometry */
static int wsbar;            /* width of bar with systray */
static int wstext;           /* width of status text */
static int th;               /* tab bar geometry */
static int lrpad;            /* sum of left and right paddings for text */
static int restart = 0;
static int running = 1;
static int (*xerrorxlib)(Display *, XErrorEvent *);
static unsigned int dsblockssig;
static unsigned int numlockmask = 0;
static void (*handler[LASTEvent]) (XEvent *) = {
	[ButtonPress] = buttonpress,
	[ClientMessage] = clientmessage,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
//	[EnterNotify] = enternotify,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
	[PropertyNotify] = propertynotify,
	[ResizeRequest] = resizerequest,
	[UnmapNotify] = unmapnotify
};
static Atom wmatom[WMLast], netatom[NetLast], xatom[XLast];
static Cur *cursor[CurLast];
static Clr **scheme;
static Display *dpy;
static Drw *drw;
static Monitor *mons, *selmon;
static Systray *systray = NULL;
static Window root, wmcheckwin;

/* configuration, allows nested code to access above variables */
#include "config.h"

struct Pertag {
	unsigned int curtag, prevtag; /* current and previous tag */
	int nmasters[LENGTH(tags) + 1]; /* number of windows in master area */
	float mfacts[LENGTH(tags) + 1]; /* mfacts per tag */
	unsigned int sellts[LENGTH(tags) + 1]; /* selected layouts */
	const Layout *ltidxs[LENGTH(tags) + 1][2]; /* matrix of tags and layouts indexes */
	int showbars[LENGTH(tags) + 1]; /* display bar for the current tag */
        /* custom */
        unsigned int selatts[LENGTH(tags) + 1]; /* selected attach positions */
        const Attach *attidxs[LENGTH(tags) + 1][2]; /* matrix of tags and attach positions indexes */
        int showtabs[LENGTH(tags) + 1]; /* display tab per tag */
        int splus[LENGTH(tags) + 1][3]; /* extra size per tag - first master and first two stack clients */
};

/* compile-time check if all tags fit into an unsigned int bit array. */
struct NumTags { char limitexceeded[LENGTH(tags) > 31 ? -1 : 1]; };

/* function implementations */
void
applycurtagsettings(void)
{
	selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag];
	selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag];
	selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag];
	selmon->lt[0] = selmon->pertag->ltidxs[selmon->pertag->curtag][0];
	selmon->lt[1] = selmon->pertag->ltidxs[selmon->pertag->curtag][1];

	if (selmon->showbar != selmon->pertag->showbars[selmon->pertag->curtag])
		togglebar(NULL);
}

int
applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
	int baseismin;
	Monitor *m = c->mon;

	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);
	if (interact) {
		if (*x > sw)
			*x = sw - WIDTH(c);
		if (*y > sh)
			*y = sh - HEIGHT(c);
		if (*x + *w + 2 * c->bw < 0)
			*x = 0;
		if (*y + *h + 2 * c->bw < 0)
			*y = 0;
	} else {
		if (*x >= m->wx + m->ww)
			*x = m->wx + m->ww - WIDTH(c);
		if (*y >= m->wy + m->wh)
			*y = m->wy + m->wh - HEIGHT(c);
		if (*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;
		if (*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}
	if (*h < bh)
		*h = bh;
	if (*w < bh)
		*w = bh;
	if (resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;
		if (!baseismin) { /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for aspect limits */
		if (c->mina > 0 && c->maxa > 0) {
			if (c->maxa < (float)*w / *h)
				*w = *h * c->maxa + 0.5;
			else if (c->mina < (float)*h / *w)
				*h = *w * c->mina + 0.5;
		}
		if (baseismin) { /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}
		/* adjust for increment value */
		if (c->incw)
			*w -= *w % c->incw;
		if (c->inch)
			*h -= *h % c->inch;
		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);
		if (c->maxw)
			*w = MIN(*w, c->maxw);
		if (c->maxh)
			*h = MIN(*h, c->maxh);
	}
	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}

void
arrange(Monitor *m)
{
	if (m) {
		showhide(m->stack);
		arrangemon(m);
                restack(m, 1);
        } else
                for (m = mons; m; m = m->next) {
		        showhide(m->stack);
		        arrangemon(m);
                }
}

void
arrangemon(Monitor *m)
{
        updatentiles(m);
	updatebarpos(m);
	XMoveResizeWindow(dpy, m->tabwin, m->wx, m->ty, m->ww, th);
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
	if (m->ntiles > 0 && m->lt[m->sellt]->arrange)
		m->lt[m->sellt]->arrange(m);
}

void
attach(Client *c)
{
	c->next = c->mon->clients;
	c->mon->clients = c;
}

void
attachabove(Client *c)
{
        Client *i;

        if (!c->mon->sel || c->mon->sel->isfloating || c->mon->sel == c->mon->clients) {
                attach(c);
                return;
        }
        for (i = c->mon->clients; i->next != c->mon->sel; i = i->next);
        c->next = i->next;
        i->next = c;
}

void
attachaside(Client *c)
{
        int n;
        Client *i;

        if (!c->mon->nmaster) {
                attach(c);
                return;
        }
        for (n = c->mon->nmaster, i = nexttiled(c->mon->clients);
             n > 1 && i;
             i = nexttiled(i->next), n--);
        if (!i) {
                attachbottom(c);
                return;
        }
        c->next = i->next;
        i->next = c;
}

void
attachbelow(Client *c)
{
        if (!c->mon->sel || c->mon->sel->isfloating) {
                attachbottom(c);
                return;
        }
        c->next = c->mon->sel->next;
        c->mon->sel->next = c;
}

void
attachbottom(Client *c)
{
        Client *i;

        c->next = NULL;
        if (c->mon->clients) {
                for (i = c->mon->clients; i->next; i = i->next);
                i->next = c;
        } else
                c->mon->clients = c;
}

void
attachstack(Client *c)
{
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

void
buttonpress(XEvent *e)
{
        int i, x;
        int dirty = 0;
        unsigned int click;
	Arg arg = {0};
	Client *c;
	Monitor *m;
	XButtonPressedEvent *ev = &e->xbutton;

	/* focus monitor if necessary */
	if ((m = wintomon(ev->window)) && m != selmon) {
                dirty = 1;
		unfocus(selmon->sel);
                updateselmon(m);
		focus(NULL);
	}
	if (ev->window == selmon->barwin) {
                if (ev->x < ble - blw) {
                        i = -1, x = -ev->x;
                        do
                                x += TEXTW(tags[++i]);
                        while (x <= 0);
                        click = ClkTagBar;
                        arg.ui = 1 << i;
                } else if (ev->x < ble)
                        click = ClkLtSymbol;
                else if (ev->x < wsbar - wstext)
                        click = ClkWinTitle;
                else if ((x = wsbar - lrpad / 2 - ev->x) > 0 && (x -= wstext - lrpad) <= 0) {
                        updatedsblockssig(x);
                        if (dirty)
                                return;
                        click = ClkStatusText;
                } else
                        return;
	} else if (ev->window == selmon->tabwin && selmon->ntiles > 0) {
                int ntabs, ofst, tbw, lft;

                if (selmon->lt[selmon->sellt]->arrange == deck &&
                    selmon->pertag->showtabs[selmon->pertag->curtag] == showtab_auto) {
                        ntabs = MIN(selmon->ntiles - selmon->nmaster, MAXTABS);
                        ofst = selmon->nmaster;
                } else {
                        ntabs = MIN(selmon->ntiles, MAXTABS);
                        ofst = 0;
                }
                tbw = selmon->ww / ntabs;
                lft = selmon->ww - tbw * ntabs;
                i = 0, x = -ev->x;
                do
                        x += (++i <= lft) ? tbw + 1 : tbw;
                while (x <= 0);
                click = ClkTabBar;
                arg.i = i + ofst;
        } else if ((c = wintoclient(ev->window))) {
		focus(c);
                restack(selmon, 0);
		XAllowEvents(dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
        } else
                click = ClkRootWin;
	for (i = 0; i < LENGTH(buttons); i++)
		if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
		&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state)){
			buttons[i].func(((click == ClkTagBar || click == ClkTabBar) &&
                                         buttons[i].arg.i == 0) ? &arg : &buttons[i].arg);
		}
}

void
checkotherwm(void)
{
	xerrorxlib = XSetErrorHandler(xerrorstart);
	/* this causes an error if some other window manager is running */
	XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);
	XSync(dpy, False);
	XSetErrorHandler(xerror);
	XSync(dpy, False);
}

void
cleanup(void)
{
	size_t i;
        Arg v = {.ui = ~0}, lt = {.v = &layouts[1]};
        Client *c, *s;
	Monitor *m;

	view(&v);
        setlayout(&lt);
	for (m = mons; m; m = m->next) {
                if (m == selmon)
                        continue;
                while (m->clients) {
                        for (c = m->clients; c->next; c = c->next);
                        sendmon(c, selmon);
                }
        }
        s = NULL;
        for (c = selmon->clients; c; c = c->next) {
                if (c->isfloating <= 0) {
                        c->isfloating = 1;
                        resize(c, c->sfx, c->sfy, c->sfw, c->sfh, 0);
                }
                c->snext = s;
                s = c;
        }
        selmon->stack = s;
        while (selmon->stack)
                unmanage(selmon->stack, 0);
	XUngrabKey(dpy, AnyKey, AnyModifier, root);
	while (mons)
		cleanupmon(mons);
	if (showsystray) {
		XUnmapWindow(dpy, systray->win);
		XDestroyWindow(dpy, systray->win);
		free(systray);
	}
	for (i = 0; i < CurLast; i++)
		drw_cur_free(drw, cursor[i]);
	for (i = 0; i < LENGTH(colors); i++)
		free(scheme[i]);
	XDestroyWindow(dpy, wmcheckwin);
	drw_free(drw);
	XSync(dpy, False);
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}

void
cleanupmon(Monitor *mon)
{
	Monitor *m;

	if (mon == mons)
		mons = mons->next;
	else {
                for (m = mons; m->next != mon; m = m->next);
		m->next = mon->next;
	}
	XUnmapWindow(dpy, mon->barwin);
	XDestroyWindow(dpy, mon->barwin);
	XUnmapWindow(dpy, mon->tabwin);
	XDestroyWindow(dpy, mon->tabwin);
        free(mon->pertag);
	free(mon);
}

void
clientmessage(XEvent *e)
{
	XWindowAttributes wa;
	XSetWindowAttributes swa;
	XClientMessageEvent *cme = &e->xclient;
	Client *c = wintoclient(cme->window);

	if (showsystray && cme->window == systray->win && cme->message_type == netatom[NetSystemTrayOP]) {
		/* add systray icons */
		if (cme->data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
			c = ecalloc(1, sizeof(Client));
			if (!(c->win = cme->data.l[2])) {
				free(c);
				return;
			}
			c->mon = selmon;
			c->next = systray->icons;
			systray->icons = c;
			if (!XGetWindowAttributes(dpy, c->win, &wa)) {
				/* use sane defaults */
				wa.width = SH;
				wa.height = SH;
				wa.border_width = 0;
			}
			c->x = c->oldx = c->y = c->oldy = 0;
			c->w = c->oldw = wa.width;
			c->h = c->oldh = wa.height;
			c->oldbw = wa.border_width;
			c->bw = 0;
                        c->isfloating = 1;
			/* reuse tags field as mapped status */
			c->tags = 1;
			updatesizehints(c);
			updatesystrayicongeom(c, wa.width, wa.height);
			XAddToSaveSet(dpy, c->win);
                        XSelectInput(dpy, c->win, StructureNotifyMask|PropertyChangeMask|ResizeRedirectMask);
			XReparentWindow(dpy, c->win, systray->win, 0, 0);
			/* use parent's background color */
                        swa.background_pixel = scheme[SchemeTray][ColBg].pixel;
			XChangeWindowAttributes(dpy, c->win, CWBackPixel, &swa);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime,
                                  XEMBED_EMBEDDED_NOTIFY, 0, systray->win, XEMBED_EMBEDDED_VERSION);
			/* FIXME: not sure if I have to send these events too */
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime,
                                  XEMBED_FOCUS_IN, 0, systray->win, XEMBED_EMBEDDED_VERSION);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime,
                                  XEMBED_WINDOW_ACTIVATE, 0, systray->win, XEMBED_EMBEDDED_VERSION);
			sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime,
                                  XEMBED_MODALITY_ON, 0, systray->win, XEMBED_EMBEDDED_VERSION);
			XSync(dpy, False);
			resizebarwin(selmon);
			updatesystray();
			setclientstate(c, NormalState);
		}
		return;
	}
	if (!c)
		return;
	if (cme->message_type == netatom[NetWMState]) {
		if (cme->data.l[1] == netatom[NetWMFullscreen]
		|| cme->data.l[2] == netatom[NetWMFullscreen])
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD */
				|| (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
	} else if (cme->message_type == netatom[NetActiveWindow])
                focusclient(c, 0);
/*
		if (c != selmon->sel) {
                        if (c->mon == selmon)
                                focusclient(c, 0);
                        else if (!c->isurgent) {
                                seturgent(c, 1);
                                XSetWindowBorder(dpy, c->win, scheme[SchemeUrg][ColBorder].pixel);
                                drawbar(c->mon);
                                drawtab(c->mon);
                        }
                }
*/
}

void
configure(Client *c)
{
	XConfigureEvent ce;

	ce.type = ConfigureNotify;
	ce.display = dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

void
configurenotify(XEvent *e)
{
	Monitor *m;
	Client *c;
	XConfigureEvent *ev = &e->xconfigure;
	int dirty;

	/* TODO: updategeom handling sucks, needs to be simplified */
	if (ev->window == root) {
		dirty = (sw != ev->width || sh != ev->height);
		sw = ev->width;
		sh = ev->height;
		if (updategeom() || dirty) {
			drw_resize(drw, sw, bh);
			updatebars();
			for (m = mons; m; m = m->next) {
				for (c = m->clients; c; c = c->next)
					if (c->isfullscreen)
						resizeclient(c, m->mx, m->my, m->mw, m->mh);
				resizebarwin(m);
			}
			focus(NULL);
			arrange(NULL);
		}
	}
}

void
configurerequest(XEvent *e)
{
	Client *c;
	Monitor *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;

	if ((c = wintoclient(ev->window))) {
		if (ev->value_mask & CWBorderWidth)
			c->bw = ev->border_width;
		else if (c->isfloating || !selmon->lt[selmon->sellt]->arrange) {
			m = c->mon;
			if (ev->value_mask & CWX) {
				c->oldx = c->x;
				c->x = m->mx + ev->x;
			}
			if (ev->value_mask & CWY) {
				c->oldy = c->y;
				c->y = m->my + ev->y;
			}
			if (ev->value_mask & CWWidth) {
				c->oldw = c->w;
				c->w = ev->width;
			}
			if (ev->value_mask & CWHeight) {
				c->oldh = c->h;
				c->h = ev->height;
			}
			if ((c->x + c->w) > m->mx + m->mw && c->isfloating)
				c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
			if ((c->y + c->h) > m->my + m->mh && c->isfloating)
				c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
			if ((ev->value_mask & (CWX|CWY)) && !(ev->value_mask & (CWWidth|CWHeight)))
				configure(c);
			if (ISVISIBLE(c))
				XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
		} else
			configure(c);
	} else {
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
	}
	XSync(dpy, False);
}

Monitor *
createmon(void)
{
	Monitor *m;
	unsigned int i;

	m = ecalloc(1, sizeof(Monitor));
	m->tagset[0] = m->tagset[1] = 1;
	m->mfact = mfact;
	m->nmaster = nmaster;
	m->showbar = showbar;
	m->topbar = topbar;
	m->toptab = toptab;
	m->lt[0] = m->lt[1] = &layouts[def_layouts[1]];
	strncpy(m->ltsymbol, layouts[def_layouts[1]].symbol, sizeof m->ltsymbol);
	m->pertag = ecalloc(1, sizeof(Pertag));
	m->pertag->curtag = m->pertag->prevtag = 1;

	for (i = 0; i <= LENGTH(tags); i++) {
		m->pertag->nmasters[i] = m->nmaster;
		m->pertag->mfacts[i] = m->mfact;
		m->pertag->ltidxs[i][0] = m->pertag->ltidxs[i][1] = &layouts[def_layouts[i]];
		m->pertag->sellts[i] = m->sellt;
		m->pertag->showbars[i] = m->showbar;
                /* custom */
                m->pertag->attidxs[i][0] = m->pertag->attidxs[i][1] = &attachs[def_attachs[i]];
                m->pertag->showtabs[i] = showtab;
                m->pertag->splus[i][0] = m->pertag->splus[i][1] = m->pertag->splus[i][2] = 0;
	}

	return m;
}

void
deck(Monitor *m)
{
        tiledeck(m, 1);
}

void
destroynotify(XEvent *e)
{
	Client *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;

	if ((c = wintoclient(ev->window)))
		unmanage(c, 1);
	else if ((c = wintosystrayicon(ev->window))) {
		removesystrayicon(c);
		resizebarwin(selmon);
		updatesystray();
	}
}

void
detach(Client *c)
{
	Client **tc;

	for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
	*tc = c->next;
}

void
detachstack(Client *c)
{
	Client **tc, *t;

	for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext);
	*tc = c->snext;

	if (c == c->mon->sel) {
		for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext);
		c->mon->sel = t;
	}
}

/*
Monitor *
dirtomon(int dir)
{
	Monitor *m = NULL;

	if (dir > 0) {
		if (!(m = selmon->next))
			m = mons;
	} else if (selmon == mons)
		for (m = mons; m->next; m = m->next);
	else
		for (m = mons; m->next != selmon; m = m->next);
	return m;
}
*/

void
drawbar(Monitor *m)
{
	int x, w;
        int wbar = m->ww;
	int boxs = drw->fonts->h / 9;
	int boxw = drw->fonts->h / 6 + 2;
	unsigned int i, nhid = 0, occ = 0, urg = 0;
        char halsymbol[43]; /* 10 + 1 + 15 + 1 + 15 + 1 */
	Client *c;

	/* draw status first so it can be overdrawn by tags later */
	if (m == selmon) { /* status is only drawn on focused monitor */
                char *ts = stextc;
                char *tp = stextc;
                char ctmp;

                if (showsystray)
                        wbar -= getsystraywidth();
                wsbar = wbar;
                drw_setscheme(drw, scheme[SchemeStts]);
                x = wbar - wstext;
                drw_rect(drw, x, 0, lrpad / 2, bh, 1, 1); x += lrpad / 2; /* to keep left padding clean */
                for (;;) {
                        if ((unsigned char)*ts > LENGTH(colors) + DELIMITERENDCHAR) {
                                ts++;
                                continue;
                        }
                        ctmp = *ts;
                        *ts = '\0';
                        if (*tp != '\0')
                                x = drw_text(drw, x, 0, TTEXTW(tp), bh, 0, tp, 0);
                        if (ctmp == '\0')
                                break;
                        drw_setscheme(drw, scheme[ctmp - DELIMITERENDCHAR - 1]);
                        *ts = ctmp;
                        tp = ++ts;
                }
                drw_setscheme(drw, scheme[SchemeStts]);
                drw_rect(drw, x, 0, wbar - x, bh, 1, 1); /* to keep right padding clean */
	}

	for (c = m->clients; c; c = c->next) {
                if (c->ishidden && c->tags & m->tagset[m->seltags])
                        nhid++;
                occ |= c->tags;
		if (c->isurgent)
                        urg |= c->tags ? c->tags : m->tagset[m->seltags];
	}
	x = 0;
	for (i = 0; i < LENGTH(tags); i++) {
		w = TEXTW(tags[i]);
                drw_setscheme(drw, scheme[urg & 1 << i ? SchemeUrg :
                                          m->tagset[m->seltags] & 1 << i ? SchemeSel : SchemeNorm]);
		w = drw_text(drw, x, 0, w, bh, lrpad / 2, tags[i], 0);
		if (occ & 1 << i)
			drw_rect(drw, x + boxs, boxs, boxw, boxw,
				m == selmon && selmon->sel && selmon->sel->tags & 1 << i, 0);
		x = w;
	}

        if (nhid)
                snprintf(halsymbol, sizeof halsymbol, "%u %s %s", nhid, ATT(m)->symbol, m->ltsymbol);
        else
                snprintf(halsymbol, sizeof halsymbol, "%s %s", ATT(m)->symbol, m->ltsymbol);
        w = TEXTW(halsymbol);
        drw_setscheme(drw, scheme[SchemeLtSm]);
        x = drw_text(drw, x, 0, w, bh, lrpad / 2, halsymbol, 0);

        if (m == selmon) {
                blw = w, ble = x;
                w = wbar - wstext - lrpad / 2 - x; /* - lrpad / 2 for right padding */
        } else
                w = wbar - lrpad / 2 - x; /* - lrpad / 2 for right padding */

        drw_setscheme(drw, scheme[SchemeNorm]);
        if (m->sel && w > bh) {
                w = drw_text(drw, x, 0, w, bh, lrpad / 2, m->sel->name, 0);
                if (m->sel->isfloating)
                        drw_rect(drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
                drw_rect(drw, w, 0, lrpad / 2, bh, 1, 1); /* to keep right padding clean */
        } else
                drw_rect(drw, x, 0, w + lrpad / 2, bh, 1, 1); /* to keep title area clean */

        XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, wbar, bh);
	drw_map(drw, m->barwin, 0, 0, wbar, bh);
}

void
drawbars(void)
{
	for (Monitor *m = mons; m; m = m->next)
		drawbar(m);
}

void
drawtab(Monitor *m)
{
        if (m->pertag->showtabs[m->pertag->curtag] == showtab_always) {
                updatentiles(m);
                if (m->ntiles == 0) {
                        drw_rect(drw, 0, 0, m->ww, th, 1, 1);
                        drw_map(drw, m->tabwin, 0, 0, m->ww, th);
                } else
                        drawtabhelper(m, 0);
        } else if (m->pertag->showtabs[m->pertag->curtag] == showtab_auto) {
                updatentiles(m);
                if (m->lt[m->sellt]->arrange == monocle && m->ntiles > 1)
                        drawtabhelper(m, 0);
                else if (m->lt[m->sellt]->arrange == deck && m->ntiles > m->nmaster + 1)
                        drawtabhelper(m, 1);
        }
}

void
drawtabhelper(Monitor *m, int onlystack)
{
        int i;
        int ntabs, tbw, lft;
        int x = 0;
        Client *c;

        if (onlystack) {
                ntabs = MIN(m->ntiles - m->nmaster, MAXTABS);
                /* the following loop assumes m->ntiles > m->nmaster */
                for (i = m->nmaster, c = nexttiled(m->clients);
                     i > 0;
                     c = nexttiled(c->next), i--);
        } else {
                ntabs = MIN(m->ntiles, MAXTABS);
                c = nexttiled(m->clients);
        }
        tbw = m->ww / ntabs; /* provisional width for each tab */
        lft = m->ww - tbw * ntabs; /* leftover pixels */
        for (i = 0; i < ntabs; c = nexttiled(c->next), i++) {
                drw_setscheme(drw, scheme[c->isurgent ? SchemeUrg :
                                          c == selmon->sel ? SchemeSel :
                                          i % 2 == 0 ? SchemeNorm : SchemeStts]);
                /* lrpad / 2 below for padding */
                x = drw_text(drw, x, 0, (i < lft ? tbw + 1 : tbw) - lrpad / 2, th, lrpad / 2, c->name, 0);
                drw_rect(drw, x, 0, lrpad / 2, th, 1, 1); x += lrpad / 2; /* to keep right padding clean */
        }
        drw_map(drw, m->tabwin, 0, 0, m->ww, th);
}

void
drawtabs(void) {
	Monitor *m;

	for (m = mons; m; m = m->next)
                drawtab(m);
}

/*
void
enternotify(XEvent *e)
{
	Client *c;
	Monitor *m;
	XCrossingEvent *ev = &e->xcrossing;

	if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root)
		return;
	c = wintoclient(ev->window);
	m = c ? c->mon : wintomon(ev->window);
	if (m != selmon) {
		unfocus(selmon->sel);
                updateselmon(m);
	} else if (!c || c == selmon->sel)
		return;
	focus(c);
}
*/

void
expose(XEvent *e)
{
	Monitor *m;
	XExposeEvent *ev = &e->xexpose;

	if (ev->count == 0 && (m = wintomon(ev->window))) {
		drawbar(m);
		drawtab(m);
                if (showsystray && m == selmon)
			updatesystray();
	}
}

void
focus(Client *c)
{
	if (!c || !ISVISIBLE(c))
		for (c = selmon->stack; c && !ISVISIBLE(c); c = c->snext);
	if (selmon->sel && selmon->sel != c)
		unfocus(selmon->sel);
	if (c) {
		if (c->mon != selmon)
                        updateselmon(c->mon);
		if (c->isurgent)
			seturgent(c, 0);
		detachstack(c);
		attachstack(c);
		grabbuttons(c, 1);
		XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
		setfocus(c);
	} else {
		XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
	}
	selmon->sel = c;
	drawbars();
	drawtabs();
}

/* c must be non-NULL, on selmon and VISIBLE */
void
focusalt(Client *c)
{
        if (selmon->sel && selmon->sel != c)
                unfocus(selmon->sel);
        detachstack(c);
        attachstack(c);
        grabbuttons(c, 1);
        XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
        setfocus(c);
        selmon->sel = c;
        drawbars();
        drawtabs();
}

void
focusclient(Client *c, unsigned int tag)
{
        if (c->mon != selmon) {
                unfocus(selmon->sel);
                updateselmon(c->mon);
        }
        if (c->isurgent)
                seturgent(c, 0);
        if (c->tags & selmon->tagset[selmon->seltags]) {
                if (c->ishidden) {
                        c->ishidden = 0;
                        updateclientdesktop(c);
                        focusalt(c);
                        arrange(selmon);
                } else {
                        focusalt(c);
                        restack(selmon, 0);
                }
                return;
        }
        if (!tag || !(1 << (tag -= 1) & c->tags)) {
                for (tag = 0; tag < LENGTH(tags) && !(1 << tag & c->tags); tag++);
                if (tag >= LENGTH(tags)) {
                        c->tags = selmon->tagset[selmon->seltags];
                        if (c->ishidden)
                                c->ishidden = 0;
                        updateclientdesktop(c);
                        focusalt(c);
                        arrange(selmon);
                        return;
                }
        }
        if (c->ishidden) {
                c->ishidden = 0;
                updateclientdesktop(c);
        }
        selmon->seltags ^= 1;
        selmon->tagset[selmon->seltags] = 1 << tag & TAGMASK;
        resettagifempty(selmon->pertag->curtag);
        selmon->pertag->prevtag = selmon->pertag->curtag;
        selmon->pertag->curtag = tag + 1;
        applycurtagsettings();
        focusalt(c);
        arrange(selmon);
}

/* there are some broken focus acquiring clients needing extra handling */
void
focusin(XEvent *e)
{
	XFocusChangeEvent *ev = &e->xfocus;

	if (selmon->sel && ev->window != selmon->sel->win)
		setfocus(selmon->sel);
}

void
focuslast(const Arg *arg)
{
        Client *c = selmon->sel ? selmon->sel->snext : selmon->stack;

        for (; c && (c->ishidden || !c->tags); c = c->snext);
        if (c)
                focusclient(c, selmon->pertag->prevtag);
}

/*
void
focusmon(const Arg *arg)
{
        Client *c;
	Monitor *m;

	if (!mons->next)
		return;
	if ((m = dirtomon(arg->i)) == selmon)
		return;
	unfocus(selmon->sel);
        updateselmon(m);
	focus(NULL);
}
*/

/*
void
focusstack(const Arg *arg)
{
	Client *c = NULL, *i;

	if (!selmon->sel)
		return;
	if (arg->i > 0) {
		for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
		if (!c)
			for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
	} else {
		for (i = selmon->clients; i != selmon->sel; i = i->next)
			if (ISVISIBLE(i))
				c = i;
		if (!c)
			for (; i; i = i->next)
				if (ISVISIBLE(i))
					c = i;
	}
	if (c) {
		focusalt(c);
		restack(selmon, 0);
	}
}
*/

void
focuswin(const Arg* arg)
{
        int i = arg->i;
        Client *c;

        for (c = nexttiled(selmon->clients); c && i > 1; c = nexttiled(c->next), i--);
        if (c) {
                focusalt(c);
                restack(selmon, 0);
        }
}

Atom
getatomprop(Client *c, Atom prop)
{
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;
	Atom req = XA_ATOM;

	/* FIXME: getatomprop should return the number of items and a pointer to
	 * the stored data instead of this workaround */
	if (prop == xatom[XembedInfo])
		req = xatom[XembedInfo];
	if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, req,
		&da, &di, &dl, &dl, &p) == Success && p) {
		atom = *(Atom *)p;
		if (da == xatom[XembedInfo] && dl == 2)
			atom = ((Atom *)p)[1];
		XFree(p);
	}
	return atom;
}

int
getrootptr(int *x, int *y)
{
	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

int
getwinptr(Window win, int *x, int *y)
{
	int di;
	unsigned int dui;
	Window dummy;

	return XQueryPointer(dpy, win, &dummy, &dummy, &di, &di, x, y, &dui);
}

long
getstate(Window w)
{
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;

	if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState],
		&real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;
	if (n != 0)
		result = *p;
	XFree(p);
	return result;
}

unsigned int
getsystraywidth()
{
	unsigned int w = 0;

        for (Client *i = systray->icons; i; w += i->w + systrayspacing, i = i->next);
	return w ? w + systrayspacing : 1;
}

int
gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
	char **list = NULL;
	int n;
	XTextProperty name;

	if (!text || size == 0)
		return 0;
	text[0] = '\0';
	if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems)
		return 0;
	if (name.encoding == XA_STRING)
		strncpy(text, (char *)name.value, size - 1);
	else {
		if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
			strncpy(text, *list, size - 1);
			XFreeStringList(list);
		}
	}
	text[size - 1] = '\0';
	XFree(name.value);
	return 1;
}

void
grabbuttons(Client *c, int focused)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };

		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		if (!focused)
			XGrabButton(dpy, AnyButton, AnyModifier, c->win, False,
				BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
		for (i = 0; i < LENGTH(buttons); i++)
			if (buttons[i].click == ClkClientWin)
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabButton(dpy, buttons[i].button,
						buttons[i].mask | modifiers[j],
						c->win, False, BUTTONMASK,
						GrabModeAsync, GrabModeSync, None, None);
	}
}

void
grabkeys(void)
{
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
		KeyCode code;

		XUngrabKey(dpy, AnyKey, AnyModifier, root);
		for (i = 0; i < LENGTH(keys); i++)
			if ((code = XKeysymToKeycode(dpy, keys[i].keysym)))
				for (j = 0; j < LENGTH(modifiers); j++)
					XGrabKey(dpy, code, keys[i].mod | modifiers[j], root,
						True, GrabModeAsync, GrabModeAsync);
	}
}

void
incnmaster(const Arg *arg)
{
	selmon->nmaster = selmon->pertag->nmasters[selmon->pertag->curtag] = MAX(selmon->nmaster + arg->i, 0);
	arrange(selmon);
}

#ifdef XINERAMA
static int
isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
	while (n--)
		if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org
		&& unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	return 1;
}
#endif /* XINERAMA */

void
keypress(XEvent *e)
{
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;

	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
	for (i = 0; i < LENGTH(keys); i++)
		if (keysym == keys[i].keysym
		&& CLEANMASK(keys[i].mod) == CLEANMASK(ev->state)
		&& keys[i].func)
			keys[i].func(&(keys[i].arg));
}

void
killclient(const Arg *arg)
{
	if (!selmon->sel)
		return;
	if (!sendevent(selmon->sel->win, wmatom[WMDelete], NoEventMask,
                       wmatom[WMDelete], CurrentTime, 0, 0, 0)) {
		XGrabServer(dpy);
		XSetErrorHandler(xerrordummy);
		XSetCloseDownMode(dpy, DestroyAll);
		XKillClient(dpy, selmon->sel->win);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
}

void
manage(Window w, XWindowAttributes *wa)
{
	Client *c, *t = NULL;
	Window trans = None;
	XWindowChanges wc;

	c = ecalloc(1, sizeof(Client));
	c->win = w;
	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;
        c->bw = borderpx;

	updatetitle(c);
	if (XGetTransientForHint(dpy, w, &trans) && (t = wintoclient(trans))) {
		c->mon = t->mon;
		c->tags = t->tags;
	} else {
		c->mon = selmon;
		applyrules(c);
	}

	if (c->x + WIDTH(c) > c->mon->mx + c->mon->mw)
		c->x = c->mon->mx + c->mon->mw - WIDTH(c);
	if (c->y + HEIGHT(c) > c->mon->my + c->mon->mh)
		c->y = c->mon->my + c->mon->mh - HEIGHT(c);
	c->x = MAX(c->x, c->mon->mx);
	/* only fix client y-offset, if the client center might cover the bar */
	c->y = MAX(c->y, ((c->mon->by == c->mon->my) && (c->x + (c->w / 2) >= c->mon->wx)
		&& (c->x + (c->w / 2) < c->mon->wx + c->mon->ww)) ? bh : c->mon->my);

	wc.border_width = c->bw;
	XConfigureWindow(dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
	configure(c); /* propagates border_width, if size doesn't change */
	updatewindowtype(c);
	updatesizehints(c);
	updatewmhints(c);
	c->sfx = c->x;
	c->sfy = c->y;
	c->sfw = c->w;
	c->sfh = c->h;
	XSelectInput(dpy, w, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
	grabbuttons(c, 0);
	if (!c->isfloating)
		c->isfloating = c->oldstate = trans != None || c->isfixed;
	if (c->isfloating)
		XRaiseWindow(dpy, c->win);
        ATT(c->mon)->attach(c);
	attachstack(c);
	XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
		(unsigned char *) &(c->win), 1);
	XMoveResizeWindow(dpy, c->win, c->x + 2 * sw, c->y, c->w, c->h); /* some windows require this */
	setclientstate(c, NormalState);
	if (c->mon == selmon)
		unfocus(selmon->sel);
	c->mon->sel = c;
	arrange(c->mon);
	XMapWindow(dpy, c->win);
	focus(NULL);
	updateclientdesktop(c);
}

void
mappingnotify(XEvent *e)
{
	XMappingEvent *ev = &e->xmapping;

	XRefreshKeyboardMapping(ev);
	if (ev->request == MappingKeyboard)
		grabkeys();
}

void
maprequest(XEvent *e)
{
	static XWindowAttributes wa;
	Client *i;
	XMapRequestEvent *ev = &e->xmaprequest;

	if ((i = wintosystrayicon(ev->window))) {
		sendevent(i->win, netatom[Xembed], StructureNotifyMask, CurrentTime,
                          XEMBED_WINDOW_ACTIVATE, 0, systray->win, XEMBED_EMBEDDED_VERSION);
		resizebarwin(selmon);
		updatesystray();
	}
	if (!XGetWindowAttributes(dpy, ev->window, &wa))
		return;
	if (wa.override_redirect)
		return;
	if (!wintoclient(ev->window))
		manage(ev->window, &wa);
}

void
monocle(Monitor *m)
{
	Client *c;

        /* override layout symbol */
        snprintf(m->ltsymbol, sizeof m->ltsymbol, "[M%d]", m->ntiles);

        if (m->ntiles == 1) {
                c = nexttiled(m->clients);
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
        } else {
                int wx, wy, ww, wh;

                wx = m->wx + gappoh;
                wy = m->wy + gappih;
                ww = m->ww - 2*gappoh;
                wh = m->wh - 2*gappih;

                for (Client *c = nexttiled(m->clients); c; c = nexttiled(c->next))
                        resize(c, wx, wy, ww - 2 * c->bw, wh - 2 * c->bw, 0);
        }
}

/*
void
motionnotify(XEvent *e)
{
	static Monitor *mon = NULL;
	Monitor *m;
	XMotionEvent *ev = &e->xmotion;

	if (ev->window != root)
		return;
	if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
		unfocus(selmon->sel);
                updateselmon(m);
		focus(NULL);
	}
	mon = m;
}
*/

void
motionnotify(XEvent *e)
{
        int x;
        Monitor *m;
        XMotionEvent *ev = &e->xmotion;

        for (m = mons; m && m->barwin != ev->window; m = m->next);
        if (!m)
                return;
        if (m == selmon && (x = wsbar - lrpad / 2 - ev->x) > 0 && (x -= wstext - lrpad) <= 0)
                updatedsblockssig(x);
        else if (m->statushandcursor) {
                m->statushandcursor = 0;
                XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
        }
}

void
movemouse(const Arg *arg)
{
	int x, y, ocx, ocy, nx, ny;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;
	if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;
        restack(selmon, 1);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if (abs(selmon->wx - nx) < snap)
				nx = selmon->wx;
			else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < snap)
				nx = selmon->wx + selmon->ww - WIDTH(c);
			if (abs(selmon->wy - ny) < snap)
				ny = selmon->wy;
			else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < snap)
				ny = selmon->wy + selmon->wh - HEIGHT(c);
			if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
			&& (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
				togglefloating(NULL);
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
                updateselmon(m);
		sendmon(c, selmon);
	}
}

void
moveprevtag(const Arg *arg)
{
        unsigned long t;

        if (!selmon->sel)
                return;
	selmon->seltags ^= 1;
        if (selmon->pertag->prevtag) {
                selmon->sel->tags = selmon->tagset[selmon->seltags] = 1 << (selmon->pertag->prevtag - 1);
                t = selmon->pertag->prevtag;
        } else {
                selmon->sel->tags = selmon->tagset[selmon->seltags] = ~0 & TAGMASK;
                t = 10;
        }
        XChangeProperty(dpy, selmon->sel->win, netatom[NetWMDesktop], XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char *) &t, 1);
        resettagifempty(selmon->pertag->curtag);
        SWAP(selmon->pertag->prevtag, selmon->pertag->curtag);
        applycurtagsettings();
	arrange(selmon);
}

Client *
nexttiled(Client *c)
{
	for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

void
pop(Client *c)
{
	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}

void
propertynotify(XEvent *e)
{
	Client *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;

	if ((c = wintosystrayicon(ev->window))) {
		if (ev->atom == XA_WM_NORMAL_HINTS) {
			updatesizehints(c);
			updatesystrayicongeom(c, c->w, c->h);
		} else
			updatesystrayiconstate(c, ev);
		resizebarwin(selmon);
		updatesystray();
	}
	if ((ev->window == root) && (ev->atom == XA_WM_NAME))
		updatestatus();
	else if (ev->state == PropertyDelete)
		return; /* ignore */
	else if ((c = wintoclient(ev->window))) {
		switch (ev->atom) {
		default: break;
		case XA_WM_TRANSIENT_FOR:
			if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans)) &&
				(c->isfloating = (wintoclient(trans)) != NULL))
				arrange(c->mon);
			break;
		case XA_WM_NORMAL_HINTS:
			updatesizehints(c);
			break;
		case XA_WM_HINTS:
			updatewmhints(c);
			drawbars();
			drawtabs();
			break;
		}
		if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
			updatetitle(c);
			if (c == c->mon->sel)
				drawbar(c->mon);
			drawtab(c->mon);
		}
		if (ev->atom == netatom[NetWMWindowType])
			updatewindowtype(c);
	}
}

void
quit(const Arg *arg)
{
	if (arg->i)
                restart = 1;
	running = 0;
}

Monitor *
recttomon(int x, int y, int w, int h)
{
	Monitor *m, *r = selmon;
	int a, area = 0;

	for (m = mons; m; m = m->next)
		if ((a = INTERSECT(x, y, w, h, m)) > area) {
			area = a;
			r = m;
		}
	return r;
}

void
removesystrayicon(Client *i)
{
	Client **ii;

	for (ii = &systray->icons; *ii && *ii != i; ii = &(*ii)->next);
        *ii = i->next;
	free(i);
}

void
resettagifempty(unsigned int tag)
{
        Client *c;
        unsigned int tagm = tag ? 1 << (tag - 1) : ~0 & TAGMASK;

        for (c = selmon->clients; c && !(c->tags & tagm); c = c->next);
        if (c)
                return;
        selmon->pertag->nmasters[tag] = nmaster;
        selmon->pertag->mfacts[tag] = mfact;
        selmon->pertag->ltidxs[tag][0] = selmon->pertag->ltidxs[tag][1] = &layouts[def_layouts[tag]];
        selmon->pertag->showbars[tag] = showbar;
        /* custom */
        selmon->pertag->attidxs[tag][0] = selmon->pertag->attidxs[tag][1] = &attachs[def_attachs[tag]];
        selmon->pertag->showtabs[tag] = showtab;
        selmon->pertag->splus[tag][0] = selmon->pertag->splus[tag][1] = selmon->pertag->splus[tag][2] = 0;
}

void
resetsplus(const Arg *arg)
{
        SPLUS(selmon)[0] = SPLUS(selmon)[1] = SPLUS(selmon)[2] = 0;
	if (selmon->ntiles > 0 && selmon->lt[selmon->sellt]->arrange)
                selmon->lt[selmon->sellt]->arrange(selmon);
}

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
	if (applysizehints(c, &x, &y, &w, &h, interact))
		resizeclient(c, x, y, w, h);
}

void
resizebarwin(Monitor *m)
{
	unsigned int w = m->ww;

	if (showsystray && m == selmon)
		w -= getsystraywidth();
	XMoveResizeWindow(dpy, m->barwin, m->wx, m->by, w, bh);
}

void
resizeclient(Client *c, int x, int y, int w, int h)
{
	XWindowChanges wc;

	c->oldx = c->x; c->x = wc.x = x;
	c->oldy = c->y; c->y = wc.y = y;
	c->oldw = c->w; c->w = wc.width = w;
	c->oldh = c->h; c->h = wc.height = h;
	wc.border_width = c->bw;
	XConfigureWindow(dpy, c->win, CWX|CWY|CWWidth|CWHeight|CWBorderWidth, &wc);
	configure(c);
	XSync(dpy, False);
}

void
resizemouse(const Arg *arg)
{
	int ocx, ocy, ocw, och, px, py, nw, nh;
	Client *c;
	Monitor *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;
	if (c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;
        restack(selmon, 1);
	ocx = c->x;
	ocy = c->y;
        ocw = c->w;
        och = c->h;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;
        if (!getwinptr(c->win, &px, &py))
	        return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
			if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
					togglefloating(NULL);
			}
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
        XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, (px * c->w) / ocw , (py * c->h) / och);
	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
                updateselmon(m);
		sendmon(c, selmon);
	}
}

void
resizerequest(XEvent *e)
{
	Client *c;
	XResizeRequestEvent *ev = &e->xresizerequest;

        if ((c = wintosystrayicon(ev->window))) {
		updatesystrayicongeom(c, ev->width, ev->height);
		resizebarwin(selmon);
		updatesystray();
	}
}

void
restack(Monitor *m, int dbr)
{
	Client *c;
	XEvent ev;
	XWindowChanges wc;

        if (dbr) {
                drawbar(m);
                drawtab(m);
        }
	if (!m->sel)
		return;
	if (m->sel->isfloating || !m->lt[m->sellt]->arrange)
		XRaiseWindow(dpy, m->sel->win);
	if (m->lt[m->sellt]->arrange) {
		wc.stack_mode = Below;
		wc.sibling = m->barwin;
		for (c = m->stack; c; c = c->snext)
			if (!c->isfloating && ISVISIBLE(c)) {
				XConfigureWindow(dpy, c->win, CWSibling|CWStackMode, &wc);
				wc.sibling = c->win;
			}
	}
	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

/* Checks for fake signal in root name and removes if there is one, restoring
 * status text. Make sure to include status text after a newline character
 * whenever sending a fake signal. */
void
restorestatus(void)
{
	char curstext[ROOTNAMELENGTH];
        char *newstext;

        if (!gettextprop(root, XA_WM_NAME, curstext, sizeof curstext))
                return;
        if (strncmp(curstext, FSIGID, FSIGIDLEN) != 0)
                return;
        for (newstext = curstext; *newstext != '\n' && *newstext != '\0'; newstext++);
        if (*newstext != '\0')
                XStoreName(dpy, DefaultRootWindow(dpy), ++newstext);
}

void
run(void)
{
	XEvent ev;

	/* main event loop */
	XSync(dpy, False);
	while (running && !XNextEvent(dpy, &ev))
		if (handler[ev.type])
			handler[ev.type](&ev); /* call handler */
}

void
scan(void)
{
	unsigned int i, num;
	Window d1, d2, *wins = NULL;
	XWindowAttributes wa;

	if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
		for (i = 0; i < num; i++) {
			if (!XGetWindowAttributes(dpy, wins[i], &wa)
			|| wa.override_redirect || XGetTransientForHint(dpy, wins[i], &d1))
				continue;
			if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState)
				manage(wins[i], &wa);
		}
		for (i = 0; i < num; i++) { /* now the transients */
			if (!XGetWindowAttributes(dpy, wins[i], &wa))
				continue;
			if (XGetTransientForHint(dpy, wins[i], &d1)
			&& (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
				manage(wins[i], &wa);
		}
		if (wins)
			XFree(wins);
	}
}

void
scratchhide(const Arg *arg)
{
        if (selmon->sel && selmon->sel->scratchkey == arg->i)
                scratchhidehelper();
}

void
scratchhidehelper(void)
{
        unsigned long t = 0;

        selmon->sel->tags = 0;
        XChangeProperty(dpy, selmon->sel->win, netatom[NetWMDesktop], XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char *) &t, 1);
        focus(NULL);
        arrange(selmon);
}

void
scratchshow(const Arg *arg)
{
        if (!selmon->sel || selmon->sel->scratchkey != arg->i)
                scratchshowhelper(arg->i);
}

void
scratchshowhelper(int key)
{
        Client *c;

        for (Monitor *m = mons; m; m = m->next)
                for (c = m->clients; c; c = c->next)
                        if (c->scratchkey == key)
                                goto show;
        spawn(&((Arg){ .v = scratchcmds[key - 1] }));
        return;
show:
        if (c->ishidden)
                c->ishidden = 0;
        if (c->mon != selmon) {
                sendmon(c, selmon);
                return;
        }
        c->tags = selmon->tagset[selmon->seltags];
        updateclientdesktop(c);
        detach(c);
        ATT(c->mon)->attach(c);
        focusalt(c);
        arrange(selmon);
}

void
scratchtoggle(const Arg *arg)
{
        if (selmon->sel && selmon->sel->scratchkey == arg->i)
                scratchhidehelper();
        else
                scratchshowhelper(arg->i);
}

int
sendevent(Window w, Atom proto, int mask, long d0, long d1, long d2, long d3, long d4)
{
	int n;
        int exists = 0;
	Atom *protocols, mt;
	XEvent ev;

	if (proto == wmatom[WMTakeFocus] || proto == wmatom[WMDelete]) {
		mt = wmatom[WMProtocols];
		if (XGetWMProtocols(dpy, w, &protocols, &n)) {
			while (!exists && n--)
				exists = protocols[n] == proto;
			XFree(protocols);
		}
	} else {
                exists = 1;
		mt = proto;
	}
	if (exists) {
		ev.type = ClientMessage;
		ev.xclient.window = w;
		ev.xclient.message_type = mt;
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = d0;
		ev.xclient.data.l[1] = d1;
		ev.xclient.data.l[2] = d2;
		ev.xclient.data.l[3] = d3;
		ev.xclient.data.l[4] = d4;
		XSendEvent(dpy, w, False, mask, &ev);
	}
	return exists;
}

void
sendmon(Client *c, Monitor *m)
{
	unfocus(c);
	detach(c);
	detachstack(c);
	c->mon = m;
	c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
	updateclientdesktop(c);
        ATT(c->mon)->attach(c);
	attachstack(c);
	focus(c);
	arrange(NULL);
}

void
setattach(const Arg *arg)
{
        if (!arg || !arg->v || arg->v != ATT(selmon))
                selmon->pertag->selatts[selmon->pertag->curtag] ^= 1; /* toggle or save the previous */
        if (arg && arg->v)
                ATT(selmon) = (Attach *)arg->v;
        drawbar(selmon);
}

void
setattorprev(const Arg *arg)
{
	setattach(ATT(selmon) == arg->v ? NULL : arg);
}

void
setclientstate(Client *c, long state)
{
	long data[] = { state, None };

	XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32,
		PropModeReplace, (unsigned char *)data, 2);
}

void
setdesktopnames(void)
{
	XTextProperty text;
        char *tagnames[] = { "S -", "N 1", "N 2", "N 3", "N 4", "N 5", "N 6", "N 7", "N 8", "N 9", "N A",
                                    "H 1", "H 2", "H 3", "H 4", "H 5", "H 6", "H 7", "H 8", "H 9", "H A",
                                    "D 1", "D 2", "D 3", "D 4", "D 5", "D 6", "D 7", "D 8", "D 9", "D A" };

	Xutf8TextListToTextProperty(dpy, tagnames, LENGTH(tagnames), XUTF8StringStyle, &text);
	XSetTextProperty(dpy, root, &text, netatom[NetDesktopNames]);
}

void
setfocus(Client *c)
{
	if (!c->neverfocus) {
		XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dpy, root, netatom[NetActiveWindow],
			XA_WINDOW, 32, PropModeReplace,
			(unsigned char *) &(c->win), 1);
	}
	sendevent(c->win, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

void
setfullscreen(Client *c, int fullscreen)
{
	if (fullscreen && !c->isfullscreen) {
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)&netatom[NetWMFullscreen], 1);
		c->isfullscreen = 1;
		c->oldstate = c->isfloating;
		c->oldbw = c->bw;
		c->bw = 0;
		c->isfloating = 1;
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
		XRaiseWindow(dpy, c->win);
	} else if (!fullscreen && c->isfullscreen){
		XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32,
			PropModeReplace, (unsigned char*)0, 0);
		c->isfullscreen = 0;
		c->isfloating = c->oldstate;
		c->bw = c->oldbw;
		c->x = c->oldx;
		c->y = c->oldy;
		c->w = c->oldw;
		c->h = c->oldh;
		resizeclient(c, c->x, c->y, c->w, c->h);
		arrange(c->mon);
	}
}

void
setlayout(const Arg *arg)
{
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt = selmon->pertag->sellts[selmon->pertag->curtag] ^= 1;
	if (arg && arg->v)
		selmon->lt[selmon->sellt] =
                        selmon->pertag->ltidxs[selmon->pertag->curtag][selmon->sellt] =
                                (Layout *)arg->v;
        if (selmon->lt[selmon->sellt]->attach != ATT(selmon)) {
                selmon->pertag->selatts[selmon->pertag->curtag] ^= 1;
                ATT(selmon) = selmon->lt[selmon->sellt]->attach;
        }
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if (selmon->sel)
		arrange(selmon);
	else
		drawbar(selmon);
}

void
setltorprev(const Arg *arg)
{
	setlayout(selmon->lt[selmon->sellt] == arg->v ? NULL : arg);
}

/* arg > 1.0 will set mfact absolutely */
void
setmfact(const Arg *arg)
{
	float f;

	if (!selmon->lt[selmon->sellt]->arrange || selmon->lt[selmon->sellt]->arrange == monocle)
		return;
	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95)
		return;
	selmon->mfact = selmon->pertag->mfacts[selmon->pertag->curtag] = f;
	arrange(selmon);
}

void
setsplus(const Arg *arg)
{
        int selidx = 0;

	if (!selmon->lt[selmon->sellt]->arrange || selmon->lt[selmon->sellt]->arrange == monocle)
		return;
        if (!selmon->sel)
                return;
        for (Client *c = nexttiled(selmon->clients); c != selmon->sel; c = nexttiled(c->next), selidx++);
        if (selmon->lt[selmon->sellt]->arrange == tile) {
                if (selidx < selmon->nmaster) {
                        if (MIN(selmon->nmaster, selmon->ntiles) > 1)
                                SPLUS(selmon)[0] = arg->i == 0 ? 0 : MAX(SPLUS(selmon)[0] + arg->i, 0);
                        else
                                return;
                } else if ((selmon->ntiles - selmon->nmaster) > 1) {
                        if ((selmon->ntiles - selmon->nmaster) > 2 && selidx == selmon->nmaster + 1)
                                SPLUS(selmon)[2] = arg->i == 0 ? 0 : MAX(SPLUS(selmon)[2] + arg->i, 0);
                        else {
                                SPLUS(selmon)[1] = arg->i == 0 ? 0 : MAX(SPLUS(selmon)[1] + arg->i, 0);
                        }
                } else
                        return;
        } else {
                if (MIN(selmon->nmaster, selmon->ntiles) > 1)
                        SPLUS(selmon)[0] = arg->i == 0 ? 0 : MAX(SPLUS(selmon)[0] + arg->i, 0);
                else
                        return;
        }
	if (selmon->ntiles > 0 && selmon->lt[selmon->sellt]->arrange)
                selmon->lt[selmon->sellt]->arrange(selmon);
}

void
setup(void)
{
	int i;
	XSetWindowAttributes wa;
	Atom utf8string;

	/* clean up any zombies immediately */
	sigchld(0);

        /* be the child subreaper */
        if (prctl(PR_SET_CHILD_SUBREAPER, 1) == -1)
		fputs("warning: could not set dwm as the subreaper\n", stderr);

	/* init screen */
	screen = DefaultScreen(dpy);
	sw = DisplayWidth(dpy, screen);
	sh = DisplayHeight(dpy, screen);
	root = RootWindow(dpy, screen);
	drw = drw_create(dpy, screen, root, sw, sh);
	if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");
	lrpad = drw->fonts->h;
	bh = drw->fonts->h + 2;
	th = bh;
	updategeom();
	/* init atoms */
	utf8string = XInternAtom(dpy, "UTF8_STRING", False);
	wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
	wmatom[WMDelete] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	wmatom[WMState] = XInternAtom(dpy, "WM_STATE", False);
	wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);
	netatom[NetActiveWindow] = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	netatom[NetSupported] = XInternAtom(dpy, "_NET_SUPPORTED", False);
	netatom[NetSystemTray] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_S0", False);
	netatom[NetSystemTrayOP] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
	netatom[NetSystemTrayOrientation] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION", False);
	netatom[NetSystemTrayOrientationHorz] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
	netatom[NetWMName] = XInternAtom(dpy, "_NET_WM_NAME", False);
	netatom[NetWMState] = XInternAtom(dpy, "_NET_WM_STATE", False);
	netatom[NetWMCheck] = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	netatom[NetWMFullscreen] = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
	netatom[NetWMWindowType] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	netatom[NetWMWindowTypeDialog] = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	netatom[NetDesktopNames] = XInternAtom(dpy, "_NET_DESKTOP_NAMES", False);
	netatom[NetWMDesktop] = XInternAtom(dpy, "_NET_WM_DESKTOP", False);
	netatom[NetClientList] = XInternAtom(dpy, "_NET_CLIENT_LIST", False);
	xatom[Manager] = XInternAtom(dpy, "MANAGER", False);
	xatom[Xembed] = XInternAtom(dpy, "_XEMBED", False);
	xatom[XembedInfo] = XInternAtom(dpy, "_XEMBED_INFO", False);
	/* init cursors */
	cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
        cursor[CurHand] = drw_cur_create(drw, XC_hand2);
        cursor[CurResize] = drw_cur_create(drw, XC_bottom_right_corner);
	cursor[CurMove] = drw_cur_create(drw, XC_fleur);
	/* init appearance */
	scheme = ecalloc(LENGTH(colors), sizeof(Clr *));
	for (i = 0; i < LENGTH(colors); i++)
		scheme[i] = drw_scm_create(drw, colors[i], 3);
	/* init system tray */
        if (showsystray)
                updatesystray();
	/* init bars */
	updatebars();
	updatestatus();
	/* supporting window for NetWMCheck */
	wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8,
		PropModeReplace, (unsigned char *) "dwm", 3);
	XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32,
		PropModeReplace, (unsigned char *) &wmcheckwin, 1);
	/* EWMH support per view */
	XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32,
		PropModeReplace, (unsigned char *) netatom, NetLast);
	setdesktopnames();
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	/* select events */
	wa.cursor = cursor[CurNormal]->cursor;
	wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask
		|ButtonPressMask|PointerMotionMask|EnterWindowMask
		|LeaveWindowMask|StructureNotifyMask|PropertyChangeMask;
	XChangeWindowAttributes(dpy, root, CWEventMask|CWCursor, &wa);
	XSelectInput(dpy, root, wa.event_mask);
	grabkeys();
	focus(NULL);
}

void
seturgent(Client *c, int urg)
{
	XWMHints *wmh;

	c->isurgent = urg;
	if (!(wmh = XGetWMHints(dpy, c->win)))
		return;
	wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
	XSetWMHints(dpy, c->win, wmh);
	XFree(wmh);
}

void
shifttag(const Arg *arg)
{
        Arg shifted;

        if (!selmon->pertag->curtag)
                return;
        if (arg->i > 0)
                shifted.ui = selmon->pertag->curtag == LENGTH(tags) ?
                        1 << 0 : 1 << (selmon->pertag->curtag);
        else
                shifted.ui = selmon->pertag->curtag == 1 ?
                        1 << (LENGTH(tags) - 1) : 1 << (selmon->pertag->curtag - 2);
        view(&shifted);
}

void
shiftview(const Arg *arg)
{
        unsigned int activetags = 0;
        Arg shifted;

        if (!selmon->pertag->curtag)
                return;
        for (Client *c = selmon->clients; c; c = c->next)
                activetags |= c->tags;
        if (!activetags || activetags == (shifted.ui = 1 << (selmon->pertag->curtag - 1)))
                return;
        if (arg->i > 0)
                do
                        shifted.ui = shifted.ui << 1 | (shifted.ui >> (LENGTH(tags) - 1));
                while (!(shifted.ui & activetags));
        else
                do
                        shifted.ui = shifted.ui >> 1 | (shifted.ui << (LENGTH(tags) - 1));
                while (!(shifted.ui & activetags));
        view(&shifted);
}

void
showhide(Client *c)
{
	if (!c)
		return;
	if (ISVISIBLE(c)) {
		/* show clients top down */
		XMoveWindow(dpy, c->win, c->x, c->y);
		if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
			resize(c, c->x, c->y, c->w, c->h, 0);
		showhide(c->snext);
	} else {
		/* hide clients bottom up */
		showhide(c->snext);
		XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);
	}
}

void
sigchld(int unused)
{
	if (signal(SIGCHLD, sigchld) == SIG_ERR)
		die("can't install SIGCHLD handler:");
	while (0 < waitpid(-1, NULL, WNOHANG));
}

void
sigdsblocks(const Arg *arg)
{
        int fd;
        struct flock fl;
        union sigval sv;

        if (!dsblockssig)
                return;
        sv.sival_int = (dsblockssig << 8) | arg->i;
        fd = open(DSBLOCKSLOCKFILE, O_RDONLY);
        if (fd == -1)
                return;
        fl.l_type = F_WRLCK;
        fl.l_start = 0;
        fl.l_whence = SEEK_SET;
        fl.l_len = 0;
        if (fcntl(fd, F_GETLK, &fl) == -1 || fl.l_type == F_UNLCK)
                return;
        sigqueue(fl.l_pid, SIGRTMIN, sv);
}

void
spawn(const Arg *arg)
{
	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();
		execvp(((char **)arg->v)[0], (char **)arg->v);
		fprintf(stderr, "dwm: execvp %s", ((char **)arg->v)[0]);
		perror(" failed");
		exit(EXIT_SUCCESS);
	}
}

void
swaptags(const Arg *arg)
{
        unsigned int newtags = 1 << arg->ui;
        unsigned long newtag;
        Client* c;

        if (newtags & selmon->tagset[selmon->seltags])
                return;
        newtag = arg->ui + 1;
        if (selmon->pertag->curtag) {
                unsigned int curtags = 1 << (selmon->pertag->curtag - 1);

                for (c = selmon->clients; c; c = c->next)
                        if (c->tags & newtags) {
                                c->tags = (c->tags ^ newtags) | curtags;
                                updateclientdesktop(c);
                        } else if (c->tags & curtags) {
                                c->tags = (c->tags ^ curtags) | newtags;
                                updateclientdesktop(c);
                        }
                newtags |= selmon->tagset[selmon->seltags] ^ curtags;
        } else
                for (c = selmon->clients; c; c = c->next) {
                        c->tags = newtags;
                        XChangeProperty(dpy, c->win, netatom[NetWMDesktop], XA_CARDINAL, 32,
                                        PropModeReplace, (unsigned char *) &newtag, 1);
                }
        selmon->seltags ^= 1;
        selmon->tagset[selmon->seltags] = newtags;
        /* pertag swaps */
        SWAP(selmon->pertag->nmasters[newtag],
             selmon->pertag->nmasters[selmon->pertag->curtag]);
        SWAP(selmon->pertag->mfacts[newtag],
             selmon->pertag->mfacts[selmon->pertag->curtag]);
        SWAP(selmon->pertag->sellts[newtag],
             selmon->pertag->sellts[selmon->pertag->curtag]);
        SWAP(selmon->pertag->ltidxs[newtag][0],
             selmon->pertag->ltidxs[selmon->pertag->curtag][0]);
        SWAP(selmon->pertag->ltidxs[newtag][1],
             selmon->pertag->ltidxs[selmon->pertag->curtag][1]);
        SWAP(selmon->pertag->showbars[newtag],
             selmon->pertag->showbars[selmon->pertag->curtag]);
        /* custom pertag swaps */
        SWAP(selmon->pertag->selatts[newtag],
             selmon->pertag->selatts[selmon->pertag->curtag]);
        SWAP(selmon->pertag->attidxs[newtag][0],
             selmon->pertag->attidxs[selmon->pertag->curtag][0]);
        SWAP(selmon->pertag->attidxs[newtag][1],
             selmon->pertag->attidxs[selmon->pertag->curtag][1]);
        SWAP(selmon->pertag->showtabs[newtag],
             selmon->pertag->showtabs[selmon->pertag->curtag]);
        SWAP(selmon->pertag->splus[newtag][0],
             selmon->pertag->splus[selmon->pertag->curtag][0]);
        SWAP(selmon->pertag->splus[newtag][1],
             selmon->pertag->splus[selmon->pertag->curtag][1]);
        SWAP(selmon->pertag->splus[newtag][2],
             selmon->pertag->splus[selmon->pertag->curtag][2]);
        /* change curtag */
        selmon->pertag->curtag = newtag;
        drawbar(selmon);
}

void
tabmode(const Arg *arg)
{
	if (arg->i >= 0)
                selmon->pertag->showtabs[selmon->pertag->curtag] = arg->ui % showtab_nmodes;
	else
                selmon->pertag->showtabs[selmon->pertag->curtag] =
                        (selmon->pertag->showtabs[selmon->pertag->curtag] + 1 ) % showtab_nmodes;
	arrange(selmon);
}

void
tag(const Arg *arg)
{
	if (selmon->sel && arg->ui & TAGMASK) {
		selmon->sel->tags = arg->ui & TAGMASK;
		updateclientdesktop(selmon->sel);
		focus(NULL);
		arrange(selmon);
	}
}

void
tagandview(const Arg *arg)
{
        if (selmon->sel && (1 << arg->ui) != selmon->tagset[selmon->seltags]) {
                unsigned long t = arg->ui + 1;

                selmon->seltags ^= 1;
                selmon->sel->tags = selmon->tagset[selmon->seltags] = 1 << arg->ui;
                XChangeProperty(dpy, selmon->sel->win, netatom[NetWMDesktop], XA_CARDINAL, 32,
                                PropModeReplace, (unsigned char *) &t, 1);
                resettagifempty(selmon->pertag->curtag);
                selmon->pertag->prevtag = selmon->pertag->curtag;
                selmon->pertag->curtag = t;
                applycurtagsettings();
                focus(selmon->sel);
                arrange(selmon);
        }
}

/*
void
tagmon(const Arg *arg)
{
        Monitor *m;

	if (!selmon->sel || !mons->next)
		return;
        if ((m = dirtomon(arg->i)) != selmon)
                sendmon(selmon->sel, m);
}
*/

void
tile(Monitor *m)
{
        tiledeck(m, 0);
}

void
tiledeck(Monitor *m, int deck)
{
	Client *c;

        if (m->ntiles == 1) {
                c = nexttiled(m->clients);
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
        } else {
                int x, y, w, h, wx, wy, ww, wh;
                int r;

                wx = m->wx + gappoh;
                wy = m->wy + gappov;
                ww = m->ww - 2*gappoh;
                wh = m->wh - 2*gappov;

                /* masters */
                w = m->ntiles > m->nmaster ? ww * m->mfact : ww;
                r = MIN(m->ntiles, m->nmaster);
                c = nexttiled(m->clients);

                if (r > 1 && SPLUS(m)[0]) {
                        int h1, h2, hleft;

                        /* first master */
                        hleft = wh - gappih * (r - 1);
                        h = hleft / r; /* height without including splus */
                        h1 = h + SPLUS(m)[0]; /* provisional height after including splus */
                        h2 = MAX(h, hleft - MINWINHEIGHT * (r - 1)); /* maximum allowed height */
                        if (h1 > h2) {
                                SPLUS(m)[0] = h2 - h;
                                h = h2;
                        } else
                                h = h1;
                        resize(c, wx, wy, w - (2 * c->bw), h - (2 * c->bw), 0);
                        y = HEIGHT(c) + gappih;
                        c = nexttiled(c->next);
                        r--;
                        /* rest of the masters */
                        for (; r; c = nexttiled(c->next), r--) {
                                h = (wh - y - gappih * (r - 1)) / r;
                                resize(c, wx, wy + y, w - (2 * c->bw), h - (2 * c->bw), 0);
                                y += HEIGHT(c) + gappih;
                        }
                } else {
                        y = 0;
                        for (; r; c = nexttiled(c->next), r--) {
                                h = (wh - y - gappih * (r - 1)) / r;
                                resize(c, wx, wy + y, w - (2 * c->bw), h - (2 * c->bw), 0);
                                y += HEIGHT(c) + gappih;
                        }
                }

                /* stack */
                if (m->ntiles > m->nmaster)
                        r = m->ntiles - m->nmaster;
                else
                        return;

                x = m->nmaster ? wx + w + gappiv : wx;
                w = ww - x + wx;

                if (deck) {
                        /* override layout symbol */
                        snprintf(m->ltsymbol, sizeof m->ltsymbol, "[D%d]", r);

                        for (; c; c = nexttiled(c->next))
                                resize(c, x, wy, w - (2 * c->bw), wh - (2 * c->bw), 0);
                        return;
                }

                if (r > 1 && (SPLUS(m)[1] || SPLUS(m)[2])) {
                        int h1, h2, hleft;

                        if (r > 2) {
                                y = 0;
                                /* first two stack clients */
                                for (int i = 1; i < 3; c = nexttiled(c->next), r--, i++) {
                                        hleft = wh - y - gappih * (r - 1);
                                        h = hleft / r; /* height without including splus */
                                        h1 = h + SPLUS(m)[i]; /* provisional height after including splus */
                                        h2 = MAX(h, hleft - MINWINHEIGHT * (r - 1)); /* maximum allowed height */
                                        if (h1 > h2) {
                                                SPLUS(m)[i] = h2 - h;
                                                h = h2;
                                        } else
                                                h = h1;
                                        resize(c, x, wy + y, w - (2*c->bw), h - (2*c->bw), 0);
                                        y += HEIGHT(c) + gappih;
                                }
                                /* rest of the stack clients */
                                for (; r; c = nexttiled(c->next), r--) {
                                        h = (wh - y - gappih * (r - 1)) / r;
                                        resize(c, x, wy + y, w - (2 * c->bw), h - (2 * c->bw), 0);
                                        y += HEIGHT(c) + gappih;
                                }
                        } else {
                                /* first stack client */
                                hleft = wh - gappih;
                                h = hleft / 2; /* height without including splus */
                                h1 = h + SPLUS(m)[1]; /* provisional height after including splus */
                                h2 = MAX(h, hleft - MINWINHEIGHT); /* maximum allowed height */
                                if (h1 > h2) {
                                        SPLUS(m)[1] = h2 - h;
                                        h = h2;
                                } else
                                        h = h1;
                                resize(c, x, wy, w - (2 * c->bw), h - (2 * c->bw), 0);
                                y = HEIGHT(c) + gappih;
                                c = nexttiled(c->next);
                                /* last stack client */
                                h = (wh - y);
                                resize(c, x, wy + y, w - (2 * c->bw), h - (2 * c->bw), 0);
                        }
                } else {
                        y = 0;
                        for (; r; c = nexttiled(c->next), r--) {
                                h = (wh - y - gappih * (r - 1)) / r;
                                resize(c, x, wy + y, w - (2 * c->bw), h - (2 * c->bw), 0);
                                y += HEIGHT(c) + gappih;
                        }
                }
        }
}

void
togglebar(const Arg *arg)
{
	selmon->showbar = selmon->pertag->showbars[selmon->pertag->curtag] = !selmon->showbar;
	updatebarpos(selmon);
	resizebarwin(selmon);
	if (showsystray) {
		XWindowChanges wc;

                wc.y = selmon->showbar ? (selmon->topbar ? 0 : selmon->mh - bh) : -bh;
		XConfigureWindow(dpy, systray->win, CWY, &wc);
	}
	arrange(selmon);
}

void
togglefloating(const Arg *arg)
{
	if (!selmon->sel)
		return;
	if (selmon->sel->isfullscreen) /* no support for fullscreen windows */
		return;
	selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
        if (arg && arg->i) {
                if (selmon->sel->isfloating)
                        /* restore last known float dimensions */
                        resize(selmon->sel, selmon->sel->sfx, selmon->sel->sfy,
                               selmon->sel->sfw, selmon->sel->sfh, 0);
                else {
                        /* save current dimensions before resizing */
                        selmon->sel->sfx = selmon->sel->x;
                        selmon->sel->sfy = selmon->sel->y;
                        selmon->sel->sfw = selmon->sel->w;
                        selmon->sel->sfh = selmon->sel->h;
                }
        } else
                if (selmon->sel->isfloating) {
                        selmon->sel->isfloating = -1;
                        resize(selmon->sel, selmon->sel->x, selmon->sel->y,
                                selmon->sel->w, selmon->sel->h, 0);
                }
	arrange(selmon);
}

void
toggletag(const Arg *arg)
{
	unsigned int newtags;

	if (!selmon->sel)
		return;
	newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		selmon->sel->tags = newtags;
                updateclientdesktop(selmon->sel);
		focus(NULL);
		arrange(selmon);
	}
}

void
toggleview(const Arg *arg)
{
        int i;
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);

        if (!newtagset)
                return;
        if (!(selmon->tagset[selmon->seltags] & arg->ui)) {
                /* clients in the master area should be the same after we add a new tag */
                Client **const masters = ecalloc(selmon->nmaster, sizeof(Client *));

                /* collect (from last to first) references to all clients in the master area */
                i = selmon->nmaster - 1;
                for (Client *c = nexttiled(selmon->clients); c && i >= 0; c = nexttiled(c->next), i--)
                        masters[i] = c;
                /* put the master clients at the front of the list */
                for (i = 0; i < selmon->nmaster; i++)
                        if (masters[i]) {
                                detach(masters[i]);
                                attach(masters[i]);
                        }
                free(masters);

                if (newtagset == (~0 & TAGMASK)) {
                        resettagifempty(selmon->pertag->curtag);
                        selmon->pertag->prevtag = selmon->pertag->curtag;
                        selmon->pertag->curtag = 0;
                }
        } else
                if (!selmon->pertag->curtag || !(newtagset & 1 << (selmon->pertag->curtag - 1))) {
                        resettagifempty(selmon->pertag->curtag);
                        selmon->pertag->prevtag = selmon->pertag->curtag;
                        for (i = 0; !(newtagset & 1 << i); i++);
                        selmon->pertag->curtag = i + 1;
                }
        selmon->tagset[selmon->seltags] = newtagset;
        applycurtagsettings();
        focus(NULL);
        arrange(selmon);
}

void
togglewin(const Arg *arg)
{
        Client *c;

        if (selmon->sel && selmon->sel->scratchkey == ((Win*)(arg->v))->scratchkey) {
                for (c = selmon->sel->snext; c && (c->ishidden || !c->tags); c = c->snext);
                if (c)
                        focusclient(c, selmon->pertag->prevtag);
                else
                        view(&((Arg){0}));
        } else {
                for (Monitor *m = mons; m; m = m->next)
                        for (c = m->clients; c; c = c->next)
                                if (c->scratchkey == ((Win*)(arg->v))->scratchkey)
                                        goto show;
                view(&((Arg){ .ui = 1 << (((Win*)(arg->v))->tag - 1) }));
                spawn(&((Win*)(arg->v))->cmd);
                return;
show:
                if (c->mon == selmon)
                        focusclient(c, ((Win*)(arg->v))->tag);
                else {
                        view(&((Arg){ .ui = 1 << (((Win*)(arg->v))->tag - 1) }));
                        sendmon(c, selmon);
                }
        }
}

void
unfocus(Client *c)
{
	if (!c)
		return;
	grabbuttons(c, 0);
	XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);
}

void
unmanage(Client *c, int destroyed)
{
	Monitor *m = c->mon;
	XWindowChanges wc;

	detach(c);
	detachstack(c);
	if (!destroyed) {
		wc.border_width = c->oldbw;
		XGrabServer(dpy); /* avoid race conditions */
		XSetErrorHandler(xerrordummy);
		XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dpy, False);
		XSetErrorHandler(xerror);
		XUngrabServer(dpy);
	}
	free(c);
	focus(NULL);
	updateclientlist();
	arrange(m);
}

void
unmapnotify(XEvent *e)
{
	Client *c;
	XUnmapEvent *ev = &e->xunmap;

	if ((c = wintoclient(ev->window))) {
		if (ev->send_event)
			setclientstate(c, WithdrawnState);
		else
			unmanage(c, 0);
	} else if ((c = wintosystrayicon(ev->window))) {
		/* KLUDGE! sometimes icons occasionally unmap their windows, but do
		 * not destroy them. We map those windows back */
		XMapRaised(dpy, c->win);
		updatesystray();
	}
}

void
updatebarpos(Monitor *m)
{
	m->wy = m->my;
	m->wh = m->mh;
	if (m->showbar) {
		m->wh -= bh;
		m->by = m->topbar ? m->wy : m->wy + m->wh;
		if ( m->topbar )
			m->wy += bh;
	} else {
		m->by = -bh;
	}

        if (m->pertag->showtabs[m->pertag->curtag] == showtab_always ||
            (m->pertag->showtabs[m->pertag->curtag] == showtab_auto &&
             ((m->lt[m->sellt]->arrange == monocle && m->ntiles > 1) ||
              (m->lt[m->sellt]->arrange == deck && m->ntiles > m->nmaster + 1)))) {
		m->wh -= th;
		m->ty = m->toptab ? m->wy : m->wy + m->wh;
		if ( m->toptab )
			m->wy += th;
	} else {
		m->ty = -th;
	}
}

void
updatebars(void)
{
	int w;
	Monitor *m;
	XSetWindowAttributes wab = {
		.override_redirect = True,
		.background_pixmap = ParentRelative,
		.event_mask = ButtonPressMask|ExposureMask|PointerMotionMask
	};
	XSetWindowAttributes wat = {
		.override_redirect = True,
		.background_pixmap = ParentRelative,
		.event_mask = ButtonPressMask|ExposureMask
	};
	XClassHint ch = {"dwm", "dwm"};

	for (m = mons; m; m = m->next) {
		if (m->barwin)
			continue;
		w = m->ww;
		if (showsystray && m == selmon)
			w -= getsystraywidth();
		m->barwin = XCreateWindow(dpy, root, m->wx, m->by, w, bh, 0, DefaultDepth(dpy, screen),
				CopyFromParent, DefaultVisual(dpy, screen),
				CWOverrideRedirect|CWBackPixmap|CWEventMask, &wab);
		XDefineCursor(dpy, m->barwin, cursor[CurNormal]->cursor);
		XMapRaised(dpy, m->barwin);
		m->tabwin = XCreateWindow(dpy, root, m->wx, m->ty, m->ww, th, 0, DefaultDepth(dpy, screen),
                                CopyFromParent, DefaultVisual(dpy, screen),
                                CWOverrideRedirect|CWBackPixmap|CWEventMask, &wat);
		XDefineCursor(dpy, m->tabwin, cursor[CurNormal]->cursor);
		XMapRaised(dpy, m->tabwin);
		XSetClassHint(dpy, m->barwin, &ch);
		XSetClassHint(dpy, m->tabwin, &ch);
	}
}

void
updateclientdesktop(Client *c)
{
        unsigned long t;

        if (c->tags == (~0 & TAGMASK))
                t = 10;
        else if (selmon->pertag->curtag && c->tags & 1 << (selmon->pertag->curtag - 1))
                t = selmon->pertag->curtag;
        else {
                for (t = 0; t < LENGTH(tags) && !(1 << t & c->tags); t++);
                if (++t > LENGTH(tags)) {
                        t = 0;
                        goto update;
                }
        }
        if (c->mon != selmon)
                t += 20;
        else if (c->ishidden)
                t += 10;
update:
	XChangeProperty(dpy, c->win, netatom[NetWMDesktop], XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *) &t, 1);
}

void
updateclientlist(void)
{
	Client *c;
	Monitor *m;

	XDeleteProperty(dpy, root, netatom[NetClientList]);
	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			XChangeProperty(dpy, root, netatom[NetClientList],
				XA_WINDOW, 32, PropModeAppend,
				(unsigned char *) &(c->win), 1);
}

void
updatedsblockssig(int x)
{
        char *ts = stexts;
        char *tp = stexts;
        char ctmp;

        while (*ts != '\0') {
                if ((unsigned char)*ts > DELIMITERENDCHAR) {
                        ts++;
                        continue;
                }
                ctmp = *ts;
                *ts = '\0';
                x += TTEXTW(tp);
                *ts = ctmp;
                if (x > 0) {
                        if (ctmp == DELIMITERENDCHAR)
                                goto cursorondelim;
                        if (!selmon->statushandcursor) {
                                selmon->statushandcursor = 1;
                                XDefineCursor(dpy, selmon->barwin, cursor[CurHand]->cursor);
                        }
                        dsblockssig = ctmp;
                        return;
                }
                tp = ++ts;
        }
cursorondelim:
        if (selmon->statushandcursor) {
                selmon->statushandcursor = 0;
                XDefineCursor(dpy, selmon->barwin, cursor[CurNormal]->cursor);
        }
        dsblockssig = 0;
}

int
updategeom(void)
{
	int dirty = 0;
        Monitor *p = selmon;

#ifdef XINERAMA
	if (XineramaIsActive(dpy)) {
		int i, j, n, nn;
		Client *c;
		Monitor *m;
		XineramaScreenInfo *info = XineramaQueryScreens(dpy, &nn);
		XineramaScreenInfo *unique = NULL;

		for (n = 0, m = mons; m; m = m->next, n++);
		/* only consider unique geometries as separate screens */
		unique = ecalloc(nn, sizeof(XineramaScreenInfo));
		for (i = 0, j = 0; i < nn; i++)
			if (isuniquegeom(unique, j, &info[i]))
				memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
		XFree(info);
		nn = j;
		if (n <= nn) { /* new monitors available */
			for (i = 0; i < (nn - n); i++) {
                                if (mons) {
                                        for (m = mons; m->next; m = m->next);
					m->next = createmon();
                                } else
					mons = createmon();
			}
			for (i = 0, m = mons; i < nn && m; m = m->next, i++)
				if (i >= n
				|| unique[i].x_org != m->mx || unique[i].y_org != m->my
				|| unique[i].width != m->mw || unique[i].height != m->mh)
				{
					dirty = 1;
					m->num = i;
					m->mx = m->wx = unique[i].x_org;
					m->my = m->wy = unique[i].y_org;
					m->mw = m->ww = unique[i].width;
					m->mh = m->wh = unique[i].height;
					updatebarpos(m);
				}
		} else { /* less monitors available nn < n */
			for (i = nn; i < n; i++) {
                                for (m = mons; m->next; m = m->next);
				while ((c = m->clients)) {
					dirty = 1;
					m->clients = c->next;
					detachstack(c);
					c->mon = mons;
                                        ATT(c->mon)->attach(c);
					attachstack(c);
				}
				if (m == selmon)
					selmon = mons;
				cleanupmon(m);
			}
		}
		free(unique);
	} else
#endif /* XINERAMA */
	{ /* default monitor setup */
		if (!mons)
			mons = createmon();
		if (mons->mw != sw || mons->mh != sh) {
			dirty = 1;
			mons->mw = mons->ww = sw;
			mons->mh = mons->wh = sh;
			updatebarpos(mons);
		}
	}
	if (dirty) {
		selmon = mons;
		selmon = wintomon(root);
	}
        if (selmon != p) {
                Client *c;

                if (p)
                        for (c = p->clients; c; c = c->next)
                                updateclientdesktop(c);
                for (c = selmon->clients; c; c = c->next)
                        updateclientdesktop(c);
        }
	return dirty;
}

void
updatentiles(Monitor *m)
{
        m->ntiles = 0;
        for (Client *c = nexttiled(m->clients); c; c = nexttiled(c->next), m->ntiles++);
}

void
updateselmon(Monitor *m)
{
        Client *c;
        Monitor *p = selmon;

        selmon = m;
        if (p)
                for (c = p->clients; c; c = c->next)
                        updateclientdesktop(c);
        for (c = selmon->clients; c; c = c->next)
                updateclientdesktop(c);
}

void
updatenumlockmask(void)
{
	unsigned int i, j;
	XModifierKeymap *modmap;

	numlockmask = 0;
	modmap = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		for (j = 0; j < modmap->max_keypermod; j++)
			if (modmap->modifiermap[i * modmap->max_keypermod + j]
				== XKeysymToKeycode(dpy, XK_Num_Lock))
				numlockmask = (1 << i);
	XFreeModifiermap(modmap);
}

void
updatesizehints(Client *c)
{
	long msize;
	XSizeHints size;

	if (!XGetWMNormalHints(dpy, c->win, &size, &msize))
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	if (size.flags & PBaseSize) {
		c->basew = size.base_width;
		c->baseh = size.base_height;
	} else if (size.flags & PMinSize) {
		c->basew = size.min_width;
		c->baseh = size.min_height;
	} else
		c->basew = c->baseh = 0;
	if (size.flags & PResizeInc) {
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	} else
		c->incw = c->inch = 0;
	if (size.flags & PMaxSize) {
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	} else
		c->maxw = c->maxh = 0;
	if (size.flags & PMinSize) {
		c->minw = size.min_width;
		c->minh = size.min_height;
	} else if (size.flags & PBaseSize) {
		c->minw = size.base_width;
		c->minh = size.base_height;
	} else
		c->minw = c->minh = 0;
	if (size.flags & PAspect) {
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	} else
		c->maxa = c->mina = 0.0;
	c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
}

void
updatestatus(void)
{
	char rawstext[ROOTNAMELENGTH];

	if (!gettextprop(root, XA_WM_NAME, rawstext, sizeof rawstext)) {
                strcpy(stextc, "dwm-"VERSION);
                strcpy(stexts, stextc);
                wstext = TEXTW(stextc);
                drawbar(selmon);
                return;
        }
        /* fake signal handler */
        if (strncmp(rawstext, FSIGID, FSIGIDLEN) == 0) {
                int len, lensig, numarg;
                char sig[MAXFSIGNAMELEN + 1], arg[MAXFSIGARGLEN + 1];
                Arg a;

                numarg = sscanf(rawstext + FSIGIDLEN, "%" STR(MAXFSIGNAMELEN) "s%n%" STR(MAXFSIGARGLEN) "s%n",
                                sig, &lensig, arg, &len);
                if (numarg == 1)
                        a = (Arg){0};
                else if (numarg == 2) {
                        if (strncmp(arg, "i", len - lensig) == 0) {
                                if (sscanf(rawstext + FSIGIDLEN + len, "%i", &(a.i)) != 1)
                                        return;
                        } else if (strncmp(arg, "ui", len - lensig) == 0) {
                                if (sscanf(rawstext + FSIGIDLEN + len, "%u", &(a.ui)) != 1)
                                        return;
                        } else if (strncmp(arg, "f", len - lensig) == 0) {
                                if (sscanf(rawstext + FSIGIDLEN + len, "%f", &(a.f)) != 1)
                                        return;
                        } else
                                return;
                } else
                        return;
                for (int i = 0; i < LENGTH(signals); i++)
                        if (strncmp(sig, signals[i].sig, lensig) == 0 && signals[i].func)
                                signals[i].func(&a);
	} else {
                char stextp[STATUSLENGTH];
                char *stp = stextp, *stc = stextc, *sts = stexts;

                for (char *rst = rawstext; *rst != '\0'; rst++)
                        if ((unsigned char)*rst >= ' ')
                                *(stp++) = *(stc++) = *(sts++) = *rst;
                        else if ((unsigned char)*rst > DELIMITERENDCHAR)
                                *(stc++) = *rst;
                        else
                                *(sts++) = *rst;
                *stp = *stc = *sts = '\0';
                wstext = TEXTW(stextp);
                drawbar(selmon);
        }
}

/* systray is drawn on focused monitor */
void
updatesystray(void)
{
	unsigned int x = selmon->mx + selmon->mw, w = 1;
	Client *i;
	XSetWindowAttributes wa;
	XWindowChanges wc;

	if (!systray) {
		/* init systray */
		systray = ecalloc(1, sizeof(Systray));
		systray->win = XCreateSimpleWindow(dpy, root, x, selmon->by, w, bh, 0, 0,
                                                   scheme[SchemeStts][ColBg].pixel);
		wa.event_mask = ButtonPressMask|ExposureMask;
		wa.override_redirect = True;
		wa.background_pixel = scheme[SchemeStts][ColBg].pixel;
		XSelectInput(dpy, systray->win, SubstructureNotifyMask);
		XChangeProperty(dpy, systray->win, netatom[NetSystemTrayOrientation], XA_CARDINAL, 32,
				PropModeReplace, (unsigned char *)&netatom[NetSystemTrayOrientationHorz], 1);
		XChangeWindowAttributes(dpy, systray->win, CWEventMask|CWOverrideRedirect|CWBackPixel, &wa);
		XMapRaised(dpy, systray->win);
		XSetSelectionOwner(dpy, netatom[NetSystemTray], systray->win, CurrentTime);
		if (XGetSelectionOwner(dpy, netatom[NetSystemTray]) == systray->win) {
			sendevent(root, xatom[Manager], StructureNotifyMask, CurrentTime,
                                  netatom[NetSystemTray], systray->win, 0, 0);
			XSync(dpy, False);
		} else {
			fputs("dwm: unable to obtain system tray\n", stderr);
			free(systray);
			systray = NULL;
			return;
		}
	}
	for (w = 0, i = systray->icons; i; i = i->next) {
		/* prevent corruption of background color */
                wa.background_pixel = scheme[SchemeTray][ColBg].pixel;
		XChangeWindowAttributes(dpy, i->win, CWBackPixel, &wa);
		XMapRaised(dpy, i->win);
		w += systrayspacing;
		i->x = w;
		XMoveResizeWindow(dpy, i->win, i->x, (bh - SH) / 2, i->w, i->h);
		w += i->w;
		if (i->mon != selmon)
			i->mon = selmon;
	}
	w = w ? w + systrayspacing : 1;
	x -= w;
	XMoveResizeWindow(dpy, systray->win, x, selmon->by, w, bh);
	wc.x = x;
        wc.y = selmon->by;
        wc.width = w;
        wc.height = bh;
	wc.stack_mode = Above;
        wc.sibling = selmon->barwin;
	XConfigureWindow(dpy, systray->win, CWX|CWY|CWWidth|CWHeight|CWSibling|CWStackMode, &wc);
	XMapWindow(dpy, systray->win);
	XMapSubwindows(dpy, systray->win);
	/* redraw background */
        XSetForeground(dpy, drw->gc, scheme[w > 1 ? SchemeTray : SchemeStts][ColBg].pixel);
	XFillRectangle(dpy, systray->win, drw->gc, 0, 0, w, bh);
	XSync(dpy, False);
}

void
updatesystrayicongeom(Client *i, int w, int h)
{
        i->h = SH;
        if (w == h)
                i->w = SH;
        else if (h == SH)
                i->w = w;
        else
                i->w = (SH * w) / h;
        applysizehints(i, &(i->x), &(i->y), &(i->w), &(i->h), False);
        /* force icons into systray dimensions if they don't want to */
        if (i->h > SH) {
                if (i->w == i->h)
                        i->w = SH;
                else
                        i->w = (SH * i->w) / i->h;
                i->h = SH;
        }
}

void
updatesystrayiconstate(Client *i, XPropertyEvent *ev)
{
	long flags;
	int code;

	if (ev->atom != xatom[XembedInfo] || !(flags = getatomprop(i, xatom[XembedInfo])))
		return;
	if (flags & XEMBED_MAPPED && !i->tags) {
		i->tags = 1;
		code = XEMBED_WINDOW_ACTIVATE;
		XMapRaised(dpy, i->win);
		setclientstate(i, NormalState);
	} else if (!(flags & XEMBED_MAPPED) && i->tags) {
		i->tags = 0;
		code = XEMBED_WINDOW_DEACTIVATE;
		XUnmapWindow(dpy, i->win);
		setclientstate(i, WithdrawnState);
	} else
		return;
	sendevent(i->win, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0,
			systray->win, XEMBED_EMBEDDED_VERSION);
}

void
updatetitle(Client *c)
{
	if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
	if (c->name[0] == '\0') /* hack to mark broken clients */
		strcpy(c->name, broken);
}

void
updatewindowtype(Client *c)
{
	Atom state = getatomprop(c, netatom[NetWMState]);
	Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

	if (state == netatom[NetWMFullscreen])
		setfullscreen(c, 1);
	if (wtype == netatom[NetWMWindowTypeDialog])
		c->isfloating = 1;
}

void
updatewmhints(Client *c)
{
	XWMHints *wmh;

	if ((wmh = XGetWMHints(dpy, c->win))) {
		if (c == selmon->sel && wmh->flags & XUrgencyHint) {
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dpy, c->win, wmh);
		} else {
			c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
			if (c->isurgent)
				XSetWindowBorder(dpy, c->win, scheme[SchemeUrg][ColBorder].pixel);
		}
		if (wmh->flags & InputHint)
			c->neverfocus = !wmh->input;
		else
			c->neverfocus = 0;
		XFree(wmh);
	}
}

void
view(const Arg *arg)
{
	int i;

	if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
		return;
        resettagifempty(selmon->pertag->curtag);
	selmon->seltags ^= 1; /* toggle sel tagset */
	if (arg->ui & TAGMASK) {
		selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
		selmon->pertag->prevtag = selmon->pertag->curtag;

		if (arg->ui == ~0)
			selmon->pertag->curtag = 0;
		else {
			for (i = 0; !(arg->ui & 1 << i); i++);
			selmon->pertag->curtag = i + 1;
		}
	} else
                SWAP(selmon->pertag->prevtag, selmon->pertag->curtag);
        applycurtagsettings();
	focus(NULL);
	arrange(selmon);
}

Client *
wintoclient(Window w)
{
	Client *c;
	Monitor *m;

	for (m = mons; m; m = m->next)
		for (c = m->clients; c; c = c->next)
			if (c->win == w)
				return c;
	return NULL;
}

Client *
wintosystrayicon(Window w)
{
	Client *i;

	if (!showsystray || !w)
		return NULL;
	for (i = systray->icons; i && i->win != w; i = i->next);
	return i;
}

Monitor *
wintomon(Window w)
{
	int x, y;
	Client *c;
	Monitor *m;

	if (w == root && getrootptr(&x, &y))
		return recttomon(x, y, 1, 1);
	for (m = mons; m; m = m->next)
		if (w == m->barwin || w == m->tabwin)
			return m;
	if ((c = wintoclient(w)))
		return c->mon;
	return selmon;
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int
xerror(Display *dpy, XErrorEvent *ee)
{
	if (ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;
	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
		ee->request_code, ee->error_code);
	return xerrorxlib(dpy, ee); /* may call exit */
}

int
xerrordummy(Display *dpy, XErrorEvent *ee)
{
	return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int
xerrorstart(Display *dpy, XErrorEvent *ee)
{
	die("dwm: another window manager is already running");
	return -1;
}

void
zoom(const Arg *arg)
{
	Client *c;

        if (!selmon->sel)
                return;
        if (selmon->sel->isfloating || !selmon->lt[selmon->sellt]->arrange) {
                resize(selmon->sel,
                       selmon->mx + (selmon->mw - WIDTH(selmon->sel)) / 2,
                       selmon->my + (selmon->mh - HEIGHT(selmon->sel)) / 2,
                       selmon->sel->w, selmon->sel->h, 0);
                return;
        }
        c = selmon->sel;
	if (c == nexttiled(selmon->clients))
		if (!c || !(c = nexttiled(c->next)))
			return;
	pop(c);
}

int
main(int argc, char *argv[])
{
	if (argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-"VERSION);
	else if (argc != 1)
		die("usage: dwm [-v]");
	if (!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);
	if (!(dpy = XOpenDisplay(NULL)))
		die("dwm: cannot open display");
	checkotherwm();
	setup();
	scan();
	run();
	cleanup();
        restorestatus();
	XCloseDisplay(dpy);
	if (restart)
                execvp(argv[0], argv);
	return EXIT_SUCCESS;
}
