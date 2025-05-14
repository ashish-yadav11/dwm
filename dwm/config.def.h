/* See LICENSE file for copyright and license details. */

/* maximum number of tabs to show */
#define MAXTABS                         15
/* systray icon height, change to bh to keep it equal to font height */
#define SYSTRAYHEIGTH                   16
/* spacing between systray icons */
#define SYSTRAYSPACING                  4
/* colorscheme used for background of systray */
#define SCHEMESYSTRAY                   SchemeNorm

#define SCRIPT(name)                    "/home/ashish/.scripts/"name

#define CMD(...)                        { .v = (const char*[]){ __VA_ARGS__, NULL } }
#define SCRIPTCMD(...)                  { .v = (const char*[]){ SCRIPT("")__VA_ARGS__, NULL } }
#define SHCMD(cmd)                      { .v = (const char*[]){ "dash", "-c", cmd, NULL } }
#define TERMCMD(...)                    { .v = (const char*[]){ "st", "-e", __VA_ARGS__, NULL } }
#define TERMSHCMD(cmd)                  { .v = (const char*[]){ "st", "-e", "dash", "-c", cmd, NULL } }

/* command used to notify that a client has been removed from dynamic scratchpad */
#define NOTIFYDYNSCRATCH0               { .v = (const char*[]){ "notify-send", "-h", "string:x-canonical-private-synchronous:scratch", "-t", "1500", "dwm", "unscratched focused window", NULL } }
/* command used to notify that a client has been added to dynamic scratchpad */
#define NOTIFYDYNSCRATCH1               { .v = (const char*[]){ "notify-send", "-h", "string:x-canonical-private-synchronous:scratch", "-t", "1500", "dwm", "scratched focused window", NULL } }
/* command used to notify that a client already has a scratch mark */
#define NOTIFYDYNSCRATCH2               { .v = (const char*[]){ "notify-send", "-h", "string:x-canonical-private-synchronous:scratch", "-t", "1500", "dwm", "focused window already scratched", NULL } }
/* window switcher command */
#define ROFIWIN                         { .v = (const char*[]){ "rofi", "-show", "window", NULL } }
/* alternate window switcher command */
#define ROFIWINREGEX                    { .v = (const char*[]){ "rofi", "-show", "window", "-matching", "regex", NULL } }

typedef struct {
        const Arg cmd;
        const unsigned int tag;
        const int scratchkey;
} Win;

static const unsigned int borderpx      = 2;    /* border pixel of windows */
static const unsigned int snap          = 10;   /* snap pixel */
static const float mfact                = 0.60; /* factor of master area size (0.05 - 0.95) */
static const int nmaster                = 1;    /* number of clients in master area */
static const int resizehints            = 0;    /* 1 means respect size hints in tiled resizals */
static const int gappih                 = 1;    /* horiz inner gap between windows */
static const int gappiv                 = 1;    /* vert inner gap between windows */
static const int gappoh                 = 1;    /* horiz outer gap between windows and screen edge */
static const int gappov                 = 0;    /* vert outer gap between windows and screen edge */
static const int showsystray            = 1;    /* 0 means no systray */
static const int showbar                = 1;    /* 0 means no bar */
static const int topbar                 = 1;    /* 0 means bottom bar */

/* display modes of the tab bar, modes after ShowtabPivot are disabled */
enum { ShowtabNever, ShowtabAuto, ShowtabPivot, ShowtabAlways};

static const int showtab                = ShowtabAuto; /* default tab bar display mode */
static const int toptab                 = 0;           /* 0 means bottom tab bar */

static const char *fonts[] = {
        "Fira Sans:size=12",
        "Siji:pixelsize=12",
        "Noto Color Emoji:pixelsize=12",
};

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

static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" };

/* first element is for all-tag view */
static int def_layouts[1 + LENGTH(tags)] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2};
static int def_attachs[1 + LENGTH(tags)] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1};

static const Attach attachs[] = {
	/* symbol       attach function */
	{ "T",          attach },
	{ "U",          attachabove },
	{ "D",          attachbelow },
	{ "S",          attachaside },
	{ "B",          attachbottom },
};

static const Layout layouts[] = {
       /* symbol       arrange function        default attach */
       { "[ ]=",       tilehor,                &attachs[0] },
       { "[ . ]",      NULL,                   &attachs[0] }, /* no layout function means floating behavior */
       { "[M]",        monocle,                &attachs[1] },
       { "[H]",        deckhor,                &attachs[3] },
       { "=[ ]",       tilever,                &attachs[0] },
       { "[V]",        deckver,                &attachs[3] },
};

#define ASKLAUNCH(name, ...)            (const char *[]){ SCRIPT("asklaunch.sh"), name, __VA_ARGS__, NULL }

static const char *const *scratchcmds[] = {
	(const char *[]){ "st", "-n", "scratch-st", NULL },
	ASKLAUNCH("YouTube Music", "brave", "--app-id=cinhimbnkkaeohfgghhklpknlkffjgod"),
	(const char *[]){ "st", "-n", "pyfzf-st", "-e", "pyfzf", NULL },
	(const char *[]){ "st", "-n", "calcurse-st", "-t", "Calcurse", "-e", "calcurse", NULL },
	ASKLAUNCH("Signal", "signal-desktop", "--use-tray-icon"),
	ASKLAUNCH("Telegram", "telegram-desktop"),
	(const char *[]){ SCRIPT("scrcpy.sh"), NULL },
	(const char *[]){ "st", "-n", "music-st", "-e", "ranger", "/media/storage/Music", NULL },
	(const char *[]){ "st", "-n", "neovim-st", "-e", "nvim", NULL },
};

