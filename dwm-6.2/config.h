/* See LICENSE file for copyright and license details. */

/* maximum number of tabs to show */
#define MAXTABS                         15
/* minimum allowed vertical height of a window when resizing with setsplus */
#define MINWINHEIGHT                    26
/* systray icon height, change to bh to keep it equal to font height */
#define SYSTRAYHEIGTH                   16
/* spacing between systray icons */
#define SYSTRAYSPACING                  4
/* colorscheme used for background of systray */
#define SCHEMESYSTRAY                   SchemeNorm

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
static const int showsystray                    = 1;  /* 0 means no systray */
static const int showbar                        = 1;  /* 0 means no bar */
static const int topbar                         = 1;  /* 0 means bottom bar */
/* display modes of the tab bar
 * modes after showtab_nmodes are disabled */
enum showtab_modes { showtab_never, showtab_auto, showtab_nmodes, showtab_always};
static const int showtab                        = showtab_auto; /* default tab bar show mode */
static const int toptab                         = 0;            /* 0 means bottom tab bar */

static const char *fonts[] = { "Fira Sans:size=12",
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
static const char col2[]                = "#bebd82"; /* alternate icon color */
static const char col3[]                = "#cda091"; /* mail block - syncing */
static const char col4[]                = "#9e95cd"; /* mail block - frozen */

enum { SchemeStts, SchemeCol1, SchemeCol2, SchemeCol3, SchemeCol4,
       SchemeCol5, SchemeNorm, SchemeSel, SchemeUrg, SchemeLtSm }; /* color schemes */

static const char *colors[][3] = {
        /*                  fg            bg              border   */
	[SchemeStts]    = { col_white1,   col_black,      col_gray2 },
	[SchemeCol1]    = { col1,         col_black,      col_gray2 },
	[SchemeCol2]    = { col2,         col_black,      col_gray2 },
	[SchemeCol3]    = { col3,         col_black,      col_gray2 },
	[SchemeCol4]    = { col4,         col_black,      col_gray2 },
	[SchemeNorm]    = { col_white1,   col_gray1,      col_gray2 },
	[SchemeSel]     = { col_white1,   col_cyan,       col_cyan },
	[SchemeUrg]     = { col_white1,   col_red,        col_red },
	[SchemeLtSm]    = { col_white2,   col_black,      col_gray2 },
};

static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

/* first element is for all-tag view */
static int def_layouts[1 + LENGTH(tags)]  = { 0, 0, 0, 0, 0, 0, 0, 0, 2, 2};
static int def_attachs[1 + LENGTH(tags)] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1};

static const float mfact        = 0.60; /* factor of master area size (0.05 - 0.95) */
static const int nmaster        = 1;    /* number of clients in master area */
static const int resizehints    = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol       arrange function */
	{ "[ ]=",       tile },
	{ "[ . ]",      NULL },    /* no layout function means floating behavior */
	{ "[M]",        monocle },
	{ "[D]",        deck },
};

static const Attach attachs[] = {
	/* symbol       attach function */
	{ "T",          attach },
	{ "D",          attachbelow },
	{ "U",          attachabove },
	{ "S",          attachaside },
	{ "B",          attachbottom },
};

static const char *const *scratchcmds[] = {
	(const char *[]){ "termite", "--name=scratch_Termite", NULL },
	(const char *[]){ "brave", "--app-id=cinhimbnkkaeohfgghhklpknlkffjgod", NULL },
	(const char *[]){ "termite", "--name=pyfzf_Termite", "-e", "/home/ashish/.local/bin/pyfzf", NULL },
	(const char *[]){ "termite", "--name=calcurse_Termite", "-t", "Calcurse", "-e", "calcurse", NULL },
	(const char *[]){ "signal-desktop", NULL },
	(const char *[]){ "telegram-desktop", NULL },
};

/* key definitions */
#define MODLKEY Mod3Mask
#define MODRKEY Mod1Mask
#define SUPKEY  Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODLKEY,                      KEY,      vieworprev,     {.ui = 1 << TAG} }, \
	{ MODLKEY|ShiftMask,            KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODLKEY|ControlMask,          KEY,      toggletag,      {.ui = 1 << TAG} }, \
	{ SUPKEY,                       KEY,      tagandview,     {.ui = TAG} }, \
	{ SUPKEY|ShiftMask,             KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ SUPKEY|ControlMask,           KEY,      swaptags,       {.ui = TAG} },

#define SCRIPT(name) "/home/ashish/.scripts/"name

#define CMD(...) { .v = (const char*[]){ __VA_ARGS__, NULL } }
#define SCRIPTCMD(...) { .v = (const char*[]){ "/home/ashish/.scripts/"__VA_ARGS__, NULL } }
#define SHCMD(cmd) { .v = (const char*[]){ "dash", "-c", cmd, NULL } }
#define TERMCMD(cmd) { .v = (const char*[]){ "termite", "-e", cmd, NULL } }
#define TERMSCRIPTCMD(cmd) { .v = (const char*[]){ "termite", "-e", SCRIPT(cmd), NULL } }

#define REDSHIFT(arg) { .v = (const char*[]){ "redshift", "-PO" arg, NULL } }
#define REDSHIFTDEFAULT { .v = (const char*[]){ "redshift", "-x", NULL } }

