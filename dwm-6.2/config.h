/* See LICENSE file for copyright and license details. */

typedef struct {
        const Arg cmd;
        const unsigned int tag;
        const int scratchkey;
} Win;

/* appearance */
static const unsigned int borderpx              = 2;  /* border pixel of windows */
static const unsigned int snap                  = 10; /* snap pixel */
static const int gappih                         = 1;  /* horiz inner gap between windows */
static const int gappiv                         = 1;  /* vert inner gap between windows */
static const int gappoh                         = 1;  /* horiz outer gap between windows and screen edge */
static const int gappov                         = 1;  /* vert outer gap between windows and screen edge */
static const unsigned int systraypinning        = 0;  /* 0 means systray follows selected monitor; n other than 0 means pin systray to nth monitor */
static const unsigned int systrayspacing        = 4;  /* systray spacing */
static const int systraypinningfailfirst        = 1;  /* 1 means if pinning fails, display systray on the first monitor; 0 means display systray on the last monitory */
static const int showsystray                    = 1;  /* 0 means no systray */
static const int showbar                        = 1;  /* 0 means no bar */
static const int topbar                         = 1;  /* 0 means bottom bar */
/*  Display modes of the tab bar: never shown, always shown, shown only in  */
/*  monocle mode in the presence of several windows.                        */
/*  Modes after showtab_nmodes are disabled.                                */
enum showtab_modes { showtab_never, showtab_auto, showtab_nmodes, showtab_always};
static const int showtab                        = showtab_auto;        /* Default tab bar show mode */
static const int toptab                         = False;               /* False means bottom tab bar */

static const char *fonts[] = { "Fira Sans:size=12",
//                               "Font Awesome 5 Brands Regular:size=12",
//                               "Font Awesome 5 Free Solid:size=12",
//                               "Font Awesome 5 Free Regular:size=12",
                               "Siji:pixelsize=12",
                               "Noto Color Emoji:pixelsize=12" };

static const char col_black[]           = "#222222";
static const char col_cyan[]            = "#005577";
static const char col_gray1[]           = "#333333";
static const char col_gray2[]           = "#4e4e4e";
static const char col_white1[]          = "#eeeeee";
static const char col_white2[]          = "#dddddd";
static const char col_red[]             = "#b21e19";
static const char col1[]                = "#8fb4a6"; /* default icon color */
static const char col2[]                = "#c3c26f"; /* warning icon color */
static const char col3[]                = "#8d97cd"; /* mail block - frozen */
static const char col4[]                = "#b399cd"; /* mail block - MAILSYNC started */
static const char col5[]                = "#d29b88"; /* mail block - syncing */

enum { SchemeStts, SchemeCol1, SchemeCol2, SchemeCol3, SchemeCol4,
       SchemeCol5, SchemeNorm, SchemeSel, SchemeUrg, SchemeLtSm, SchemeTray }; /* color schemes */

static const char *colors[][3] = {
        /*                  fg            bg              border   */
	[SchemeStts]    = { col_white1,   col_black,      col_gray2 },
	[SchemeCol1]    = { col1,         col_black,      col_gray2 },
	[SchemeCol2]    = { col2,         col_black,      col_gray2 },
	[SchemeCol3]    = { col3,         col_black,      col_gray2 },
	[SchemeCol4]    = { col4,         col_black,      col_gray2 },
	[SchemeCol5]    = { col5,         col_black,      col_gray2 },
	[SchemeNorm]    = { col_white1,   col_gray1,      col_gray2 },
	[SchemeSel]     = { col_white1,   col_cyan,       col_cyan },
	[SchemeUrg]     = { col_white1,   col_red,        col_red },
	[SchemeLtSm]    = { col_white2,   col_black,      col_gray2 },
	[SchemeTray]    = { col_white2,   col_gray1,      col_gray2 },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

/* default layout per tags */
/* The first element is for all-tag view, following i-th element corresponds to */
/* tags[i]. Layout is referred using the layouts array index.*/
static int def_layouts[1 + LENGTH(tags)]  = { 0, 0, 0, 0, 0, 0, 0, 0, 2, 2};
static int def_attachs[1 + LENGTH(tags)] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1};

/* layout(s) */
static const float mfact        = 0.60; /* factor of master area size [0.05..0.95] */
static const int nmaster        = 1;    /* number of clients in master area */
static const int resizehints    = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol       arrange function */
	{ "[ ]=",       tile },
	{ "[ . ]",      NULL },    /* no layout function means floating behavior */
	{ "[M]",        monocle },
	{ "[D]",        deck },
        { NULL,         NULL }
};

static const Attach attachs[] = {
	/* symbol       attach function */
	{ "M",          attach },
	{ "D",          attachbelow },
	{ "S",          attachaside },
	{ "B",          attachbottom },
        { NULL,         NULL }
};

static const char *const *scratchcmds[] = {
	(const char *[]){ "termite", "--name=scratch_Termite", NULL },
	(const char *[]){ "brave", "--profile-directory=Default", "--app-id=cinhimbnkkaeohfgghhklpknlkffjgod", NULL },
	(const char *[]){ "termite", "--name=pyfzf_Termite", "-e", "/home/ashish/.local/bin/pyfzf", NULL },
	(const char *[]){ "signal-desktop", NULL },
	(const char *[]){ "telegram-desktop", NULL },
};

/* key definitions */
#define MODKEY Mod3Mask
#define MODRKEY Mod1Mask
#define SUPKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      vieworprev,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggletag,      {.ui = 1 << TAG} }, \
	{ SUPKEY,                       KEY,      tagandview,     {.ui = TAG} }, \
	{ SUPKEY|ShiftMask,             KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ SUPKEY|ControlMask,           KEY,      swaptags,       {.ui = TAG} },