#define DYNSCRATCHKEY(i)                (LENGTH(scratchcmds) + i)

#include "inplacerotate.c"
#include <X11/XF86keysym.h>

#define MODLKEY Mod3Mask
#define MODRKEY Mod1Mask
#define SUPKEY  Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODLKEY,                      KEY,      vieworprev,     {.ui = 1 << TAG} }, \
	{ MODLKEY|ShiftMask,            KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODLKEY|ControlMask,          KEY,      toggletag,      {.ui = 1 << TAG} }, \
	{ SUPKEY,                       KEY,      tagandview,     {.ui = TAG + 1} }, \
	{ SUPKEY|ShiftMask,             KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ SUPKEY|ControlMask,           KEY,      swaptags,       {.ui = TAG} },

#define REDSHIFT(arg)                   { .v = (const char*[]){ "redshift", "-PO" arg, NULL } }
#define REDSHIFTDEFAULT                 { .v = (const char*[]){ "redshift", "-x", NULL } }

#define ROFIDRUN                        { .v = (const char*[]){ "rofi", "-show", "drun", "-show-icons", NULL } }
#define ROFIRUN                         { .v = (const char*[]){ "rofi", "-show", "run", NULL } }

#define VOLUMEM                         { .v = (const char*[]){ "pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle", NULL } }
#define VOLUMEL                         { .v = (const char*[]){ "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-5%", NULL } }
#define VOLUMER                         { .v = (const char*[]){ "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+5%", NULL } }
#define VOLUMEl                         { .v = (const char*[]){ "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-1%", NULL } }
#define VOLUMEr                         { .v = (const char*[]){ "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+1%", NULL } }
#define PLAYERT                         { .v = (const char*[]){ "playerctl", "play-pause", NULL } }
#define PLAYERP                         { .v = (const char*[]){ "playerctl", "previous", NULL } }
#define PLAYERN                         { .v = (const char*[]){ "playerctl", "next", NULL } }

#define INHIBITSUSPEND0                 { .v = (const char*[]){ "systemd-inhibit", "--what=handle-lid-switch", SCRIPT("inhibitsuspend.sh"), NULL } }
#define INHIBITSUSPEND1                 { .v = (const char*[]){ "systemd-inhibit", "--what=handle-lid-switch", SCRIPT("inhibitsuspend.sh"), "lock", NULL } }

#define TOGGLEVPN                       SHCMD("killall -INT riseup-vpn 2>/dev/null || riseup-vpn --start-vpn on")

#define DISABLEDEMODE                   SHCMD("xmodmap /home/ashish/.Xmodmap_de0 && notify-send -h string:x-canonical-private-synchronous:demode -t 1000 'data entry mode deactivated'")
#define ENABLEDEMODE                    SHCMD("xmodmap /home/ashish/.Xmodmap_de1 && notify-send -h string:x-canonical-private-synchronous:demode -t 0 'data entry mode activated'")

static const Win browser = { .cmd = CMD("brave"), .tag = 10, .scratchkey = -1 };
static const Win mail = { .cmd = SCRIPTCMD("neomutt.sh", "scratch"), .tag = 9, .scratchkey = -2 };

enum { MoveX, MoveY, ResizeX, ResizeY, ResizeA }; /* floatmoveresize */

/* custom function declarations */
static void dynscratchtoggle(const Arg *arg);
static void dynscratchunmark(const Arg *arg);
static void floatmoveresize(const Arg *arg);
static void focusmaster(const Arg *arg);
static void focusstackalt(const Arg *arg);
static void focusurgent(const Arg *arg);
static void hideclient(const Arg *arg);
static void hidefloating(const Arg *arg);
static void hideshowfloating(const Arg *arg);
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
static int hasleasttag(Client *c, int tag);
static void windowlineupc(const Arg *arg);
static void windowlineupr(const Arg *arg);
static void windowlineups(const Arg *arg);
static void windowswitcher(const Arg *arg);
static void winview(const Arg* arg);
static void zoomswap(const Arg *arg);
static void zoomvar(const Arg *arg);