#define ROFIDRUN { .v = (const char*[]){ "rofi", "-show", "drun", "-show-icons", NULL } }
#define ROFIRUN { .v = (const char*[]){ "rofi", "-show", "run", NULL } }
#define ROFIWIN { .v = (const char*[]){ "rofi", "-show", "window", NULL } }
#define ROFIWINREGEX { .v = (const char*[]){ "rofi", "-show", "window", "-matching", "regex", NULL } }

#define VOLUMEL { .v = (const char*[]){ "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-5%", NULL } }
#define VOLUMEM { .v = (const char*[]){ "pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle", NULL } }
#define VOLUMER { .v = (const char*[]){ "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+5%", NULL } }

#define DICTIONARYHISTORY { .v = (const char*[]){ "termite", "--name=floating_Termite", "-t", "Dictionary", "-e", SCRIPT("dictionary_history.sh") } }

#define INHIBITSUSPEND0 { .v = (const char*[]){ "systemd-inhibit", "--what=handle-lid-switch", SCRIPT("inhibitsuspend0.sh"), NULL } }
#define INHIBITSUSPEND1 { .v = (const char*[]){ "systemd-inhibit", "--what=handle-lid-switch", SCRIPT("inhibitsuspend1.sh"), NULL } }

#define DISABLEDEMODE SHCMD("xmodmap /home/ashish/.Xmodmap_de0 && notify-send -h string:x-canonical-private-synchronous:demode -t 1000 'data entry mode deactivated'")
#define ENABLEDEMODE SHCMD("xmodmap /home/ashish/.Xmodmap_de1 && notify-send -h string:x-canonical-private-synchronous:demode -t 0 'data entry mode activated'")

#define NOTIFYSCRATCHED { .v = (const char*[]){ "notify-send", "-t", "1500", "dwm", "scratched focused window", NULL } }
#define NOTIFYUNSCRATCHED { .v = (const char*[]){ "notify-send", "-t", "1500", "dwm", "unscratched focused window", NULL } }

static const Win browser = { .cmd = CMD("brave"), .tag = 8, .scratchkey = -1 };
static const Win mail = { .cmd = SCRIPTCMD("neomutt.sh"), .tag = 9, .scratchkey = -2 };

/* custom function declarations */
static void dynscratchtoggle(const Arg *arg);
static void dynscratchunmark(const Arg *arg);
static void floatmovex(const Arg *arg);
static void floatmovey(const Arg *arg);
static void floatresizeh(const Arg *arg);
static void floatresizew(const Arg *arg);
static void focusmaster(const Arg *arg);
static void focusstackalt(const Arg *arg);
static void focusurgent(const Arg *arg);
static void hideclient(const Arg *arg);
static void hidefloating(const Arg *arg);
static Client *nextsamefloat(int next);
static Client *nextvisible(int next);
static void push(const Arg *arg);
static void scratchhide(const Arg *arg);
static void scratchhidevisible(const Arg *arg);
static void scratchshow(const Arg *arg);
static void scratchtoggle(const Arg *arg);
static void showfloating(const Arg *arg);
static void togglefocusarea(const Arg *arg);
static void togglefocusfloat(const Arg *arg);
static void togglefullscreen(const Arg *arg);
static void vieworprev(const Arg *arg);
static void windowswitcher(const Arg *arg);
static void winview(const Arg* arg);
static void zoomswap(const Arg *arg);
static void zoomvar(const Arg *arg);

#include "inplacerotate.c"
#include <X11/XF86keysym.h>