/* helper for spawning shell commands in pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/usr/bin/dash", "-c", cmd, NULL } }
/* helpler for commands with no argument to be executed within a terminal */
#define TERMCMD(cmd) { .v = (const char*[]){ "termite", "-e", cmd, NULL } }

#define SCRIPT(name) "/home/ashish/.scripts/"name

#define CMD0(cmd) { .v = (const char*[]){ cmd, NULL } }
#define CMD1(cmd, arg) { .v = (const char*[]){ cmd, arg, NULL } }
#define SCRIPT0(name) { .v = (const char*[]){ SCRIPT(name), NULL } }
#define SCRIPT1(name, arg) { .v = (const char*[]){ SCRIPT(name), arg, NULL } }
#define REDSHIFT(arg) { .v = (const char*[]){ "redshift", "-O", arg, "-P", NULL } }

#define DICTIONARYHISTORY { .v = (const char*[]){ "termite", "--name=floating_Termite", \
        "-t", "Dictionary", "-e", SCRIPT("dictionary_history.sh") } }
#define INHIBITSUSPEND0 { .v = (const char*[]){ "systemd-inhibit", "--what=handle-lid-switch", \
        SCRIPT("watchlidswitch0.sh"), NULL } }
#define INHIBITSUSPEND1 { .v = (const char*[]){ "systemd-inhibit", "--what=handle-lid-switch", \
        SCRIPT("watchlidswitch1.sh"), NULL } }

#define PACTLD { .v = (const char*[]){ "pactl", "set-sink-volume", "0", "-5%", NULL } }
#define PACTLI { .v = (const char*[]){ "pactl", "set-sink-volume", "0", "+5%", NULL } }
#define PACTLM { .v = (const char*[]){ "pactl", "set-sink-mute", "0", "toggle", NULL } }
#define REDSHIFTDEFAULT { .v = (const char*[]){ "redshift", "-x", NULL } }
#define ROFIDRUN { .v = (const char*[]){ "rofi", "-show", "drun", "-show-icons", NULL } }
#define ROFIRUN { .v = (const char*[]){ "rofi", "-show", "run", NULL } }
#define ROFIWIN { .v = (const char*[]){ "rofi", "-show", "window", NULL } }

#define DISABLEDEMODE SHCMD("xmodmap /home/ashish/.Xmodmap_ude && dunstify -r 4120 -t 1000 'data entry mode deactivated'")
#define ENABLEDEMODE SHCMD("xmodmap /home/ashish/.Xmodmap_de && dunstify -r 4120 -t 0 'data entry mode activated'")

static const Win browser = { .cmd = CMD0("brave"), .tag = 7, .scratchkey = -1 };
static const Win mail = { .cmd = SCRIPT0("neomutt.sh"), .tag = 8, .scratchkey = -2 };

/* custom function declarations */
static void floatmovex(const Arg *arg);
static void floatmovey(const Arg *arg);
static void floatresizeh(const Arg *arg);
static void floatresizew(const Arg *arg);
static void focuslast(const Arg *arg);
static void focusmaster(const Arg *arg);
static void focusstackalt(const Arg *arg);
static void focusurgent(const Arg *arg);
static void hideclient(const Arg *arg);
static void hidefloating(const Arg *arg);
static void hidevisscratch(const Arg *arg);
static Client *nextprevsamefloat(int next);
static Client *nextprevvisible(int next);
static void push(const Arg *arg);
static void scratchhide(const Arg *arg);
static void scratchshow(const Arg *arg);
static void showfloating(const Arg *arg);
static void tagandview(const Arg *arg);
static void togglefocus(const Arg *arg);
static void togglefullscr(const Arg *arg);
static void togglestkpos(const Arg *arg);
static void togglewin(const Arg *arg);
static void vieworprev(const Arg *arg);
static void windowswitcher(const Arg *arg);
static void winview(const Arg* arg);
static void zoomswap(const Arg *arg);
static void zoomvar(const Arg *arg);