static const Key keys[] = {
	/* modifier                     key             function                argument */
	{ MODLKEY,                      XK_d,           spawn,                  ROFIDRUN },
	{ MODLKEY|ShiftMask,            XK_d,           spawn,                  ROFIRUN },
	{ MODLKEY,                      XK_Return,      spawn,                  CMD("st") },
	{ MODLKEY,                      XK_b,           togglebar,              {0} },
	{ MODLKEY|ShiftMask,            XK_b,           tabmode,                {0} },
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
	{ MODLKEY|ShiftMask,            XK_j,           push,                   {.i = +1 } },
	{ MODLKEY|ShiftMask,            XK_Down,        push,                   {.i = +1 } },
	{ MODLKEY|ShiftMask,            XK_k,           push,                   {.i = -1 } },
	{ MODLKEY|ShiftMask,            XK_Up,          push,                   {.i = -1 } },
	{ SUPKEY,                       XK_comma,       inplacerotate,          {.i = +1 } },
	{ SUPKEY,                       XK_period,      inplacerotate,          {.i = +2 } },
	{ SUPKEY|ShiftMask,             XK_comma,       inplacerotate,          {.i = -1 } },
	{ SUPKEY|ShiftMask,             XK_period,      inplacerotate,          {.i = -2 } },
	{ SUPKEY,                       XK_j,           floatmoveresize,        {.v = (int []){MoveY, +20} } },
	{ SUPKEY,                       XK_Down,        floatmoveresize,        {.v = (int []){MoveY, +20} } },
	{ SUPKEY,                       XK_k,           floatmoveresize,        {.v = (int []){MoveY, -20} } },
	{ SUPKEY,                       XK_Up,          floatmoveresize,        {.v = (int []){MoveY, -20} } },
	{ SUPKEY,                       XK_h,           floatmoveresize,        {.v = (int []){MoveX, -20} } },
	{ SUPKEY,                       XK_Left,        floatmoveresize,        {.v = (int []){MoveX, -20} } },
	{ SUPKEY,                       XK_l,           floatmoveresize,        {.v = (int []){MoveX, +20} } },
	{ SUPKEY,                       XK_Right,       floatmoveresize,        {.v = (int []){MoveX, +20} } },
	{ SUPKEY|ShiftMask,             XK_j,           floatmoveresize,        {.v = (int []){ResizeY, +20} }},
	{ SUPKEY|ShiftMask,             XK_Down,        floatmoveresize,        {.v = (int []){ResizeY, +20} }},
	{ SUPKEY|ShiftMask,             XK_k,           floatmoveresize,        {.v = (int []){ResizeY, -20} }},
	{ SUPKEY|ShiftMask,             XK_Up,          floatmoveresize,        {.v = (int []){ResizeY, -20} }},
	{ SUPKEY|ShiftMask,             XK_h,           floatmoveresize,        {.v = (int []){ResizeX, -20} }},
	{ SUPKEY|ShiftMask,             XK_Left,        floatmoveresize,        {.v = (int []){ResizeX, -20} }},
	{ SUPKEY|ShiftMask,             XK_l,           floatmoveresize,        {.v = (int []){ResizeX, +20} }},
	{ SUPKEY|ShiftMask,             XK_Right,       floatmoveresize,        {.v = (int []){ResizeX, +20} }},
	{ SUPKEY|ControlMask,           XK_j,           floatmoveresize,        {.v = (int []){ResizeA, +20} }},
	{ SUPKEY|ControlMask,           XK_Down,        floatmoveresize,        {.v = (int []){ResizeA, +20} }},
	{ SUPKEY|ControlMask,           XK_k,           floatmoveresize,        {.v = (int []){ResizeA, -20} }},
	{ SUPKEY|ControlMask,           XK_Up,          floatmoveresize,        {.v = (int []){ResizeA, -20} }},
	{ MODLKEY,                      XK_i,           incnmaster,             {.i = +1 } },
	{ MODLKEY|ShiftMask,            XK_i,           incnmaster,             {.i = -1 } },
	{ MODLKEY,                      XK_space,       zoomvar,                {.i = 1} },
	{ MODLKEY|ShiftMask,            XK_space,       zoomvar,                {.i = 0} },
	{ SUPKEY,                       XK_space,       view,                   {0} },
	{ SUPKEY|ShiftMask,             XK_space,       tagandview,             {0} },
	{ MODLKEY,                      XK_f,           togglefocusfloat,       {0} },
	{ MODLKEY|ShiftMask,            XK_f,           togglefullscreen,       {0} },
	{ SUPKEY,                       XK_f,           togglefloating,         {.i = 1} },
	{ SUPKEY|ShiftMask,             XK_f,           togglefloating,         {.i = 0} },
	{ MODLKEY,                      XK_Escape,      killclient,             {0} },
	{ MODLKEY,                      XK_e,           setltorprev,            {.v = &layouts[0]} },
	{ MODLKEY|ShiftMask,            XK_e,           setltorprev,            {.v = &layouts[1]} },
	{ MODLKEY|ControlMask,          XK_e,           setltorprev,            {.v = &layouts[4]} },
	{ MODLKEY,                      XK_w,           setltorprev,            {.v = &layouts[2]} },
	{ MODLKEY|ShiftMask,            XK_w,           setltorprev,            {.v = &layouts[3]} },
	{ MODLKEY|ControlMask,          XK_w,           setltorprev,            {.v = &layouts[5]} },
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
	{ MODLKEY,                      XK_q,           windowswitcher,         {.i = +1} },
	{ MODLKEY|ControlMask,          XK_q,           windowswitcher,         {.i = -1} },
	{ SUPKEY,                       XK_q,           windowswitcher,         {.i = +2} },
	{ SUPKEY|ShiftMask,             XK_q,           windowswitcher,         {.i = -3} },
	{ SUPKEY|ControlMask,           XK_q,           windowswitcher,         {.i = -2} },
	{ MODLKEY,                      XK_comma,       shiftview,              {.i = -1 } },
	{ MODLKEY,                      XK_period,      shiftview,              {.i = +1 } },
	{ MODLKEY|ShiftMask,            XK_comma,       shifttag,               {.i = -1 } },
	{ MODLKEY|ShiftMask,            XK_period,      shifttag,               {.i = +1 } },
	{ MODLKEY|ControlMask,          XK_period,      hideclient,             {0} },
	{ SUPKEY,                       XK_F1,          scratchtoggle,          {.i = 1} },
	{ MODRKEY,                      XK_m,           scratchtoggle,          {.i = 2} },
	{ SUPKEY,                       XK_p,           scratchtoggle,          {.i = 3} },
	{ SUPKEY,                       XK_c,           scratchtoggle,          {.i = 4} },
	{ SUPKEY,                       XK_e,           scratchtoggle,          {.i = 5} },
	{ SUPKEY,                       XK_w,           scratchtoggle,          {.i = 6} },
	{ SUPKEY,                       XK_o,           scratchtoggle,          {.i = 7} },
	{ SUPKEY,                       XK_y,           scratchtoggle,          {.i = 8} },
	{ SUPKEY,                       XK_u,           scratchtoggle,          {.i = 9} },
	{ SUPKEY,                       XK_a,           dynscratchtoggle,       {.i = DYNSCRATCHKEY(1) } },
	{ SUPKEY|ShiftMask,             XK_a,           dynscratchunmark,       {.i = DYNSCRATCHKEY(1) } },
	{ SUPKEY,                       XK_s,           dynscratchtoggle,       {.i = DYNSCRATCHKEY(2) } },
	{ SUPKEY|ShiftMask,             XK_s,           dynscratchunmark,       {.i = DYNSCRATCHKEY(2) } },
	{ SUPKEY,                       XK_d,           dynscratchtoggle,       {.i = DYNSCRATCHKEY(3) } },
	{ SUPKEY|ShiftMask,             XK_d,           dynscratchunmark,       {.i = DYNSCRATCHKEY(3) } },
	{ SUPKEY|ControlMask,           XK_a,           spawn,                  SCRIPTCMD("dynscript.sh", "1") },
	{ SUPKEY|ControlMask,           XK_s,           spawn,                  SCRIPTCMD("dynscript.sh", "2") },
	{ SUPKEY|ControlMask,           XK_d,           spawn,                  SCRIPTCMD("dynscript.sh", "3") },
	{ MODLKEY,                      XK_s,           togglefocusarea,        {0} },
	{ MODRKEY,                      XK_space,       togglewin,              {.v = &browser} },
	{ SUPKEY,                       XK_m,           togglewin,              {.v = &mail} },
	{ ControlMask,                  XK_Escape,      spawn,                  CMD("dunstctl", "close") },
	{ ControlMask|ShiftMask,        XK_Escape,      spawn,                  CMD("dunstctl", "close-all") },
	{ ControlMask,                  XK_grave,       spawn,                  CMD("dunstctl", "history-pop") },
	{ ControlMask|ShiftMask,        XK_grave,       spawn,                  CMD("dunstctl", "context") },
	{ 0,                            XK_Print,       spawn,                  SCRIPTCMD("screenshot.sh", "0") },
	{ ShiftMask,                    XK_Print,       spawn,                  SCRIPTCMD("screenshot.sh", "1") },
	{ MODLKEY,                      XK_c,           spawn,                  SCRIPTCMD("color_under_cursor.sh") },
	{ MODLKEY,                      XK_F7,          spawn,                  DISABLEDEMODE },
	{ MODLKEY,                      XK_F8,          spawn,                  ENABLEDEMODE },
	{ MODLKEY,                      XK_semicolon,   spawn,                  SCRIPTCMD("dictionary.sh", "selection") },
	{ MODLKEY|ShiftMask,            XK_semicolon,   spawn,                  SCRIPTCMD("dictionary.sh") },
	{ MODLKEY|ControlMask,          XK_semicolon,   spawn,                  SCRIPTCMD("dictionary_last.sh") },
	{ SUPKEY,                       XK_semicolon,   spawn,                  SCRIPTCMD("espeak.sh", "selection") },
	{ SUPKEY|ShiftMask,             XK_semicolon,   spawn,                  SCRIPTCMD("espeak.sh") },
	{ SUPKEY|ControlMask,           XK_semicolon,   spawn,                  SCRIPTCMD("espeak_last.sh") },
	{ MODLKEY|ShiftMask,            XK_l,           spawn,                  SHCMD("systemctl start lock.service; screen off") },
	{ MODLKEY|ShiftMask,            XK_q,           spawn,                  SCRIPTCMD("quit.sh") },
	{ MODLKEY|ControlMask,          XK_h,           spawn,                  SCRIPTCMD("hotspot_launch.sh") },
	{ MODLKEY|ControlMask,          XK_l,           spawn,                  CMD("systemctl", "restart", "iiserlogin.service") },
	{ MODLKEY|ControlMask,          XK_m,           spawn,                  SCRIPTCMD("toggletouchpad.sh") },
	{ SUPKEY|ControlMask,           XK_m,           spawn,                  SCRIPTCMD("togglekeynav.sh") },
	{ MODLKEY|ControlMask,          XK_r,           spawn,                  SCRIPTCMD("reflector_launch.sh") },
	{ MODLKEY|ControlMask,          XK_v,           spawn,                  TOGGLEVPN },
	{ MODLKEY,                      XK_F10,         spawn,                  SCRIPTCMD("pomodoro.sh") },
	{ MODLKEY|ShiftMask,            XK_F10,         spawn,                  SCRIPTCMD("pomodoro.sh", "status") },
	{ MODLKEY|ControlMask,          XK_F10,         spawn,                  SCRIPTCMD("pomodoro.sh", "stop") },
	{ SUPKEY,                       XK_b,           spawn,                  SCRIPTCMD("gbtns.sh") },
	{ SUPKEY|ShiftMask,             XK_p,           spawn,                  TERMCMD("pyfzf") },
	{ SUPKEY,                       XK_r,           spawn,                  TERMCMD("ranger", "--cmd=set show_hidden=false") },
	{ SUPKEY|ShiftMask,             XK_r,           spawn,                  TERMCMD("ranger") },
	{ SUPKEY|ControlMask,           XK_r,           spawn,                  TERMCMD("ranger", "--cmd=set show_hidden=false", "--cmd=set sort=ctime") },
	{ SUPKEY,                       XK_t,           spawn,                  TERMCMD("htop") },
	{ SUPKEY|ShiftMask,             XK_t,           spawn,                  TERMCMD("htop", "-s", "PERCENT_CPU") },
	{ SUPKEY|ControlMask,           XK_t,           spawn,                  TERMCMD("htop", "-s", "PERCENT_MEM") },
	{ SUPKEY|ShiftMask,             XK_m,           spawn,                  SCRIPTCMD("neomutt.sh") },
	{ MODLKEY|ControlMask,          XK_s,           spawn,                  CMD("sigdsblocks", "3") },
	{ MODRKEY,                      XK_s,           spawn,                  INHIBITSUSPEND1 },
	{ MODRKEY|ShiftMask,            XK_s,           spawn,                  INHIBITSUSPEND0 },
	{ MODRKEY,                      XK_semicolon,   spawn,                  SCRIPTCMD("ytmsclu.sh") },
	{ MODLKEY,                      XK_Delete,      spawn,                  SCRIPTCMD("usbmount.sh") },
	{ MODLKEY|ShiftMask,            XK_Delete,      spawn,                  SCRIPTCMD("mtpmount.sh") },
	{ MODLKEY|ControlMask,          XK_Delete,      spawn,                  SCRIPTCMD("android-usbmode.sh") },
	{ MODLKEY,                      XK_y,           spawn,                  SHCMD("echo 'run /home/ashish/.scripts/ytmsclu-local.sh ${path}' | socat - /tmp/music-mpv.socket") },
	{ MODLKEY,                      XK_F9,          spawn,                  SHCMD("echo 'seek 0 absolute-percent' | socat - /tmp/music-mpv.socket") },

	{ MODLKEY,           XK_bracketleft,            hideshowfloating,       {.i = 1} },
	{ MODLKEY,           XK_bracketright,           hideshowfloating,       {.i = 0} },
	{ MODLKEY,           XK_backslash,              scratchhidevisible,     {0} },
	{ MODRKEY,           XK_bracketleft,            hideshowfloating,       {.i = 1} },
	{ MODRKEY,           XK_bracketright,           hideshowfloating,       {.i = 0} },
	{ MODRKEY,           XK_backslash,              scratchhidevisible,     {0} },
	{ 0,                 XF86XK_AudioMute,          spawn,                  VOLUMEM },
	{ 0,                 XF86XK_AudioLowerVolume,   spawn,                  SCRIPTCMD("doubleprev.sh") },
	{ 0,                 XF86XK_AudioRaiseVolume,   spawn,                  SCRIPTCMD("doublenext.sh") },
	{ ShiftMask,         XK_F7,                     spawn,                  VOLUMEL },
	{ ShiftMask,         XK_F8,                     spawn,                  VOLUMER },
	{ ControlMask,       XK_F7,                     spawn,                  VOLUMEl },
	{ ControlMask,       XK_F8,                     spawn,                  VOLUMEr },
	{ 0,                 XF86XK_AudioPlay,          spawn,                  SCRIPTCMD("doubleclick.sh") },
	{ 0,                 XF86XK_AudioPrev,          spawn,                  PLAYERP },
	{ 0,                 XF86XK_AudioNext,          spawn,                  PLAYERN },
	{ 0,                 XF86XK_MonBrightnessDown,  spawn,                  SCRIPTCMD("btnsfn.sh", "-15") },
	{ 0,                 XF86XK_MonBrightnessUp,    spawn,                  SCRIPTCMD("btnsfn.sh", "+15") },
	{ ShiftMask,         XK_F2,                     spawn,                  SCRIPTCMD("btnsfn.sh",  "-5") },
	{ ShiftMask,         XK_F3,                     spawn,                  SCRIPTCMD("btnsfn.sh",  "+5") },
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

	{ SUPKEY|MODRKEY,               XK_1,           focustiled,             {.i = +1}},
	{ SUPKEY|MODRKEY,               XK_2,           focustiled,             {.i = +2}},
	{ SUPKEY|MODRKEY,               XK_3,           focustiled,             {.i = +3}},
	{ SUPKEY|MODRKEY,               XK_4,           focustiled,             {.i = +4}},
	{ SUPKEY|MODRKEY,               XK_5,           focustiled,             {.i = +5}},
	{ SUPKEY|MODRKEY,               XK_6,           focustiled,             {.i = +6}},
	{ SUPKEY|MODRKEY,               XK_7,           focustiled,             {.i = +7}},
	{ SUPKEY|MODRKEY,               XK_8,           focustiled,             {.i = +8}},
	{ SUPKEY|MODRKEY,               XK_9,           focustiled,             {.i = +9}},
	{ SUPKEY|MODRKEY,               XK_0,           focustiled,             {.i = +10}},
	{ SUPKEY|MODRKEY|ShiftMask,     XK_1,           focustiled,             {.i = -1} },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_2,           focustiled,             {.i = -2} },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_3,           focustiled,             {.i = -3} },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_4,           focustiled,             {.i = -4} },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_5,           focustiled,             {.i = -5} },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_6,           focustiled,             {.i = -6} },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_7,           focustiled,             {.i = -7} },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_8,           focustiled,             {.i = -8} },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_9,           focustiled,             {.i = -9} },
	{ SUPKEY|MODRKEY|ShiftMask,     XK_0,           focustiled,             {.i = -10} },
	{ MODLKEY|MODRKEY,              XK_grave,       vieworprev,             {.ui = ~0 } },
	{ MODLKEY|ShiftMask,            XK_grave,       tag,                    {.ui = ~0 } },
	TAGKEYS(                        XK_1,                                   0)
	TAGKEYS(                        XK_2,                                   1)
	TAGKEYS(                        XK_3,                                   2)
	TAGKEYS(                        XK_4,                                   3)
	TAGKEYS(                        XK_5,                                   4)
	TAGKEYS(                        XK_6,                                   5)
	TAGKEYS(                        XK_7,                                   6)
	TAGKEYS(                        XK_8,                                   7)
	TAGKEYS(                        XK_9,                                   8)
	TAGKEYS(                        XK_0,                                   9)
};