static Key keys[] = {
	/* modifier                     key             function                argument */
	{ MODLKEY,                      XK_d,           spawn,                  ROFIDRUN },
	{ MODLKEY|ShiftMask,            XK_d,           spawn,                  ROFIRUN },
	{ MODLKEY,                      XK_Return,      spawn,                  CMD("termite") },
	{ MODLKEY,                      XK_b,           togglebar,              {0} },
	{ MODLKEY|ShiftMask,            XK_b,           tabmode,                {-1} },
	{ MODLKEY,                      XK_j,           focusstackalt,          {.i = +1 } },
	{ MODLKEY,                      XK_Down,        focusstackalt,          {.i = +1 } },
	{ MODLKEY,                      XK_k,           focusstackalt,          {.i = -1 } },
	{ MODLKEY,                      XK_Up,          focusstackalt,          {.i = -1 } },
	{ MODLKEY,                      XK_h,           setmfact,               {.f = -0.05} },
	{ MODLKEY,                      XK_Left,        setmfact,               {.f = -0.05} },
	{ MODLKEY,                      XK_l,           setmfact,               {.f = +0.05} },
	{ MODLKEY,                      XK_Right,       setmfact,               {.f = +0.05} },
	{ MODLKEY,                      XK_equal,       setsplus,               {.i = +40} },
	{ MODLKEY,                      XK_minus,       setsplus,               {.i = -40} },
	{ MODLKEY|ShiftMask,            XK_equal,       setsplus,               {.i = +20} },
	{ MODLKEY|ShiftMask,            XK_minus,       setsplus,               {.i = -20} },
	{ MODLKEY,                      XK_BackSpace,   setsplus,               {.i = 0} },
	{ MODLKEY|ShiftMask,            XK_BackSpace,   resetsplus,             {0} },
	{ MODLKEY|ShiftMask,            XK_j,           push,                   {.i = +1} },
	{ MODLKEY|ShiftMask,            XK_Down,        push,                   {.i = +1} },
	{ MODLKEY|ShiftMask,            XK_k,           push,                   {.i = -1} },
	{ MODLKEY|ShiftMask,            XK_Up,          push,                   {.i = -1} },
	{ SUPKEY,                       XK_comma,       inplacerotate,          {.i = +1 } },
	{ SUPKEY,                       XK_period,      inplacerotate,          {.i = +2 } },
	{ SUPKEY|ShiftMask,             XK_comma,       inplacerotate,          {.i = -1 } },
	{ SUPKEY|ShiftMask,             XK_period,      inplacerotate,          {.i = -2 } },
	{ SUPKEY,                       XK_j,           floatmovey,             {.i = +20} },
	{ SUPKEY,                       XK_Down,        floatmovey,             {.i = +20} },
	{ SUPKEY,                       XK_k,           floatmovey,             {.i = -20} },
	{ SUPKEY,                       XK_Up,          floatmovey,             {.i = -20} },
	{ SUPKEY,                       XK_h,           floatmovex,             {.i = -20} },
	{ SUPKEY,                       XK_Left,        floatmovex,             {.i = -20} },
	{ SUPKEY,                       XK_l,           floatmovex,             {.i = +20} },
	{ SUPKEY,                       XK_Right,       floatmovex,             {.i = +20} },
	{ SUPKEY|ShiftMask,             XK_j,           floatresizeh,           {.i = +20} },
	{ SUPKEY|ShiftMask,             XK_Down,        floatresizeh,           {.i = +20} },
	{ SUPKEY|ShiftMask,             XK_k,           floatresizeh,           {.i = -20} },
	{ SUPKEY|ShiftMask,             XK_Up,          floatresizeh,           {.i = -20} },
	{ SUPKEY|ShiftMask,             XK_h,           floatresizew,           {.i = -20} },
	{ SUPKEY|ShiftMask,             XK_Left,        floatresizew,           {.i = -20} },
	{ SUPKEY|ShiftMask,             XK_l,           floatresizew,           {.i = +20} },
	{ SUPKEY|ShiftMask,             XK_Right,       floatresizew,           {.i = +20} },
	{ MODLKEY,                      XK_i,           incnmaster,             {.i = +1 } },
	{ MODLKEY|ShiftMask,            XK_i,           incnmaster,             {.i = -1 } },
	{ MODLKEY,                      XK_space,       zoomvar,                {.i = 1} },
	{ MODLKEY|ShiftMask,            XK_space,       zoomvar,                {.i = 0} },
	{ SUPKEY,                       XK_space,       view,                   {0} },
	{ SUPKEY|ShiftMask,             XK_space,       moveprevtag,            {0} },
	{ MODLKEY,                      XK_f,           togglefocusfloat,       {0} },
	{ MODLKEY|ShiftMask,            XK_f,           togglefullscreen,       {0} },
	{ SUPKEY,                       XK_f,           togglefloating,         {.i = 1} },
	{ SUPKEY|ShiftMask,             XK_f,           togglefloating,         {.i = 0} },
	{ MODLKEY,                      XK_Escape,      killclient,             {0} },
	{ MODLKEY,                      XK_e,           setltorprev,            {.v = &layouts[0]} },
	{ MODLKEY|ShiftMask,            XK_e,           setltorprev,            {.v = &layouts[1]} },
	{ MODLKEY,                      XK_w,           setltorprev,            {.v = &layouts[2]} },
	{ MODLKEY|ShiftMask,            XK_w,           setltorprev,            {.v = &layouts[3]} },
	{ MODLKEY,                      XK_F1,          setattorprev,           {.v = &attachs[0]} },
	{ MODLKEY,                      XK_F2,          setattorprev,           {.v = &attachs[1]} },
	{ MODLKEY,                      XK_F3,          setattorprev,           {.v = &attachs[2]} },
	{ MODLKEY,                      XK_F4,          setattorprev,           {.v = &attachs[3]} },
	{ MODLKEY,                      XK_F5,          setattorprev,           {.v = &attachs[4]} },
	{ MODLKEY,                      XK_Tab,         focuslast,              {.i = 0} },
	{ MODLKEY|ShiftMask,            XK_Tab,         focuslast,              {.i = 1} },
	{ SUPKEY,                       XK_Tab,         focuslastvisible,       {.i = 0} },
	{ SUPKEY|ShiftMask,             XK_Tab,         focuslastvisible,       {.i = 1} },
	{ MODLKEY,                      XK_m,           focusmaster,            {0} },
	{ MODLKEY,                      XK_g,           focusurgent,            {0} },
	{ MODLKEY,                      XK_o,           winview,                {0} },
	{ MODLKEY,                      XK_q,           windowswitcher,         {.i = 0} },
	{ MODLKEY|ControlMask,          XK_q,           windowswitcher,         {.i = 1} },
	{ MODLKEY,                      XK_comma,       shiftview,              {.i = -1 } },
	{ MODLKEY,                      XK_period,      shiftview,              {.i = +1 } },
	{ MODLKEY|ShiftMask,            XK_comma,       shifttag,               {.i = -1 } },
	{ MODLKEY|ShiftMask,            XK_period,      shifttag,               {.i = +1 } },
	{ MODLKEY|ControlMask,          XK_period,      hideclient,             {0} },
	{ SUPKEY,                       XK_F1,          scratchtoggle,          {.i = 1} },
	{ MODRKEY,                      XK_m,           scratchtoggle,          {.i = 2} },
	{ SUPKEY,                       XK_p,           scratchtoggle,          {.i = 3} },
	{ SUPKEY,                       XK_c,           scratchtoggle,          {.i = 4} },
	{ SUPKEY,                       XK_s,           scratchtoggle,          {.i = 5} },
	{ SUPKEY,                       XK_w,           scratchtoggle,          {.i = 6} },
	{ MODLKEY,                      XK_t,           dynscratchtoggle,       {.i = 8} },
	{ MODLKEY|ShiftMask,            XK_t,           dynscratchunmark,       {.i = 8} },
	{ MODLKEY,                      XK_y,           dynscratchtoggle,       {.i = 9} },
	{ MODLKEY|ShiftMask,            XK_y,           dynscratchunmark,       {.i = 9} },
	{ MODLKEY,                      XK_u,           dynscratchtoggle,       {.i = 10} },
	{ MODLKEY|ShiftMask,            XK_u,           dynscratchunmark,       {.i = 10} },
	{ MODLKEY,                      XK_s,           togglefocusarea,        {0} },
	{ MODRKEY,                      XK_space,       togglewin,              {.v = &browser} },
	{ SUPKEY,                       XK_m,           togglewin,              {.v = &mail} },
	{ 0,                            XK_Print,       spawn,                  SCRIPTCMD("screenshot.sh") },
	{ MODLKEY,                      XK_c,           spawn,                  SCRIPTCMD("color_under_cursor.sh") },
	{ MODLKEY,                      XK_F7,          spawn,                  DISABLEDEMODE },
	{ MODLKEY,                      XK_F8,          spawn,                  ENABLEDEMODE },
	{ MODLKEY,                      XK_semicolon,   spawn,                  SCRIPTCMD("dictionary.sh", "selection") },
	{ MODLKEY|ShiftMask,            XK_semicolon,   spawn,                  SCRIPTCMD("dictionary.sh") },
	{ MODLKEY|ControlMask,          XK_semicolon,   spawn,                  SCRIPTCMD("dictionary_last.sh") },
	{ SUPKEY,                       XK_semicolon,   spawn,                  SCRIPTCMD("espeak.sh", "selection") },
	{ SUPKEY|ShiftMask,             XK_semicolon,   spawn,                  SCRIPTCMD("espeak.sh") },
	{ SUPKEY|ControlMask,           XK_semicolon,   spawn,                  SCRIPTCMD("espeak_last.sh") },
	{ MODLKEY|ControlMask,          XK_apostrophe,  spawn,                  DICTIONARYHISTORY },
	{ MODLKEY|ShiftMask,            XK_q,           spawn,                  SCRIPTCMD("quit.sh") },
	{ MODLKEY|ControlMask,          XK_h,           spawn,                  SCRIPTCMD("hotspot_launch.sh") },
	{ MODLKEY|ControlMask,          XK_m,           spawn,                  SCRIPTCMD("toggletouchpad.sh") },
	{ MODLKEY|ControlMask,          XK_r,           spawn,                  SCRIPTCMD("reflector_launch.sh") },
	{ MODLKEY,                      XK_F10,         spawn,                  SCRIPTCMD("systemctl_timeout.sh", "restart") },
	{ MODLKEY|ShiftMask,            XK_F10,         spawn,                  SCRIPTCMD("systemctl_timeout.sh", "toggle") },
	{ MODLKEY|ControlMask,          XK_F10,         spawn,                  SCRIPTCMD("systemctl_timeout.sh", "status") },
	{ SUPKEY,                       XK_b,           spawn,                  SCRIPTCMD("gbtns.sh") },
	{ SUPKEY,                       XK_r,           spawn,                  TERMCMD("ranger --cmd='set show_hidden=false'") },
	{ SUPKEY|ShiftMask,             XK_r,           spawn,                  TERMCMD("ranger") },
	{ SUPKEY,                       XK_t,           spawn,                  TERMCMD("htop") },
	{ SUPKEY|ShiftMask,             XK_t,           spawn,                  TERMCMD("htop -s PERCENT_CPU") },
	{ SUPKEY|ShiftMask,             XK_m,           spawn,                  TERMSCRIPTCMD("neomutt.sh") },
	{ MODRKEY,                      XK_s,           spawn,                  INHIBITSUSPEND1 },
	{ MODRKEY|ShiftMask,            XK_s,           spawn,                  INHIBITSUSPEND0 },
	{ MODRKEY,                      XK_l,           spawn,                  SCRIPTCMD("ytmsclu.sh", "1") },
	{ MODRKEY,                      XK_k,           spawn,                  SCRIPTCMD("ytmsclu.sh", "0") },
	{ MODRKEY,                      XK_semicolon,   spawn,                  SCRIPTCMD("ytresume.sh") },
	{ MODRKEY,                      XK_Delete,      spawn,                  SCRIPTCMD("usbmount.sh") },

	{ MODLKEY,           XK_bracketleft,            hidefloating,           {0} },
	{ MODLKEY,           XK_bracketright,           showfloating,           {0} },
	{ MODLKEY,           XK_backslash,              scratchhidevisible,     {0} },
	{ 0,                 XF86XK_AudioMute,          spawn,                  VOLUMEM },
	{ 0,                 XF86XK_AudioLowerVolume,   spawn,                  VOLUMEL },
	{ 0,                 XF86XK_AudioRaiseVolume,   spawn,                  VOLUMER },
	{ 0,                 XF86XK_MonBrightnessDown,  spawn,                  SCRIPTCMD("btnsd.sh") },
	{ 0,                 XF86XK_MonBrightnessUp,    spawn,                  SCRIPTCMD("btnsi.sh") },
	{ ShiftMask,         XK_F2,                     spawn,                  SCRIPTCMD("btnsds.sh") },
	{ ShiftMask,         XK_F3,                     spawn,                  SCRIPTCMD("btnsis.sh") },
	{ MODRKEY,           XK_Escape,                 spawn,                  REDSHIFTDEFAULT },
	{ MODRKEY,           XK_F1,                     spawn,                  REDSHIFT("5500") },
	{ MODRKEY,           XK_F2,                     spawn,                  REDSHIFT("5000") },
	{ MODRKEY,           XK_F3,                     spawn,                  REDSHIFT("4500") },
	{ MODRKEY,           XK_F4,                     spawn,                  REDSHIFT("4100") },
	{ MODRKEY,           XK_F5,                     spawn,                  REDSHIFT("3800") },
	{ MODRKEY,           XK_F6,                     spawn,                  REDSHIFT("3500") },
	{ MODRKEY,           XK_F7,                     spawn,                  REDSHIFT("3200") },
	{ MODRKEY,           XK_F8,                     spawn,                  REDSHIFT("2900") },
	{ MODRKEY,           XK_F9,                     spawn,                  REDSHIFT("2600") },
	{ MODRKEY,           XK_F10,                    spawn,                  REDSHIFT("2400") },
	{ MODRKEY,           XK_F11,                    spawn,                  REDSHIFT("2200") },
	{ MODRKEY,           XK_F12,                    spawn,                  REDSHIFT("2000") },

	{ SUPKEY|MODRKEY,               XK_1,           focustiled,             {.i = 0  } },
	{ SUPKEY|MODRKEY,               XK_2,           focustiled,             {.i = 1  } },
	{ SUPKEY|MODRKEY,               XK_3,           focustiled,             {.i = 2  } },
	{ SUPKEY|MODRKEY,               XK_4,           focustiled,             {.i = 3  } },
	{ SUPKEY|MODRKEY,               XK_5,           focustiled,             {.i = 4  } },
	{ SUPKEY|MODRKEY,               XK_6,           focustiled,             {.i = 5  } },
	{ SUPKEY|MODRKEY,               XK_7,           focustiled,             {.i = 6  } },
	{ SUPKEY|MODRKEY,               XK_8,           focustiled,             {.i = 7  } },
	{ SUPKEY|MODRKEY,               XK_9,           focustiled,             {.i = 8  } },
	{ SUPKEY|MODRKEY,               XK_0,           focustiled,             {.i = 9  } },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_1,           focustiled,             {.i = 10 } },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_2,           focustiled,             {.i = 11 } },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_3,           focustiled,             {.i = 12 } },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_4,           focustiled,             {.i = 13 } },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_5,           focustiled,             {.i = 14 } },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_6,           focustiled,             {.i = 15 } },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_7,           focustiled,             {.i = 16 } },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_8,           focustiled,             {.i = 17 } },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_9,           focustiled,             {.i = 18 } },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_0,           focustiled,             {.i = 19 } },
	{ MODLKEY,                      XK_0,           vieworprev,             {.ui = ~0 } },
	{ MODLKEY|ShiftMask,            XK_0,           tag,                    {.ui = ~0 } },
	TAGKEYS(                        XK_1,                                   0)
	TAGKEYS(                        XK_2,                                   1)
	TAGKEYS(                        XK_3,                                   2)
	TAGKEYS(                        XK_4,                                   3)
	TAGKEYS(                        XK_5,                                   4)
	TAGKEYS(                        XK_6,                                   5)
	TAGKEYS(                        XK_7,                                   6)
	TAGKEYS(                        XK_8,                                   7)
	TAGKEYS(                        XK_9,                                   8)
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
	{ ClkClientWin,         MODLKEY,        Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODLKEY,        Button2,        togglefloating, {.i = 0} },
	{ ClkClientWin,         MODLKEY,        Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        vieworprev,     {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODLKEY,        Button1,        tag,            {0} },
	{ ClkTagBar,            MODLKEY,        Button3,        toggletag,      {0} },
};