#include "inplacerotate.c"
#include <X11/XF86keysym.h>
static Key keys[] = {
	/* modifier                     key             function        argument */
	{ MODKEY,                       XK_d,           spawn,          ROFIDRUN },
	{ MODKEY|ShiftMask,             XK_d,           spawn,          ROFIRUN },
	{ MODKEY,                       XK_Return,      spawn,          CMD0("termite") },
	{ MODKEY,                       XK_b,           togglebar,      {0} },
	{ MODKEY|ShiftMask,             XK_b,           tabmode,        {-1} },
	{ MODKEY,                       XK_j,           focusstackalt,  {.i = +1 } },
	{ MODKEY,                       XK_Down,        focusstackalt,  {.i = +1 } },
	{ MODKEY,                       XK_k,           focusstackalt,  {.i = -1 } },
	{ MODKEY,                       XK_Up,          focusstackalt,  {.i = -1 } },
	{ MODKEY,                       XK_h,           setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_Left,        setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,           setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Right,       setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_equal,       setsplus,       {.i = +40} },
	{ MODKEY,                       XK_minus,       setsplus,       {.i = -40} },
	{ MODKEY|ShiftMask,             XK_equal,       setsplus,       {.i = +20} },
	{ MODKEY|ShiftMask,             XK_minus,       setsplus,       {.i = -20} },
	{ MODKEY,                       XK_BackSpace,   setsplus,       {.i = 0} },
	{ MODKEY|ShiftMask,             XK_BackSpace,   resetsplus,     {0} },
	{ MODKEY|ShiftMask,             XK_j,           push,           {.i = +1} },
	{ MODKEY|ShiftMask,             XK_Down,        push,           {.i = +1} },
	{ MODKEY|ShiftMask,             XK_k,           push,           {.i = -1} },
	{ MODKEY|ShiftMask,             XK_Up,          push,           {.i = -1} },
	{ SUPKEY,                       XK_period,      inplacerotate,  {.i = +2 } },
	{ SUPKEY,                       XK_comma,       inplacerotate,  {.i = +1 } },
	{ SUPKEY|ShiftMask,             XK_greater,     inplacerotate,  {.i = -2 } },
	{ SUPKEY|ShiftMask,             XK_less,        inplacerotate,  {.i = -1 } },
	{ SUPKEY,                       XK_j,           floatmovey,     {.i = +20} },
	{ SUPKEY,                       XK_Down,        floatmovey,     {.i = +20} },
	{ SUPKEY,                       XK_k,           floatmovey,     {.i = -20} },
	{ SUPKEY,                       XK_Up,          floatmovey,     {.i = -20} },
	{ SUPKEY,                       XK_h,           floatmovex,     {.i = -20} },
	{ SUPKEY,                       XK_Left,        floatmovex,     {.i = -20} },
	{ SUPKEY,                       XK_l,           floatmovex,     {.i = +20} },
	{ SUPKEY,                       XK_Right,       floatmovex,     {.i = +20} },
	{ SUPKEY|ShiftMask,             XK_j,           floatresizeh,   {.i = +20} },
	{ SUPKEY|ShiftMask,             XK_Down,        floatresizeh,   {.i = +20} },
	{ SUPKEY|ShiftMask,             XK_k,           floatresizeh,   {.i = -20} },
	{ SUPKEY|ShiftMask,             XK_Up,          floatresizeh,   {.i = -20} },
	{ SUPKEY|ShiftMask,             XK_h,           floatresizew,   {.i = -20} },
	{ SUPKEY|ShiftMask,             XK_Left,        floatresizew,   {.i = -20} },
	{ SUPKEY|ShiftMask,             XK_l,           floatresizew,   {.i = +20} },
	{ SUPKEY|ShiftMask,             XK_Right,       floatresizew,   {.i = +20} },
	{ MODKEY,                       XK_i,           incnmaster,     {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_i,           incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_space,       zoomvar,        {.i = 1} },
	{ MODKEY|ShiftMask,             XK_space,       zoomvar,        {.i = 0} },
	{ SUPKEY,                       XK_space,       view,           {0} },
	{ SUPKEY|ShiftMask,             XK_space,       moveprevtag,    {0} },
	{ MODKEY,                       XK_f,           togglefocus,    {0} },
	{ MODKEY|ShiftMask,             XK_f,           togglefullscr,  {0} },
	{ SUPKEY,                       XK_f,           togglefloating, {.i = 1} },
	{ SUPKEY|ShiftMask,             XK_f,           togglefloating, {.i = 0} },
	{ MODKEY,                       XK_Escape,      killclient,     {0} },
	{ MODKEY,                       XK_e,           setltorprev,    {.v = &layouts[0]} },
	{ MODKEY|ShiftMask,             XK_e,           setltorprev,    {.v = &layouts[1]} },
	{ MODKEY|ShiftMask,             XK_w,           setltorprev,    {.v = &layouts[2]} },
	{ MODKEY,                       XK_w,           setltorprev,    {.v = &layouts[3]} },
	{ MODKEY,                       XK_F1,          setattorprev,   {.v = &attachs[0]} },
	{ MODKEY,                       XK_F2,          setattorprev,   {.v = &attachs[1]} },
	{ MODKEY,                       XK_F3,          setattorprev,   {.v = &attachs[2]} },
	{ MODKEY,                       XK_F4,          setattorprev,   {.v = &attachs[3]} },

	{ MODKEY,                       XK_Tab,         focuslast,      {.i = 1} },
	{ SUPKEY,                       XK_Tab,         focuslast,      {.i = 0} },
	{ MODKEY,                       XK_m,           focusmaster,    {0} },
	{ MODKEY,                       XK_g,           focusurgent,    {0} },
	{ MODKEY,                       XK_o,           winview,        {0} },
	{ MODKEY,                       XK_q,           windowswitcher, {0} },
	{ MODKEY,                       XK_period,      shiftview,      {.i = +1 } },
	{ MODKEY,                       XK_comma,       shiftview,      {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,      hideclient,     {0} },
	{ SUPKEY,                       XK_F1,          togglescratch,  {.i = 1 } },
	{ MODRKEY,                      XK_m,           togglescratch,  {.i = 2 } },
	{ SUPKEY,                       XK_p,           togglescratch,  {.i = 3 } },
	{ SUPKEY,                       XK_s,           togglescratch,  {.i = 4 } },
	{ SUPKEY,                       XK_w,           togglescratch,  {.i = 5 } },
	{ MODKEY,                       XK_s,           togglestkpos,   {0} },
	{ MODRKEY,                      XK_space,       togglewin,      {.v = &browser} },
	{ SUPKEY,                       XK_m,           togglewin,      {.v = &mail} },
	{ 0,                            XK_Print,       spawn,          CMD1("gnome-screenshot", "-i") },
	{ MODKEY,                       XK_c,           spawn,          SCRIPT0("color_under_cursor.sh") },
	{ MODKEY,                       XK_F7,          spawn,          DISABLEDEMODE },
	{ MODKEY,                       XK_F8,          spawn,          ENABLEDEMODE },
	{ MODKEY,                       XK_semicolon,   spawn,          SCRIPT1("dictionary.sh", "sel") },
	{ MODKEY|ShiftMask,             XK_semicolon,   spawn,          SCRIPT0("dictionary.sh") },
	{ MODKEY|ControlMask,           XK_semicolon,   spawn,          SCRIPT1("dictionary.sh", "last") },
	{ SUPKEY,                       XK_semicolon,   spawn,          SCRIPT1("espeak.sh", "sel") },
	{ SUPKEY|ShiftMask,             XK_semicolon,   spawn,          SCRIPT0("espeak.sh") },
	{ SUPKEY|ControlMask,           XK_semicolon,   spawn,          SCRIPT1("espeak.sh", "last") },
	{ MODKEY|ControlMask,           XK_apostrophe,  spawn,          DICTIONARYHISTORY },
	{ MODKEY|ShiftMask,             XK_q,           spawn,          SCRIPT0("quit.sh") },
	{ MODKEY|ControlMask,           XK_h,           spawn,          SCRIPT0("hotspot_launch.sh") },
	{ MODKEY|ControlMask,           XK_m,           spawn,          SCRIPT0("toggletouchpad.sh") },
	{ MODKEY|ControlMask,           XK_r,           spawn,          SCRIPT0("reflector_launch.sh") },
	{ MODKEY,                       XK_F10,         spawn,          SCRIPT1("systemctl_timeout.sh", "toggle") },
	{ MODKEY|ShiftMask,             XK_F10,         spawn,          SCRIPT1("systemctl_timeout.sh", "status") },
	{ SUPKEY,                       XK_b,           spawn,          SCRIPT0("btns.sh") },
	{ SUPKEY,                       XK_c,           spawn,          TERMCMD("calcurse") },
	{ SUPKEY,                       XK_r,           spawn,          TERMCMD("ranger --cmd='set show_hidden=false'") },
	{ SUPKEY|ShiftMask,             XK_r,           spawn,          TERMCMD("ranger") },
	{ SUPKEY,                       XK_t,           spawn,          TERMCMD("htop") },
	{ MODRKEY,                      XK_s,           spawn,          INHIBITSUSPEND1 },
	{ MODRKEY|ShiftMask,            XK_s,           spawn,          INHIBITSUSPEND0 },
	{ MODRKEY,                      XK_k,           spawn,          SCRIPT1("ytmsclu.sh", "0") },
	{ MODRKEY,                      XK_l,           spawn,          SCRIPT1("ytmsclu.sh", "1") },
	{ MODRKEY,                      XK_semicolon,   spawn,          SCRIPT0("ytresume.sh") },
	{ MODRKEY,                      XK_Delete,      spawn,          SCRIPT0("usbmount.sh") },

	{ MODKEY,            XK_bracketleft,            hidefloating,   {0} },
	{ MODKEY,            XK_bracketright,           showfloating,   {0} },
	{ MODKEY,            XK_backslash,              hidevisscratch, {0} },
	{ 0,                 XF86XK_AudioMute,          spawn,          PACTLM },
	{ 0,                 XF86XK_AudioLowerVolume,   spawn,          PACTLD },
	{ 0,                 XF86XK_AudioRaiseVolume,   spawn,          PACTLI },
	{ 0,                 XF86XK_MonBrightnessDown,  spawn,          SCRIPT0("btnsd.sh") },
	{ 0,                 XF86XK_MonBrightnessUp,    spawn,          SCRIPT0("btnsi.sh") },
	{ ShiftMask,         XK_F2,                     spawn,          SCRIPT0("btnsds.sh") },
	{ ShiftMask,         XK_F3,                     spawn,          SCRIPT0("btnsis.sh") },
	{ MODRKEY,           XK_Escape,                 spawn,          REDSHIFTDEFAULT },
	{ MODRKEY,           XK_F1,                     spawn,          REDSHIFT("5500") },
	{ MODRKEY,           XK_F2,                     spawn,          REDSHIFT("4500") },
	{ MODRKEY,           XK_F3,                     spawn,          REDSHIFT("3500") },
	{ MODRKEY,           XK_F4,                     spawn,          REDSHIFT("3000") },
	{ MODRKEY,           XK_F5,                     spawn,          REDSHIFT("2500") },
	{ MODRKEY,           XK_F6,                     spawn,          REDSHIFT("2000") },
	{ MODRKEY,           XK_F7,                     spawn,          REDSHIFT("1800") },
	{ MODRKEY,           XK_F8,                     spawn,          REDSHIFT("1600") },
	{ MODRKEY,           XK_F9,                     spawn,          REDSHIFT("1400") },
	{ MODRKEY,           XK_F10,                    spawn,          REDSHIFT("1200") },
	{ MODRKEY,           XK_F11,                    spawn,          REDSHIFT("1100") },
	{ MODRKEY,           XK_F12,                    spawn,          REDSHIFT("1000") },

	{ MODKEY,                       XK_0,           vieworprev,     {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,           tag,            {.ui = ~0 } },
	TAGKEYS(                        XK_1,                           0)
	TAGKEYS(                        XK_2,                           1)
	TAGKEYS(                        XK_3,                           2)
	TAGKEYS(                        XK_4,                           3)
	TAGKEYS(                        XK_5,                           4)
	TAGKEYS(                        XK_6,                           5)
	TAGKEYS(                        XK_7,                           6)
	TAGKEYS(                        XK_8,                           7)
	TAGKEYS(                        XK_9,                           8)
};

/* button definitions */
/* click can be ClkTabBar, ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkTabBar,            0,              Button1,        focuswin,       {0} },
	{ ClkLtSymbol,          0,              Button1,        setltorprev,    {.v = &layouts[1]} },
	{ ClkWinTitle,          0,              Button1,        togglefloating, {.i = 1} },
	{ ClkStatusText,        0,              Button1,        sigdsblocks,    {.i = 1} },
	{ ClkStatusText,        0,              Button2,        sigdsblocks,    {.i = 2} },
	{ ClkStatusText,        0,              Button3,        sigdsblocks,    {.i = 3} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {.i = 0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        vieworprev,     {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

#define FSIGID "z:"
/* without considering the terminating null byte */
#define FSIGIDLEN 2
#define MAXFSIGNAMELEN 4
#define MAXFSIGARGLEN 2

/* trigger signals using `xsetroot -name "FSIGNAL<signame> [<type> <value>]"` */
/* signal definitions */
static Signal signals[] = {
	/* signame              function */
	{ "quit",               quit },
	{ "scrh",               scratchhide },
	{ "scrs",               scratchshow },
};

/* custom function implementations */
void
floatmovex(const Arg *arg)
{
        if (selmon->sel && arg->i && (!selmon->lt[selmon->sellt]->arrange || selmon->sel->isfloating)) {
                int nx, ptrx, ptry;
                int restoreptr = 0;
                int vselcw = WIDTH(selmon->sel);
                int vselmw = selmon->wx + selmon->ww;

                nx = selmon->sel->x + arg->i;
                if (selmon->sel->x > selmon->wx && nx < selmon->wx)
                        nx = selmon->wx;
                else if (selmon->sel->x + vselcw < vselmw && nx + vselcw > vselmw)
                        nx = vselmw - vselcw;
                XRaiseWindow(dpy, selmon->sel->win);
                if (getwinptr(selmon->sel->win, &ptrx, &ptry) &&
                    ptrx >= -selmon->sel->bw && ptrx <= selmon->sel->w + selmon->sel->bw - 1 &&
                    ptry >= -selmon->sel->bw && ptry <= selmon->sel->h + selmon->sel->bw - 1)
                        restoreptr = 1;
                resize(selmon->sel, nx, selmon->sel->y, selmon->sel->w, selmon->sel->h, True);
                if (restoreptr)
                        XWarpPointer(dpy, None, selmon->sel->win, 0, 0, 0, 0, ptrx, ptry);
        }
}

void
floatmovey(const Arg *arg)
{
        if (selmon->sel && arg->i && (!selmon->lt[selmon->sellt]->arrange || selmon->sel->isfloating)) {
                int ny, ptrx, ptry;
                int restoreptr = 0;
                int vselch = HEIGHT(selmon->sel);
                int vselmh = selmon->wy + selmon->wh;

                ny = selmon->sel->y + arg->i;
                if (selmon->sel->y > selmon->wy && ny < selmon->wy)
                        ny = selmon->wy;
                else if (selmon->sel->y + vselch < vselmh && ny + vselch > vselmh)
                        ny = vselmh - vselch;
                XRaiseWindow(dpy, selmon->sel->win);
                if (getwinptr(selmon->sel->win, &ptrx, &ptry) &&
                    ptrx >= -selmon->sel->bw && ptrx <= selmon->sel->w + selmon->sel->bw - 1 &&
                    ptry >= -selmon->sel->bw && ptry <= selmon->sel->h + selmon->sel->bw - 1)
                        restoreptr = 1;
                resize(selmon->sel, selmon->sel->x, ny, selmon->sel->w, selmon->sel->h, True);
                if (restoreptr)
                        XWarpPointer(dpy, None, selmon->sel->win, 0, 0, 0, 0, ptrx, ptry);
        }
}

void
floatresizeh(const Arg *arg)
{
        if (selmon->sel && arg->i && (!selmon->lt[selmon->sellt]->arrange || selmon->sel->isfloating)) {
                int nh, ptrx, ptry;
                int restoreptr = 0;
                int vselcy = selmon->sel->y + 2 * selmon->sel->bw;
                int vselmh = selmon->wy + selmon->wh;
                float ptryp;

                nh = selmon->sel->h + arg->i;
                if (vselcy + selmon->sel->h < vselmh && vselcy + nh > vselmh)
                        nh = vselmh - vselcy;
                XRaiseWindow(dpy, selmon->sel->win);
                if (arg->i < 0 && getwinptr(selmon->sel->win, &ptrx, &ptry)
                               && ptrx >= -selmon->sel->bw && ptrx <= selmon->sel->w + selmon->sel->bw - 1
                               && ptry >= -selmon->sel->bw && ptry <= selmon->sel->h + selmon->sel->bw - 1) {
                        restoreptr = 1;
                        ptryp = (float)ptry / (float)selmon->sel->h;
                }
                resize(selmon->sel, selmon->sel->x, selmon->sel->y, selmon->sel->w, nh, True);
                if (restoreptr) {
                        ptry = selmon->sel->h * ptryp;
                        XWarpPointer(dpy, None, selmon->sel->win, 0, 0, 0, 0, ptrx, ptry);
                }
        }
}

void
floatresizew(const Arg *arg)
{
        if (selmon->sel && arg->i && (!selmon->lt[selmon->sellt]->arrange || selmon->sel->isfloating)) {
                int nw, ptrx, ptry;
                int restoreptr = 0;
                int vselcx = selmon->sel->x + 2 * selmon->sel->bw;
                int vselmw = selmon->wx + selmon->ww;
                float ptrxp;

                nw = selmon->sel->w + arg->i;
                if (vselcx + selmon->sel->w < vselmw && vselcx + nw > vselmw)
                        nw = vselmw - vselcx;
                XRaiseWindow(dpy, selmon->sel->win);
                if (arg->i < 0 && getwinptr(selmon->sel->win, &ptrx, &ptry)
                               && ptrx >= -selmon->sel->bw && ptrx <= selmon->sel->w + selmon->sel->bw - 1
                               && ptry >= -selmon->sel->bw && ptry <= selmon->sel->h + selmon->sel->bw - 1) {
                        restoreptr = 1;
                        ptrxp = (float)ptrx / (float)selmon->sel->w;
                }
                resize(selmon->sel, selmon->sel->x, selmon->sel->y, nw, selmon->sel->h, True);
                if (restoreptr) {
                        ptrx = selmon->sel->w * ptrxp;
                        XWarpPointer(dpy, None, selmon->sel->win, 0, 0, 0, 0, ptrx, ptry);
                }
        }
}

void
focuslast(const Arg *arg)
{
        Client *c = selmon->sel ? selmon->sel->snext : selmon->stack;

        if (arg->i)
                for (; c && (c->ishidden || !c->tags); c = c->snext);
        else
                for (; c && !ISVISIBLE(c); c = c->snext);
        if (c)
                focusclient(c, 0);
}

void
focusmaster(const Arg *arg)
{
        Client *c;

	if (selmon->nmaster < 1)
		return;
        if ((c = nexttiled(selmon->clients))) {
                focusalt(c);
                restack(selmon, 0);
        }
}

void
focusstackalt(const Arg *arg)
{
	Client *c;

	if (!selmon->sel)
		return;
        if (selmon->lt[selmon->sellt]->arrange == deck && selmon->ntiles > selmon->nmaster + 1
        && !selmon->sel->isfloating) {
                int n, curidx;
                Client *i;

                c = NULL;
                for (curidx = 0, i = nexttiled(selmon->clients); i != selmon->sel; i = nexttiled(i->next), curidx++);
                if (arg->i > 0) {
                        /* focus the visible slave; since selmon->ntiles > selmon->nmaster + 1, it will exist */
                        if (curidx == selmon->nmaster - 1) {
                                c = selmon->sel;
                                do {
                                        for (c = c->snext; c && (!ISVISIBLE(c) || c->isfloating); c = c->snext);
                                        for (curidx = 0, i = nexttiled(selmon->clients);
                                             i != c;
                                             i = nexttiled(i->next), curidx++);
                                } while (curidx < selmon->nmaster);
                        } else if (curidx == selmon->ntiles - 1) /* focus first slave */
                                for (n = selmon->nmaster, c = nexttiled(selmon->clients);
                                     n;
                                     c = nexttiled(c->next), n--);
                        else /* focus next client */
                                c = nexttiled(selmon->sel->next);
                } else {
                        /* focus the visible slave; since selmon->ntiles > selmon->nmaster + 1, it will exist */
                        if (selmon->nmaster && curidx == 0) {
                                c = selmon->sel;
                                do {
                                        for (c = c->snext; c && (!ISVISIBLE(c) || c->isfloating); c = c->snext);
                                        for (curidx = 0, i = nexttiled(selmon->clients);
                                             i != c;
                                             i = nexttiled(i->next), curidx++);
                                } while (curidx < selmon->nmaster);
                        } else if (curidx == selmon->nmaster) /* focus last slave */
                                for (n = selmon->ntiles - 1, c = nexttiled(selmon->clients);
                                     n;
                                     c = nexttiled(c->next), n--);
                        else { /* focus previous client */
                                for (i = nexttiled(selmon->clients); i != selmon->sel; i = nexttiled(i->next)) {
                                        if (i)
                                                c = i;
                                }
                        }
                }
        } else if (selmon->lt[selmon->sellt]->arrange)
                c = nextprevsamefloat(arg->i);
        else
                c = nextprevvisible(arg->i);
	if (c) {
		focusalt(c);
		restack(selmon, 0);
        }
}

void
focusurgent(const Arg *arg)
{
        for (Monitor *m = mons; m; m = m->next)
                for (Client *c = selmon->stack; c; c = c->snext)
                        if (c && c->isurgent) {
                                focusclient(c, 0);
                                return;
                        }
}

void
hideclient(const Arg *arg)
{
        if (!selmon->sel)
                return;
        if (selmon->sel->isfullscreen)
                setfullscreen(selmon->sel, 0);
        if (!selmon->sel->isfloating) {
                selmon->sel->isfloating = 1;
                resize(selmon->sel, selmon->sel->sfx, selmon->sel->sfy,
                       selmon->sel->sfw, selmon->sel->sfh, False);
		XRaiseWindow(dpy, selmon->sel->win);
        }
        selmon->sel->ishidden = 1;
        updateclientdesktop(selmon->sel);
        focus(NULL);
        arrange(selmon);
}

void
hidefloating(const Arg *arg)
{
        for (Client *c = selmon->clients; c; c = c->next) {
                if (c->isfullscreen)
                        continue;
                if (c->isfloating && ISVISIBLE(c)) {
                        c->ishidden = 1;
                        updateclientdesktop(c);
                }
        }
        focus(NULL);
        arrange(selmon);
}

void
hidevisscratch(const Arg *arg)
{
        unsigned long t = 0;

        for (Client *c = selmon->clients; c; c = c->next)
                if ((c->scratchkey > 0) && ISVISIBLE(c)) {
                        c->tags = 0;
                        XChangeProperty(dpy, c->win, netatom[NetWMDesktop], XA_CARDINAL, 32,
                                        PropModeReplace, (unsigned char *) &t, 1);
                }
        focus(NULL);
        arrange(selmon);
}

/* the following two functions assume non-NULL selmon->sel */
Client *
nextprevsamefloat(int next)
{
        Client *c = NULL;

        if (next > 0) {
                for (c = selmon->sel->next;
                     c && (!ISVISIBLE(c) || c->isfloating != selmon->sel->isfloating);
                     c = c->next);
                if (!c)
                        for (c = selmon->clients;
                             c && (!ISVISIBLE(c) || c->isfloating != selmon->sel->isfloating);
                             c = c->next);
        } else {
                Client *i;

                for (i = selmon->clients; i != selmon->sel; i = i->next)
                        if (ISVISIBLE(i) && i->isfloating == selmon->sel->isfloating)
                                c = i;
                if (!c)
                        for (; i; i = i->next)
                                if (ISVISIBLE(i) && i->isfloating == selmon->sel->isfloating)
                                        c = i;
        }
        return c;
}

Client *
nextprevvisible(int next)
{
        Client *c = NULL;

	if (next > 0) {
		for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
		if (!c)
			for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
	} else {
                Client *i;

		for (i = selmon->clients; i != selmon->sel; i = i->next)
			if (ISVISIBLE(i))
				c = i;
		if (!c)
			for (; i; i = i->next)
				if (ISVISIBLE(i))
					c = i;
	}
        return c;
}

void
scratchhide(const Arg *arg)
{
        if (selmon->sel && selmon->sel->scratchkey == arg->i) {
                unsigned long t = 0;

                selmon->sel->tags = 0;
                XChangeProperty(dpy, selmon->sel->win, netatom[NetWMDesktop], XA_CARDINAL, 32,
                                PropModeReplace, (unsigned char *) &t, 1);
                focus(NULL);
                arrange(selmon);
        }
}

void
scratchshow(const Arg *arg)
{
        Client *c;

        for (Monitor *m = mons; m; m = m->next)
                for (c = selmon->clients; c; c = c->next)
                        if (c->scratchkey == arg->i)
                                goto show;
        spawn(&((Arg){ .v = scratchcmds[arg->i - 1] }));
        return;
show:
        if (selmon->sel && c == selmon->sel)
                return;
        if (c->ishidden)
                c->ishidden = 0;
        if (c->mon != selmon) {
                sendmon(c, selmon);
                return;
        }
        c->tags = selmon->tagset[selmon->seltags];
        updateclientdesktop(c);
        focusalt(c);
        arrange(selmon);
}

void
push(const Arg *arg)
{
        Client *c;
	Client *ps = NULL, *pc = NULL;
        Client *i, *tmp;

        if (!selmon->sel)
                return;
        if (selmon->lt[selmon->sellt]->arrange)
                c = nextprevsamefloat(arg->i);
        else
                c = nextprevvisible(arg->i);
        if (!c || c == selmon->sel)
                return;
	for (i = selmon->clients; i && (!ps || !pc); i = i->next) {
		if (i->next == selmon->sel)
			ps = i;
		if (i->next == c)
			pc = i;
	}
	/* swap c and selmon->sel in the selmon->clients list */
        tmp = (selmon->sel->next == c) ? selmon->sel : selmon->sel->next;
        selmon->sel->next = (c->next == selmon->sel) ? c : c->next;
        c->next = tmp;
        if (ps) {
                if (ps != c)
                        ps->next = c;
        } else
                selmon->clients = c;
        if (pc) {
                if (pc != selmon->sel)
                        pc->next = selmon->sel;
        } else
                selmon->clients = selmon->sel;
        arrange(selmon);
}

void
showfloating(const Arg *arg)
{
        Client *f = NULL; /* last focused hidden floating client */

        for (Client *c = selmon->stack; c; c = c->snext)
                if (c->ishidden && (c->tags & selmon->tagset[selmon->seltags])) {
                        if (!f)
                                f = c;
                        c->ishidden = 0;
                        updateclientdesktop(c);
                }
        if (f) {
                focusalt(f);
                arrange(selmon);
        }
}

void
tagandview(const Arg *arg)
{
        if (selmon->sel && (1 << arg->ui) != selmon->tagset[selmon->seltags]) {
                unsigned long t = arg->ui + 1;

                selmon->sel->tags = 1 << arg->ui;
                XChangeProperty(dpy, selmon->sel->win, netatom[NetWMDesktop], XA_CARDINAL, 32,
                                PropModeReplace, (unsigned char *) &t, 1);
                focusclient(selmon->sel, arg->ui);
        }
}

void
togglefocus(const Arg *arg)
{
        Client *c;

        if (!selmon->sel)
                return;
        if (selmon->sel->isfloating) {
                if (selmon->sel->isfullscreen)
                        return;
                for (c = selmon->sel; c && (!ISVISIBLE(c) || c->isfloating); c = c->snext);
        } else
                for (c = selmon->sel; c && (!ISVISIBLE(c) || !c->isfloating); c = c->snext);
        if (c) {
                focusalt(c);
                restack(selmon, 0);
        }
}

void
togglefullscr(const Arg *arg)
{
        int found = 0;
        Client *c;

        for (c = selmon->clients; c; c = c->next)
                if (ISVISIBLE(c) && c->isfullscreen) {
                        found = 1;
                        setfullscreen(c, 0);
                }
        if (!found && selmon->sel)
                setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
}

void
togglestkpos(const Arg *arg)
{
        int curidx, inrel;
        Client *c, *i;

        if (!selmon->sel || selmon->sel->isfloating)
                return;
        for (curidx = 0, i = nexttiled(selmon->clients); i != selmon->sel; i = nexttiled(i->next), curidx++);
        inrel = (curidx < selmon->nmaster);

        c = selmon->sel;
        do {
                for (c = c->snext; c && (!ISVISIBLE(c) || c->isfloating); c = c->snext);
                if (c)
                        for (curidx = 0, i = nexttiled(selmon->clients);
                             i != c;
                             i = nexttiled(i->next), curidx++);
                else
                        return;
        } while ((curidx < selmon->nmaster) == inrel);
        focusalt(c);
        restack(selmon, 0);
}

void
togglewin(const Arg *arg)
{
        Client *c;

        if (selmon->sel && selmon->sel->scratchkey == ((Win*)(arg->v))->scratchkey) {
                for (c = selmon->sel->snext;
                     c && (c->ishidden || !c->tags);
                     c = c->snext);
                if (c)
                        focusclient(c, 0);
                else
                        view(&((Arg){0}));
                return;
        }
        for (c = selmon->clients;
             c && c->scratchkey != ((Win*)(arg->v))->scratchkey;
             c = c->next);
        if (c)
                focusclient(c, ((Win*)(arg->v))->tag);
        else {
                view(&((Arg){ .ui = 1 << (((Win*)(arg->v))->tag) }));
                spawn(&((Win*)(arg->v))->cmd);
        }
}

void
vieworprev(const Arg *arg)
{
	view(((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags]) ? &((Arg){0}) : arg);
}

void
windowswitcher(const Arg *arg)
{
	XDeleteProperty(dpy, root, netatom[NetClientList]);
        for (Client *c = selmon->stack; c; c = c->snext)
                XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                                PropModePrepend, (unsigned char *) &(c->win), 1);
	for (Monitor *m = mons; m; m = m->next) {
                if (m == selmon)
                        continue;
                for (Client *c = selmon->stack; c; c = c->snext)
                        XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                                        PropModePrepend, (unsigned char *) &(c->win), 1);
        }
        spawn(&((Arg) ROFIWIN));
}

void
winview(const Arg* arg)
{
        if (selmon->sel)
                view(&((Arg){.ui = selmon->sel->tags}));
}

void
zoomswap(const Arg *arg)
{
	Client *c;
        Client *master, *at;

	if (!selmon->sel)
		return;
        if (selmon->sel->isfloating || !selmon->lt[selmon->sellt]->arrange) {
                resize(selmon->sel,
                       selmon->mx + (selmon->mw - WIDTH(selmon->sel)) / 2,
                       selmon->my + (selmon->mh - HEIGHT(selmon->sel)) / 2,
                       selmon->sel->w, selmon->sel->h, 0);
                return;
        }
	if ((c = selmon->sel) == (master = nexttiled(selmon->clients))) {
                for (c = c->snext; c && (c->isfloating || !ISVISIBLE(c)); c = c->snext);
                if (!c)
                        return;
        }
        for (at = selmon->clients; at && at->next != c; at = at->next);
	detach(c);
	attach(c);
	/* swap windows instead of pushing the previous one down */
	if (master && at && c != master && at != master) {
                Client **tc;

                /* make master the "snext" without focusing it */
                for (tc = &master->mon->stack; *tc && *tc != master; tc = &(*tc)->snext);
                *tc = master->snext;
                attachstack(master);

                detach(master);
                master->next = at->next;
                at->next = master;
	}
	focusalt(c);
	arrange(selmon);
}

void
zoomvar(const Arg *arg)
{
        if (selmon->lt[selmon->sellt]->arrange == deck && selmon->ntiles > selmon->nmaster + 1)
                arg->i ? zoomswap(&((Arg){0})) : zoom(&((Arg){0}));
        else
                arg->i ? zoom(&((Arg){0})) : zoomswap(&((Arg){0}));
}

/* Window rules */
static void center(Client *c);
static void marknegscratch(Client *c, int scratchkey);
static void markposscratch(Client *c, int scratchkey);

static void
applyrules(Client *c)
{
	const char *class, *instance;
	XClassHint ch = { NULL, NULL };

	XGetClassHint(dpy, c->win, &ch);
	class = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name ? ch.res_name : broken;

        if (strcmp(instance, "scratch_Termite") == 0) {
                c->isfloating = 1;
                markposscratch(c, 1);
                c->w = 750;
                c->h = 450;
                center(c);
        } else if (strcmp(instance, "neomutt_Termite") == 0) {
                marknegscratch(c, -2);
        } else if (strcmp(instance, "pyfzf_Termite") == 0) {
                c->isfloating = 1;
                markposscratch(c, 3);
                c->w = 750;
                c->h = 450;
                center(c);
        } else if (strcmp(instance, "floating_Termite") == 0) {
                c->isfloating = 1;
                c->w = 750;
                c->h = 450;
                center(c);
        /* windows to be made floating and centered */
        } else if (strstr(class, "Yad") ||
                   strcmp(class, "Sxiv") == 0 ||
                   strcmp(c->name, "Event Tester") == 0 ||
                   strcmp(class, "Matplotlib") == 0 ||
                   strcmp(class, "Ristretto") == 0 ||
                   strcmp(class, "Woeusbgui") == 0) {
                c->isfloating = 1;
                center(c);
        /* windows to be made floating and have 0 border */
        } else if (strcmp(class, "Gnome-screenshot") == 0) {
                c->isfloating = 1;
                c->bw = 0;
        /* windows generally opened once */
        } else if (strcmp(instance, "crx_cinhimbnkkaeohfgghhklpknlkffjgod") == 0) {
                c->isfloating = 1;
                markposscratch(c, 2);
                c->bw = 0;
                c->w = 950;
                c->h = 626;
                center(c);
        } else if (strcmp(instance, "brave-browser") == 0) {
                marknegscratch(c, -1);
        } else if (strcmp(class, "Signal") == 0) {
                c->isfloating = 1;
                markposscratch(c, 4);
                c->w = 880;
                c->h = 620;
                center(c);
        } else if (strcmp(class, "TelegramDesktop") == 0) {
                c->isfloating = 1;
                markposscratch(c, 5);
                c->w = 770;
                c->h = 555;
                center(c);
        }

	if (ch.res_class)
		XFree(ch.res_class);
	if (ch.res_name)
		XFree(ch.res_name);

	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

void
center(Client *c)
{
        c->x = c->mon->mx + (c->mon->mw - WIDTH(c)) / 2;
        c->y = c->mon->my + (c->mon->mh - HEIGHT(c)) / 2;
}

void
marknegscratch(Client *c, int scratchkey)
{
        for (Client *i = selmon->clients; i; i = i->next)
                if (i->scratchkey == scratchkey)
                        return;
        c->scratchkey = scratchkey;
}

void
markposscratch(Client *c, int scratchkey)
{
        for (Monitor *m = mons; m; m = m->next)
                for (Client *i = selmon->clients; i; i = i->next)
                        if (i->scratchkey == scratchkey)
                                return;
        c->scratchkey = scratchkey;
}