/* click can be ClkTabBar, ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
/* custom addition: ClkLast to match anything! */
static const Button buttons[] = {
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
	{ ClkLast,              0,                   91,        spawn,          SCRIPTCMD("doublebtn1.sh") },
	{ ClkLast,              0,                   92,        spawn,          SCRIPTCMD("doublebtn2.sh") },
	{ ClkLast,              0,                   93,        spawn,          SCRIPTCMD("doublebtn3.sh") },
};

#define FSIGID                          "z:"
#define FSIGIDLEN                       (sizeof FSIGID - 1)
/* set the following lengths without considering the terminating null byte */
#define MAXFSIGNAMELEN                  4
#define MAXFTYPELEN                     2

/* trigger signals using `xsetroot -name "FSIGID<signame> [<type> <value>]"` */
/* signal definitions */
static Signal signals[] = {
	/* signame              function */
	{ "fclg",               focuslast },
	{ "fclv",               focuslastvisible },
	{ "fcsk",               focusstackalt },
	{ "quit",               quit },
	{ "scrh",               scratchhide },
	{ "scrs",               scratchshow },
	{ "scrt",               scratchtoggle },
	{ "sfvw",               shiftview },
	{ "sftg",               shifttag },
	{ "view",               view },
	{ "wlnc",               windowlineupc },
	{ "wlnr",               windowlineupr },
	{ "wlns",               windowlineups },
};