#define FSIGID                          "z:"
#define FSIGIDLEN                       (sizeof FSIGID - 1)
/* set the following lengths without considering the terminating null byte */
#define MAXFSIGNAMELEN                  4
#define MAXFSIGARGLEN                   2

/* trigger signals using `xsetroot -name "FSIGID<signame> [<type> <value>]"` */
/* signal definitions */
static Signal signals[] = {
	/* signame              function */
	{ "quit",               quit },
	{ "scrh",               scratchhide },
	{ "scrs",               scratchshow },
	{ "scrt",               scratchtoggle },
};

/* custom function implementations */
void
dynscratchtoggle(const Arg *arg)
{
        if (!selmon->sel)
                scratchshowhelper(arg->i);
        else if (selmon->sel->scratchkey == arg->i)
                scratchhidehelper();
        else if (!scratchshowhelper(arg->i)) {
                selmon->sel->scratchkey = arg->i;
                spawn(&((Arg)NOTIFYSCRATCHED));
        }
}

void
dynscratchunmark(const Arg *arg)
{
        if (selmon->sel && selmon->sel->scratchkey == arg->i) {
                selmon->sel->scratchkey = 0;
                spawn(&((Arg)NOTIFYUNSCRATCHED));
        }
}

void
floatmovex(const Arg *arg)
{
        int nx, cw, mw;

        if (!selmon->sel || (!selmon->sel->isfloating && selmon->lt[selmon->sellt]->arrange))
                return;
        if (selmon->sel->isfullscreen)
                return;
        nx = selmon->sel->x + arg->i;
        cw = WIDTH(selmon->sel);
        mw = selmon->wx + selmon->ww;
        /* snap to monitor edge on first try of crossover */
        if (selmon->sel->x > selmon->wx && nx < selmon->wx)
                nx = selmon->wx;
        else if (selmon->sel->x + cw < mw && nx + cw > mw)
                nx = mw - cw;
        resize(selmon->sel, nx, selmon->sel->y, selmon->sel->w, selmon->sel->h, 1);
}

