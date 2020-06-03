/* modifier 0 means no modifier */
static int surfuseragent    = 1;  /* Append Surf version to default WebKit user agent */
static char *fulluseragent  = ""; /* Or override the whole user agent string */
static char *scriptfile     = "~/.local/surf/script.js";
static char *styledir       = "~/.local/surf/styles/";
static char *certdir        = "~/.local/surf/certificates/";
static char *cachedir       = "~/.local/surf/cache/";
static char *cookiefile     = "~/.local/surf/cookies.txt";

/* Webkit default features */
/* Highest priority value will be used.
 * Default parameters are priority 0
 * Per-uri parameters are priority 1
 * Command parameters are priority 2
 */
static Parameter defconfig[ParameterLast] = {
	/* parameter                    Arg value       priority */
	[AcceleratedCanvas]   =       { { .i = 1 },     },
	[AccessMicrophone]    =       { { .i = 0 },     },
	[AccessWebcam]        =       { { .i = 0 },     },
	[Certificate]         =       { { .i = 0 },     },
	[CaretBrowsing]       =       { { .i = 0 },     },
	[CookiePolicies]      =       { { .v = "@Aa" }, },
	[DefaultCharset]      =       { { .v = "UTF-8" }, },
	[DiskCache]           =       { { .i = 1 },     },
	[DNSPrefetch]         =       { { .i = 0 },     },
	[FileURLsCrossAccess] =       { { .i = 0 },     },
	[FontSize]            =       { { .i = 16 },    },
	[FrameFlattening]     =       { { .i = 0 },     },
	[Geolocation]         =       { { .i = 0 },     },
	[HideBackground]      =       { { .i = 0 },     },
	[Inspector]           =       { { .i = 0 },     },
	[Java]                =       { { .i = 1 },     },
	[JavaScript]          =       { { .i = 1 },     },
	[KioskMode]           =       { { .i = 0 },     },
	[LoadImages]          =       { { .i = 1 },     },
	[MediaManualPlay]     =       { { .i = 1 },     },
	[Plugins]             =       { { .i = 1 },     },
	[PreferredLanguages]  =       { { .v = (char *[]){ NULL } }, },
	[RunInFullscreen]     =       { { .i = 0 },     },
	[ScrollBars]          =       { { .i = 1 },     },
	[ShowIndicators]      =       { { .i = 1 },     },
	[SiteQuirks]          =       { { .i = 1 },     },
	[SmoothScrolling]     =       { { .i = 0 },     },
	[SpellChecking]       =       { { .i = 0 },     },
	[SpellLanguages]      =       { { .v = ((char *[]){ "en_US", NULL }) }, },
	[StrictTLS]           =       { { .i = 1 },     },
	[Style]               =       { { .i = 1 },     },
	[WebGL]               =       { { .i = 0 },     },
	[ZoomLevel]           =       { { .f = 1.0 },   },
};

static UriParameters uriparams[] = {
	{ "(://|\\.)suckless\\.org(/|$)", {
	  [JavaScript] = { { .i = 0 }, 1 },
	  [Plugins]    = { { .i = 0 }, 1 },
	}, },
};

/* default window size: width, height */
static int winsize[] = { 800, 600 };

static WebKitFindOptions findopts = WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE |
                                    WEBKIT_FIND_OPTIONS_WRAP_AROUND;

#define PROMPT_GO   "Go:"
#define PROMPT_FIND "Find:"

/* SETPROP(readprop, setprop, prompt)*/
#define SETPROP(r, s, p) { \
        .v = (const char *[]){ "/bin/sh", "-c", \
             "prop=\"$(printf '%b' \"$(xprop -id $1 $2 " \
             "| sed \"s/^$2(STRING) = //;s/^\\\"\\(.*\\)\\\"$/\\1/\")\" " \
             "| dmenu -p \"$4\" -w $1)\" && xprop -id $1 -f $3 8s -set $3 \"$prop\"", \
             "surf-setprop", winid, r, s, p, NULL \
        } \
}

/* DOWNLOAD(URI, referer) */
#define DOWNLOAD(u, r) { \
        .v = (const char *[]){ "st", "-e", "/bin/sh", "-c",\
             "curl -g -L -J -O -A \"$1\" -b \"$2\" -c \"$2\"" \
             " -e \"$3\" \"$4\"; read", \
             "surf-download", useragent, cookiefile, r, u, NULL \
        } \
}

/* PLUMB(URI) */
/* This called when some URI which does not begin with "about:",
 * "http://" or "https://" should be opened.
 */
#define PLUMB(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "xdg-open \"$0\"", u, NULL \
        } \
}

/* VIDEOPLAY(URI) */
#define VIDEOPLAY(u) {\
        .v = (const char *[]){ "/bin/sh", "-c", \
             "mpv --really-quiet \"$0\"", u, NULL \
        } \
}

/* styles */
/*
 * The iteration will stop at the first match, beginning at the beginning of
 * the list.
 */
static SiteSpecific styles[] = {
	/* regexp               file in $styledir */
	{ ".*",                 "default.css" },
};

/* certificates */
/*
 * Provide custom certificate for urls
 */
static SiteSpecific certs[] = {
	/* regexp               file in $certdir */
	{ "://suckless\\.org/", "suckless.org.crt" },
};

/* Search engines */