/* custom function implementations */
void
dynscratchtoggle(const Arg *arg)
{
        if (selmon->sel && selmon->sel->scratchkey == arg->i) {
                if (selmon->sel->isfloating)
                        scratchhidehelper();
                else
                        focuslast(&((Arg){0}));
        } else if (!scratchshowhelper(arg->i)) {
                if (selmon->sel->scratchkey == 0) {
                        selmon->sel->scratchkey = arg->i;
                        spawn(&((Arg)NOTIFYDYNSCRATCH1));
                } else
                        spawn(&((Arg)NOTIFYDYNSCRATCH2));
        }
}

void
dynscratchunmark(const Arg *arg)
{
        if (selmon->sel && selmon->sel->scratchkey == arg->i) {
                selmon->sel->scratchkey = 0;
                spawn(&((Arg)NOTIFYDYNSCRATCH0));
        }
}

void
floatmoveresize(const Arg *arg)
{
        if (!selmon->sel || (!selmon->sel->isfloating && selmon->lt[selmon->sellt]->arrange))
                return;
        if (selmon->sel->isfullscreen)
                return;
        switch (((int *)arg->v)[0]) {
                case MoveX:
                {
                        int nx = selmon->sel->x + ((int *)arg->v)[1];
                        int cw = WIDTH(selmon->sel);
                        int mw = selmon->wx + selmon->ww;

                        /* snap to monitor edge on first try of crossover */
                        if (nx < selmon->wx && selmon->sel->x > selmon->wx)
                                nx = selmon->wx;
                        else if (nx + cw > mw && selmon->sel->x + cw < mw)
                                nx = mw - cw;
                        resize(selmon->sel, nx, selmon->sel->y, selmon->sel->w, selmon->sel->h, 1);
                }
                        break;
                case MoveY:
                {
                        int ny = selmon->sel->y + ((int *)arg->v)[1];
                        int ch = HEIGHT(selmon->sel);
                        int mh = selmon->wy + selmon->wh;

                        /* snap to monitor edge on first try of crossover */
                        if (ny < selmon->wy && selmon->sel->y > selmon->wy)
                                ny = selmon->wy;
                        else if (ny + ch > mh && selmon->sel->y + ch < mh)
                                ny = mh - ch;
                        resize(selmon->sel, selmon->sel->x, ny, selmon->sel->w, selmon->sel->h, 1);
                }
                        break;
                case ResizeX:
                {
                        int nw = selmon->sel->w + ((int *)arg->v)[1];
                        int cx = selmon->sel->x + 2 * selmon->sel->bw;
                        int mw = selmon->wx + selmon->ww;

                        /* snap to monitor edge on first try of crossover */
                        if (cx + nw > mw && cx + selmon->sel->w < mw)
                                nw = mw - cx;
                        resize(selmon->sel, selmon->sel->x, selmon->sel->y, nw, selmon->sel->h, 1);
                }
                        break;
                case ResizeY:
                {
                        int nh = selmon->sel->h + ((int *)arg->v)[1];
                        int cy = selmon->sel->y + 2 * selmon->sel->bw;
                        int mh = selmon->wy + selmon->wh;

                        /* snap to monitor edge on first try of crossover */
                        if (cy + nh > mh && cy + selmon->sel->h < mh)
                                nh = mh - cy;
                        resize(selmon->sel, selmon->sel->x, selmon->sel->y, selmon->sel->w, nh, 1);
                }
                        break;
                case ResizeA:
                {
                        int nw = selmon->sel->w + ((int *)arg->v)[1];
                        int nh = (nw * selmon->sel->h) / selmon->sel->w;
                        int cx = selmon->sel->x + 2 * selmon->sel->bw;
                        int cy = selmon->sel->y + 2 * selmon->sel->bw;
                        int mw = selmon->wx + selmon->ww;
                        int mh = selmon->wy + selmon->wh;

                        /* snap to monitor edge on first try of crossover */
                        if (cx + nw > mw && cx + selmon->sel->w < mw)
                                nw = mw - cx;
                        if (cy + nh > mh && cy + selmon->sel->h < mh)
                                nh = mh - cy;
                        resize(selmon->sel, selmon->sel->x, selmon->sel->y, nw, nh, 1);
                }
                        break;
        }
}