void
floatmovey(const Arg *arg)
{
        int ny, ch, mh;

        if (!selmon->sel || (!selmon->sel->isfloating && selmon->lt[selmon->sellt]->arrange))
                return;
        if (selmon->sel->isfullscreen)
                return;
        ny = selmon->sel->y + arg->i;
        ch = HEIGHT(selmon->sel);
        mh = selmon->wy + selmon->wh;
        /* snap to monitor edge on first try of crossover */
        if (selmon->sel->y > selmon->wy && ny < selmon->wy)
                ny = selmon->wy;
        else if (selmon->sel->y + ch < mh && ny + ch > mh)
                ny = mh - ch;
        resize(selmon->sel, selmon->sel->x, ny, selmon->sel->w, selmon->sel->h, 1);
}

void
floatresizeh(const Arg *arg)
{
        int nh, cy, mh;

        if (!selmon->sel || (!selmon->sel->isfloating && selmon->lt[selmon->sellt]->arrange))
                return;
        if (selmon->sel->isfullscreen)
                return;
        nh = selmon->sel->h + arg->i;
        cy = selmon->sel->y + 2 * selmon->sel->bw;
        mh = selmon->wy + selmon->wh;
        /* snap to monitor edge on first try of crossover */
        if (cy + selmon->sel->h < mh && cy + nh > mh)
                nh = mh - cy;
        resize(selmon->sel, selmon->sel->x, selmon->sel->y, selmon->sel->w, nh, 1);
}