static const char * defaultsearchengine = "https://www.duckduckgo.com/?q=%s";
static SearchEngine searchengines[] = {
    { "dg",   "https://www.duckduckgo.com/?q=%s" },
    { "red", "https://reddit.com/r/%s" },
    { "gh", "https://github.com/search?q=%s" },
    { "wt", "http://en.wiktionary.org/?search=%s" },
    { "osm", "http://www.openstreetmap.org/search?query=%s" },
    { "tpb", "http://thepiratebay.org/search/%s" },
    { "laincat", "http://lainchan.org/%s/catalog.html" },
    { "eb", "https://ebay.com/sch/%s" },
    { "etym", "http://etymonline.com/index.php?allowed_in_frame=0&search=%s" },
    { "aw", "https://wiki.archlinux.org/index.php?title=Special%3ASearch&search=%s" },
    { "gw", "https://wiki.gentoo.org/index.php?title=Special%3ASearch&search=%s" },
    { "yt", "https://www.youtube.com/results?search_query=%s" },
    { "ig", "https://wiki.installgentoo.com/index.php?search=%s&title=Special%3ASearch" },
    { "w", "https://www.wikipedia.org/search-redirect.php?family=wikipedia&language=en&search=%s&language=en&go=Go" },
    { "sk", "https://www.skytorrents.in/search/all/ed/1/?l=en-us&q=%s" },
    { "vw", "https://wiki.voidlinux.eu/index.php?search=%s&title=Special%3ASearch" },
    { "thw", "http://www.thinkwiki.org/w/index.php?search=%s&title=Special%3ASearch" },
    { "vw", "http://vim.wikia.com/wiki/Special:Search?fulltext=Search&query=%s" },
    { "bc", "https://www.bitchute.com/search?q=%s&sort=date_created%20desc" },
    { "lg", "http://93.174.95.27/search.php?req=%s" },
    { "leo", "https://dict.leo.org/ende?search=%s" },
};

#define MODKEY GDK_CONTROL_MASK

/* hotkeys */
/*
 * If you use anything else but MODKEY and GDK_SHIFT_MASK, don't forget to
 * edit the CLEANMASK() macro.
 */
static Key keys[] = {
	/* modifier              keyval          function    arg */
	{ 0,                     GDK_KEY_g,      spawn,      SETPROP("_SURF_URI", "_SURF_GO", PROMPT_GO) },
	{ 0,                     GDK_KEY_f,      spawn,      SETPROP("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },
	{ 0,                     GDK_KEY_slash,  spawn,      SETPROP("_SURF_FIND", "_SURF_FIND", PROMPT_FIND) },

	{ 0,                     GDK_KEY_Escape, insert,     {.i = 0 } },
	{ 0,                     GDK_KEY_i,      insert,     {.i = 1 } },

	{ 0,                     GDK_KEY_c,      stop,       { 0 } },
	{ 0,                     GDK_KEY_r,      reload,     { .i = 0 } },

	{ 0,                     GDK_KEY_l,      navigate,   { .i = +1 } },
	{ 0,                     GDK_KEY_h,      navigate,   { .i = -1 } },

	/* vertical and horizontal scrolling, in viewport percentage */
	{ 0,                     GDK_KEY_j,      scrollv,    { .i = +10 } },
	{ 0,                     GDK_KEY_k,      scrollv,    { .i = -10 } },
	{ 0,                     GDK_KEY_space,  scrollv,    { .i = +50 } },
	{ 0,                     GDK_KEY_b,      scrollv,    { .i = -50 } },


	{ 0|GDK_SHIFT_MASK,      GDK_KEY_j,      zoom,       { .i = -1 } },
	{ 0|GDK_SHIFT_MASK,  	 GDK_KEY_k,      zoom,       { .i = +1 } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_q,      zoom,       { .i = 0  } },

	{ 0,                     GDK_KEY_p,      clipboard,  { .i = 1 } },
	{ 0,                     GDK_KEY_y,      clipboard,  { .i = 0 } },

	{ 0,                     GDK_KEY_n,      find,       { .i = +1 } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_n,      find,       { .i = -1 } },

	{ MODKEY|GDK_SHIFT_MASK, GDK_KEY_p,      print,      { 0 } },
	{ MODKEY,                GDK_KEY_t,      showcert,   { 0 } },

	{ 0|GDK_SHIFT_MASK,      GDK_KEY_a,      togglecookiepolicy, { 0 } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_i,      toggleinspector, { 0 } },

	{ 0|GDK_SHIFT_MASK,      GDK_KEY_c,      toggle,     { .i = CaretBrowsing } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_f,      toggle,     { .i = FrameFlattening } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_g,      toggle,     { .i = Geolocation } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_s,      toggle,     { .i = JavaScript } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_o,      toggle,     { .i = LoadImages } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_v,      toggle,     { .i = Plugins } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_b,      toggle,     { .i = ScrollBars } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_t,      toggle,     { .i = StrictTLS } },
	{ 0|GDK_SHIFT_MASK,      GDK_KEY_m,      toggle,     { .i = Style } },
};

/* button definitions */
/* target can be OnDoc, OnLink, OnImg, OnMedia, OnEdit, OnBar, OnSel, OnAny */
static Button buttons[] = {
	/* target       event mask      button  function        argument        stop event */
	{ OnLink,       0,              2,      clicknewwindow, { .i = 0 },     1 },
	{ OnLink,       MODKEY,         2,      clicknewwindow, { .i = 1 },     1 },
	{ OnLink,       MODKEY,         1,      clicknewwindow, { .i = 1 },     1 },
	{ OnAny,        0,              8,      clicknavigate,  { .i = -1 },    1 },
	{ OnAny,        0,              9,      clicknavigate,  { .i = +1 },    1 },
	{ OnMedia,      MODKEY,         1,      clickexternplayer, { 0 },       1 },
};

#define HOMEPAGE "https://duckduckgo.com/"