void
focusmaster(const Arg *arg)
{
        Client *c;

	if (selmon->nmaster < 1)
		return;
        if ((c = nexttiled(selmon->clients)))
                focusalt(c, 0);
}

void
focusstackalt(const Arg *arg)
{
	Client *c;

	if (!selmon->sel)
		return;
        if (!selmon->sel->isfloating &&
            (selmon->lt[selmon->sellt]->arrange == deckhor ||
             selmon->lt[selmon->sellt]->arrange == deckver) &&
            selmon->ntiles > selmon->nmaster + 1) {
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
	if (c)
		focusalt(c, 0);
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
        updateclientdesktop(selmon->sel, 0);
        focus(NULL);
        arrange(selmon);
}

void
hidefloating(const Arg *arg)
{
        for (Client *c = selmon->clients; c; c = c->next)
                if (c->isfloating && ISVISIBLE(c) && !c->isfullscreen) {
                        c->ishidden = 1;
                        updateclientdesktop(c, 0);
                }
        focus(NULL);
        arrange(selmon);
}

void
hideshowfloating(const Arg *arg)
{
        if (arg->i) {
                for (Client *c = selmon->stack; c; c = c->snext)
                        if (c->isfloating && ISVISIBLE(c) && !c->isfullscreen) {
                                hidefloating(&((Arg){0}));
                                return;
                        }
                showfloating(&((Arg){0}));
        } else {
                for (Client *c = selmon->stack; c; c = c->snext)
                        if (c->ishidden && c->tags & selmon->tagset[selmon->seltags]) {
                                showfloating(&((Arg){0}));
                                return;
                        }
                hidefloating(&((Arg){0}));
        }
}

/* the following two functions assume non-NULL selmon->sel */
Client *
nextsamefloat(int next)
{
        _Bool f = selmon->sel->isfloating;
        Client *c;

        if (next > 0) {
                for (c = selmon->sel->next;
                     c && ((_Bool)c->isfloating != f || !ISVISIBLE(c));
                     c = c->next);
                if (!c)
                        for (c = selmon->clients;
                             c && ((_Bool)c->isfloating != f || !ISVISIBLE(c));
                             c = c->next);
        } else {
                Client *i;

                c = NULL;
                for (i = selmon->clients; i != selmon->sel; i = i->next)
                        if ((_Bool)i->isfloating == f && ISVISIBLE(i))
                                c = i;
                if (!c)
                        for (; i; i = i->next)
                                if ((_Bool)i->isfloating == f && ISVISIBLE(i))
                                        c = i;
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

        if (!selmon->sel)
                return;
        c = selmon->lt[selmon->sellt]->arrange ?
                nextsamefloat(arg->i) : nextvisible(arg->i);
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
                if (c->scratchkey > 0 && ISVISIBLE(c)) {
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
                        updateclientdesktop(c, 0);
                }
        if (f)
                focusalt(f, 1);
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
        focusalt(c, 0);
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
        if (c)
                focusalt(c, 0);
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
                setfullscreen(selmon->sel, 1);
}

void
vieworprev(const Arg *arg)
{
	view((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags] ? &((Arg){0}) : arg);
}

int
hasleasttag(Client *c, int tag)
{
        for (int i = 0; i < tag; i++)
                if (1 << i & c->tags)
                        return 0;
        return 1 << tag & c->tags;
}

void
windowlineupc(const Arg *arg)
{
        int t;
        Client *s = selmon->sel;

        if (!s) {
                for (s = selmon->clients; s && !ISVISIBLE(s); s = s->next);
                if (!s)
                        return;
        }
	XDeleteProperty(dpy, root, netatom[NetClientList]);
        for (t = 0; !(1 << t & s->tags); t++);
        for (Client *c = s; c; c = c->next)
                if (hasleasttag(c, t))
                        XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                                        PropModePrepend, (unsigned char *) &(c->win), 1);
        t = (t + 1) % LENGTH(tags);
        for (int i = 1; i < LENGTH(tags); i++, t = (t + 1) % LENGTH(tags))
                for (Client *c = selmon->clients; c; c = c->next)
                        if (hasleasttag(c, t))
                                XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                                                PropModePrepend, (unsigned char *) &(c->win), 1);
        for (Client *c = selmon->clients; c; c = c->next)
                if (!c->tags)
                        XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                                        PropModePrepend, (unsigned char *) &(c->win), 1);
	for (Monitor *m = mons; m; m = m->next) {
                if (m == selmon)
                        continue;
                for (Client *c = m->stack; c; c = c->snext)
                        XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                                        PropModePrepend, (unsigned char *) &(c->win), 1);
        }
        for (Client *c = selmon->clients; c && c != s; c = c->next)
                if (hasleasttag(c, t))
                        XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                                        PropModePrepend, (unsigned char *) &(c->win), 1);
}