void
floatresizew(const Arg *arg)
{
        int nw, cx, mw;

        if (!selmon->sel || (!selmon->sel->isfloating && selmon->lt[selmon->sellt]->arrange))
                return;
        if (selmon->sel->isfullscreen)
                return;
        nw = selmon->sel->w + arg->i;
        cx = selmon->sel->x + 2 * selmon->sel->bw;
        mw = selmon->wx + selmon->ww;
        /* snap to monitor edge on first try of crossover */
        if (cx + selmon->sel->w < mw && cx + nw > mw)
                nw = mw - cx;
        resize(selmon->sel, selmon->sel->x, selmon->sel->y, nw, selmon->sel->h, 1);
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
        if (!selmon->sel->isfloating && selmon->lt[selmon->sellt]->arrange == deck
                                     && selmon->ntiles > selmon->nmaster + 1) {
                int n;

                for (n = 1, c = selmon->clients; c != selmon->sel; c = c->next)
                        if (!c->isfloating && ISVISIBLE(c))
                                n++;
                c = NULL;
                if (arg->i > 0) {
                        if (n == selmon->nmaster) /* focus first master client */
                                for (c = selmon->clients;
                                     c->isfloating || !ISVISIBLE(c);
                                     c = c->next);
                        else if (n == selmon->ntiles) /* focus first stack client */
                                for (n = selmon->nmaster, c = selmon->clients;
                                     c->isfloating || !ISVISIBLE(c) || n-- > 0;
                                     c = c->next);
                        else /* focus next client */
                                for (c = selmon->sel->next;
                                     c->isfloating || !ISVISIBLE(c);
                                     c = c->next);
                } else {
                        if (selmon->nmaster && n == 1) /* focus last master client */
                                for (n = selmon->nmaster, c = selmon->clients;
                                     c->isfloating || !ISVISIBLE(c) || --n > 0;
                                     c = c->next);
                        else if (n == selmon->nmaster + 1) /* focus last stack client */
                                for (n = selmon->ntiles, c = selmon->clients;
                                     c->isfloating || !ISVISIBLE(c) || --n > 0;
                                     c = c->next);
                        else /* focus previous client */
                                for (c = selmon->clients;
                                     c->isfloating || !ISVISIBLE(c) || --n > 1;
                                     c = c->next);
                }
        } else if (selmon->lt[selmon->sellt]->arrange)
                c = nextsamefloat(arg->i);
        else
                c = nextvisible(arg->i);
	if (c) {
		focusalt(c);
		restack(selmon, 0);
        }
}