void
windowlineupr(const Arg *arg)
{
	XDeleteProperty(dpy, root, netatom[NetClientList]);
	for (Monitor *m = mons; m; m = m->next) {
                for (int t = 0; t < LENGTH(tags); t++)
                        for (Client *c = m->clients; c; c = c->next)
                                if (hasleasttag(c, t))
                                        XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                                                        PropModePrepend, (unsigned char *) &(c->win), 1);
                for (Client *c = m->clients; c; c = c->next)
                        if (!c->tags)
                                XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32,
                                                PropModePrepend, (unsigned char *) &(c->win), 1);
        }
}

void
windowlineups(const Arg *arg)
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
}

void
windowswitcher(const Arg *arg)
{
        switch (abs(arg->i)) {
                case 1:
                        windowlineups(&((Arg){0}));
                case 2:
                        windowlineupc(&((Arg){0}));
                case 3:
                        windowlineupr(&((Arg){0}));
        }
        if (arg->i > 0)
                spawn(&((Arg)ROFIWIN));
        else
                spawn(&((Arg)ROFIWINREGEX));
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
	focusalt(nmc, 1);
}

void
zoomvar(const Arg *arg)
{
        if (((selmon->lt[selmon->sellt]->arrange == deckhor ||
              selmon->lt[selmon->sellt]->arrange == deckver) &&
             selmon->ntiles > selmon->nmaster + 1) == (_Bool)arg->i)
                        zoomswap(&((Arg){0}));
        else
                        zoom(&((Arg){0}));
}

/* Window rules */
static void center(Client *c);
static void markscratch(Client *c, int key);

static void
applyrules(Client *c)
{
        char role[16] = "";
	const char *class, *instance;
	XClassHint ch = { NULL, NULL };

	XGetClassHint(dpy, c->win, &ch);
	class = ch.res_class ? ch.res_class : broken;
	instance = ch.res_name ? ch.res_name : broken;
        gettextprop(c->win, wmatom[WMWindowRole], role, sizeof role);

        if (strcmp(instance, "crx_cinhimbnkkaeohfgghhklpknlkffjgod") == 0) {
                markscratch(c, 2);
                c->isfloating = 1;
                c->bw = 0;
                c->w = 950;
                c->h = 626;
                center(c);
        } else if (strcmp(instance, "brave-browser") == 0) {
                c->scratchkey = browser.scratchkey;
        } else if (strcmp(instance, "calcurse-st") == 0) {
                markscratch(c, 4);
                c->isfloating = 1;
                c->w = 950;
                c->h = 650;
                center(c);
        } else if (strcmp(instance, "floating-st") == 0) {
                c->isfloating = 1;
                c->w = 750;
                c->h = 450;
                center(c);
        } else if (strcmp(instance, "music-st") == 0) {
                markscratch(c, 8);
                c->isfloating = 1;
                center(c);
        } else if (strcmp(instance, "neomutt-st") == 0) {
                markscratch(c, mail.scratchkey);
        } else if (strcmp(instance, "neovim-st") == 0) {
                markscratch(c, 9);
                c->isfloating = 1;
                c->w = 1060;
                c->h = 590;
                center(c);
        } else if (strcmp(instance, "pyfzf-st") == 0) {
                markscratch(c, 3);
                c->isfloating = 1;
                c->w = 1200;
                c->h = 600;
                center(c);
        } else if (strcmp(instance, "scratch-st") == 0) {
                markscratch(c, 1);
                c->isfloating = 1;
                c->w = 980;
                c->h = 570;
                center(c);
        } else if (strcmp(class, "scrcpy") == 0) {
                markscratch(c, 7);
                c->isfloating = 1;
                center(c);
        } else if (strcmp(class, "Signal") == 0) {
                markscratch(c, 5);
                c->isfloating = 1;
                c->w = 960;
                c->h = 620;
                center(c);
        } else if (strcmp(class, "TelegramDesktop") == 0) {
                markscratch(c, 6);
                c->isfloating = 1;
                c->w = 770;
                c->h = 555;
                center(c);
        } else if (strcmp(c->name, "Event Tester") == 0 ||
                   strcmp(class, "guvcview") == 0 ||
                   strcmp(class, "matplotlib") == 0 ||
                   strcmp(class, "RiseupVPN") == 0 ||
                   strcmp(class, "SimpleScreenRecorder") == 0 ||
                   strcmp(class, "Sxiv") == 0 ||
                   strcmp(class, "Woeusbgui") == 0 ||
                   strstr(class, "Yad")) {
                c->isfloating = 1;
                center(c);
        }
        if (strcmp(c->name, "Picture-in-Picture") == 0 ||
            strcmp(c->name, "Picture in picture") == 0 ||
            strcmp(role, "pop-up") == 0) {
                c->isfloating = 1;
                center(c);
        } else if (strcmp(role, "bubble") == 0) {
                c->isfloating = 1;
                c->bw = 0;
        }

        XFree(ch.res_class);
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