void
focusurgent(const Arg *arg)
{
        for (Monitor *m = mons; m; m = m->next)
                for (Client *c = m->stack; c; c = c->snext)
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
                       selmon->sel->sfw, selmon->sel->sfh, 0);
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

/* the following two functions assume non-NULL selmon->sel */
Client *
nextsamefloat(int next)
{
        Client *c;

        if (next > 0) {
                if (selmon->sel->isfloating) {
                        for (c = selmon->sel->next;
                             c && (!c->isfloating || !ISVISIBLE(c));
                             c = c->next);
                        if (!c)
                                for (c = selmon->clients;
                                     c && (!c->isfloating || !ISVISIBLE(c));
                                     c = c->next);
                } else {
                        for (c = selmon->sel->next;
                             c && (c->isfloating || !ISVISIBLE(c));
                             c = c->next);
                        if (!c)
                                for (c = selmon->clients;
                                     c && (c->isfloating || !ISVISIBLE(c));
                                     c = c->next);
                }
        } else {
                Client *i;

                c = NULL;
                if (selmon->sel->isfloating) {
                        for (i = selmon->clients; i != selmon->sel; i = i->next)
                                if (i->isfloating && ISVISIBLE(i))
                                        c = i;
                        if (!c)
                                for (; i; i = i->next)
                                        if (i->isfloating && ISVISIBLE(i))
                                                c = i;
                } else {
                        for (i = selmon->clients; i != selmon->sel; i = i->next)
                                if (!i->isfloating && ISVISIBLE(i))
                                        c = i;
                        if (!c)
                                for (; i; i = i->next)
                                        if (!i->isfloating && ISVISIBLE(i))
                                                c = i;
                }
        }
        return c;
}

Client *
nextvisible(int next)
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
push(const Arg *arg)
{
        Client *c;
	Client *ps = NULL, *pc = NULL;
        Client *i, *tmp;

        if (!selmon->sel || !selmon->lt[selmon->sellt]->arrange)
                return;
        c = nextsamefloat(arg->i);
        if (!c || c == selmon->sel)
                return;
	for (i = selmon->clients; i && (!ps || !pc); i = i->next) {
		if (i->next == selmon->sel)
			ps = i;
		if (i->next == c)
			pc = i;
	}
	/* swap c and selmon->sel in the selmon->clients list */
        tmp = selmon->sel->next == c ? selmon->sel : selmon->sel->next;
        selmon->sel->next = c->next == selmon->sel ? c : c->next;
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
scratchhide(const Arg *arg)
{
        if (selmon->sel && selmon->sel->scratchkey == arg->i)
                scratchhidehelper();
}

void
scratchhidevisible(const Arg *arg)
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

void
scratchshow(const Arg *arg)
{
        if (selmon->sel && selmon->sel->scratchkey == arg->i)
                return;
        if (!scratchshowhelper(arg->i))
                spawn(&((Arg){ .v = scratchcmds[arg->i - 1] }));
}

void
scratchtoggle(const Arg *arg)
{
        if (selmon->sel && selmon->sel->scratchkey == arg->i)
                scratchhidehelper();
        else if (!scratchshowhelper(arg->i))
                spawn(&((Arg){ .v = scratchcmds[arg->i - 1] }));
}

void
showfloating(const Arg *arg)
{
        Client *f = NULL; /* last focused hidden floating client */

        for (Client *c = selmon->stack; c; c = c->snext)
                if (c->ishidden && c->tags & selmon->tagset[selmon->seltags]) {
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
togglefocusarea(const Arg *arg)
{
        int curidx, inrel;
        Client *c, *i;

        if (!selmon->sel || selmon->sel->isfloating || !selmon->lt[selmon->sellt]->arrange)
                return;
        for (curidx = 0, i = selmon->clients; i != selmon->sel; i = i->next)
                if (!i->isfloating && ISVISIBLE(i))
                        curidx++;
        inrel = (curidx < selmon->nmaster);
        c = selmon->sel;
        do {
                do
                        c = c->snext;
                while (c && (c->isfloating || !ISVISIBLE(c)));
                if (c) {
                        for (curidx = 0, i = selmon->clients; i != c; i = i->next)
                                if (!i->isfloating && ISVISIBLE(i))
                                        curidx++;
                } else
                        return;
        } while ((curidx < selmon->nmaster) == inrel);
        focusalt(c);
        restack(selmon, 0);
}

void
togglefocusfloat(const Arg *arg)
{
        Client *c;

        if (!selmon->sel || !selmon->lt[selmon->sellt]->arrange)
                return;
        if (selmon->sel->isfloating)
                for (c = selmon->sel; c && (c->isfloating || !ISVISIBLE(c)); c = c->snext);
        else
                for (c = selmon->sel; c && (!c->isfloating || !ISVISIBLE(c)); c = c->snext);
        if (c) {
                focusalt(c);
                restack(selmon, 0);
        }
}

void
togglefullscreen(const Arg *arg)
{
        int found = 0;

        for (Client *c = selmon->clients; c; c = c->next)
                if (ISVISIBLE(c) && c->isfullscreen) {
                        found = 1;
                        setfullscreen(c, 0);
                }
        if (!found && selmon->sel)
                setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
}

void
vieworprev(const Arg *arg)
{
	view((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags] ? &((Arg){0}) : arg);
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
                for (Client *c = m->stack; c; c = c->snext)
                        XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                                        PropModePrepend, (unsigned char *) &(c->win), 1);
        }
        if (arg->i)
                spawn(&((Arg)ROFIWINREGEX));
        else
                spawn(&((Arg)ROFIWIN));
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
	Client *nmc, *bnmc, *omc;

	if (!selmon->sel)
		return;
        if (selmon->sel->isfloating || !selmon->lt[selmon->sellt]->arrange) {
                resize(selmon->sel,
                       selmon->mx + (selmon->mw - WIDTH(selmon->sel)) / 2,
                       selmon->my + (selmon->mh - HEIGHT(selmon->sel)) / 2,
                       selmon->sel->w, selmon->sel->h, 0);
                return;
        }
        nmc = selmon->sel;
        omc = nexttiled(selmon->clients);
	if (nmc == omc) {
                do
                        nmc = nmc->snext;
                while (nmc && (nmc->isfloating || !ISVISIBLE(nmc)));
                if (!nmc)
                        return;
        } else {
                Client **bomc;

                /* make omc the "snext" without focusing it */
                for (bomc = &omc->mon->stack; *bomc && *bomc != omc; bomc = &(*bomc)->snext);
                *bomc = omc->snext;
                attachstack(omc);
        }
        for (bnmc = selmon->clients; bnmc->next != nmc; bnmc = bnmc->next);
	detach(nmc);
	attach(nmc);
	/* swap nmc and omc instead of pushing the omc down */
	if (bnmc != omc) {
                detach(omc);
                omc->next = bnmc->next;
                bnmc->next = omc;
	}
	focusalt(nmc);
	arrange(selmon);
}

void
zoomvar(const Arg *arg)
{
        if (selmon->lt[selmon->sellt]->arrange == deck && selmon->ntiles > selmon->nmaster + 1) {
                if (arg->i)
                        zoomswap(&((Arg){0}));
                else
                        zoom(&((Arg){0}));
        } else {
                if (arg->i)
                        zoom(&((Arg){0}));
                else
                        zoomswap(&((Arg){0}));
        }
}

/* Window rules */
static void center(Client *c);
static void markscratch(Client *c, int scratchkey);

static void
applyrules(Client *c)
{
	const char *class, *instance;
	XClassHint ch = { NULL, NULL };

	XGetClassHint(dpy, c->win, &ch);
	class = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name ? ch.res_name : broken;

        if (strcmp(instance, "calcurse_Termite") == 0) {
                markscratch(c, 4);
                c->isfloating = 1;
                c->w = 950;
                c->h = 650;
                center(c);
        } else if (strcmp(instance, "floating_Termite") == 0) {
                c->isfloating = 1;
                c->w = 750;
                c->h = 450;
                center(c);
        } else if (strcmp(instance, "neomutt_Termite") == 0) {
                markscratch(c, -2);
        } else if (strcmp(instance, "pyfzf_Termite") == 0) {
                markscratch(c, 3);
                c->isfloating = 1;
                c->w = 750;
                c->h = 450;
                center(c);
        } else if (strcmp(instance, "scratch_Termite") == 0) {
                markscratch(c, 1);
                c->isfloating = 1;
                c->w = 750;
                c->h = 450;
                center(c);
        } else if (strcmp(instance, "brave-browser") == 0) {
                markscratch(c, -1);
        } else if (strcmp(instance, "crx_cinhimbnkkaeohfgghhklpknlkffjgod") == 0) {
                markscratch(c, 2);
                c->isfloating = 1;
                c->bw = 0;
                c->w = 950;
                c->h = 626;
                center(c);
        } else if (strcmp(class, "Signal") == 0) {
                markscratch(c, 5);
                c->isfloating = 1;
                c->w = 880;
                c->h = 620;
                center(c);
        } else if (strcmp(class, "TelegramDesktop") == 0) {
                markscratch(c, 6);
                c->isfloating = 1;
                c->w = 770;
                c->h = 555;
                center(c);
        } else if (strcmp(class, "Sxiv") == 0 ||
                   strstr(class, "Yad") ||
                   strcmp(class, "Matplotlib") == 0 ||
                   strcmp(class, "Woeusbgui") == 0 ||
                   strcmp(c->name, "Event Tester") == 0) {
                c->isfloating = 1;
                center(c);
        } else if (strcmp(c->name, "Picture in picture") == 0 ||
                   strcmp(c->name, "Picture-in-Picture") == 0) {
                c->isfloating = 1;
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
markscratch(Client *c, int key)
{
        for (Monitor *m = mons; m; m = m->next)
                for (Client *i = m->clients; i; i = i->next)
                        if (i->scratchkey == key)
                                return;
        c->scratchkey = key;
}
