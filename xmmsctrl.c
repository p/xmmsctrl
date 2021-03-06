/* -*- mode: C; c-basic-offset: 2; -*- */
/* xmmsctrl : simple small program
 * to control xmms from the command line
 * xmms provides some basic control command
 * but nothing to control the sound volume,
 * thus this program.
 * 
 * author: Alexandre David
 * e-mail: adavid@cs.aau.dk
 * Old web page: http://user.it.uu.se/~adavid/utils/
 * license: GPL
 *
 * See the Changelog file for the change log.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Publice License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <xmms/xmmsctrl.h>  /* provided by xmms-devel */

#ifdef PRETTY_PRINT
#define COLOR  "\033[1;34m"
#define NORMAL "\033[0;0m"
#else
#define COLOR  ""
#define NORMAL ""
#endif

/* returns true if xmms is playing a stream */
static gboolean is_stream_playing(gint);

/* simple commands, argument is the session number */
static void print_current_title(gint); /* print the title of the current playing song */ 
static void print_playlist(gint);      /* print all titles in the play list */
static void print_playfiles(gint);     /* print all files in the play list */
static void remove_from_playlist(gint);/* removes mp3 from playlist currently playing  */
static void print_length(gint session);/* print the length of the playlist */
static void print_current(gint);       /* print the current playing song */
static void print_help(gint);          /* print help   */
static void print_volume(gint);        /* print volume */
static void print_time(gint);          /* print played time */
static void print_current_pos(gint);   /* print current song position in the play list */
static void play_prev(gint);           /* play the previous track wrapping to the last from the first */
static void clear_playlist(gint);      /* clear the playlist */
void unique(gint);                     /* remove duplicates from the play list (in removefile.c) */

/* commands needing an argument, arguments are session number and the read string argument */
static void set_track(gint,char*);     /* set the current playing song */
static void set_vol(gint,char*);       /* set the volume */
static void set_time(gint,char*);      /* set time position */
static void set_dir(gint,char*);       /* set playing list to directory/file/device */
static void add_file(gint,char*);      /* add file/dir to playlist */
static void print(gint,char*);         /* general print */
static void def_eq(gint,char*);        /* add string if equal to current */
static void def_neq(gint,char*);       /* add string if not equal to current */
void remove_file(gint,char*);          /* remove file/dir from playlist (in removefile.c) */


/* type for conditional print */
typedef struct {
  char *eq;
  char *neq;
} CondString;

CondString *condStrings = NULL;
int nbCondStrings = 0;
int nbEq = 0;
int nbNEq = 0;


/* type for simple commands */
typedef struct {
  const char *name;               /* command line argument function */
  void (*command)(gint);          /* xmms API function to use       */
  const char *help;               /* help for this command          */
} Command;


/* simple command list */
Command com[]={
  {
    "clear",
    clear_playlist,
    "clear the play list."
  },
  {
    "cur" , 
    print_current ,
    "print the current mp3 song file."
  },
  {
    "eject" ,
    xmms_remote_eject ,
    "open xmms \"Load file(s)\" dialog window."
  },
  {
    "getlength" ,
    print_length ,
    "print the length of the play list."
  },
  {
    "getpos" , 
    print_current_pos ,
    "print the current mp3 song position in the play list."
  },
  {
    "gettime" ,
    print_time ,
    "print the current song's playback time in seconds."
  },
  {
    "getvol" ,
    print_volume ,
    "print the master volume value."
  },
  {
    "help",
    print_help ,
    "print this help message."
  },
  {
    "next" , 
    xmms_remote_playlist_next ,
    "xmms next song command, go to the next song."
  },
  {
    "pause" ,
    xmms_remote_pause ,
    "xmms pause command, pause the playing song."
  },
  {
    "play" ,
    xmms_remote_play ,
    "xmms play command, play the current song."
  },
  {
    "playlist" ,
    print_playlist ,
    "print the play list songs."
  },
  {
    "playfiles" ,
    print_playfiles ,
    "print the play list files."
  },
  {
    "pref" ,
    xmms_remote_show_prefs_box ,
    "open xmms preference window (as Ctrl-P)."
  },
  {
    "prev" ,
    xmms_remote_playlist_prev ,
    "xmms previous song command, go to the previous song."
  },
  {
    "previous" ,
    play_prev ,
    "go to the previous song and wrap round to last if position is first."
  },
  {
    "remove" ,
    remove_from_playlist ,
    "xmms removes mp3 currently playing from playlist."
  },
  {
    "repeat" , 
    xmms_remote_toggle_repeat ,
    "toggle xmms repeat flag."
  },
  {
    "shuffle" , 
    xmms_remote_toggle_shuffle ,
    "toggle xmms shuffle flag."
  },
  {
    "stop" ,
    xmms_remote_stop ,
    "xmms stop command, stop playing."
  },
  {
    "title" ,
    print_current_title ,
    "print the current mp3 song title."
  },
  {
    "uniq" ,
    unique,
    "remove duplicate files from the playlist."
  },
  {
    "quit", 
    xmms_remote_quit ,
    "terminate xmms."
  },
  {
    "--help",
    print_help ,
    "print this help message."
  }
};


/* type for test commands */
typedef struct {
  const char *name;
  gboolean (*test)(gint);
  const char *help;
} Test;


/* test command list */
Test test[]={
  {
    "paused" , 
    xmms_remote_is_paused ,
    "returns OK if xmms is paused."
  },
  {
    "playing" , 
    xmms_remote_is_playing ,
    "returns OK if xmms is playing a song."
  },
  {
    "is_equalizer" , 
    xmms_remote_is_eq_win ,
    "returns OK if xmms has its equalizer window open."
  },
  {
    "is_main" , 
    xmms_remote_is_main_win ,
    "returns OK if xmms has its main window open."
  },
  {
    "is_play_list" , 
    xmms_remote_is_pl_win ,
    "returns OK if xmms has its playing list window open."
  },
  {
    "is_stream" ,
    is_stream_playing ,
    "returns OK if xmms is playing a stream (http://somewhere)."
  },
  {
    "running" , 
    xmms_remote_is_running ,
    "returns OK if xmms is running."
  }
};


/* type for toggle commands */
typedef struct {
  const char *name;
  void (*command)(gint,gboolean);
  const char *help;
} ToggleCommand;


/* toggle command list */
ToggleCommand toggle[]={
  {
    "equalizer" ,
    xmms_remote_eq_win_toggle ,
    "hide/show xmms equalizer window."
  },
  {
    "main" , 
    xmms_remote_main_win_toggle ,
    "hide/show xmms main window."
  },
  {
    "play_list" , 
    xmms_remote_pl_win_toggle ,
    "hide/show xmms playing list window."
  }
};


/* type for commands wanting an argument */
typedef struct {
  const char *name;
  void (*command)(gint, char*);
  const char *help;
} ArgCommand;


/* one-argument-command list */
ArgCommand argcom[]={
  {
    "dir" ,
    set_dir ,
    COLOR"dir <name>"NORMAL" : clear the play list and load the directory/file\n"
    "              <name> as the play list. This works with devices\n"
    "              like /dev/cdrom to handle music CDs."
  },
  {
    "time" ,
    set_time ,
    COLOR"time [+|-|/]seconds"NORMAL" :\n"
    "\t seconds : set the playback time to seconds,\n"
    "\t+seconds : advance the playback by seconds,\n"
    "\t-seconds : rewind the playback by seconds,\n"
    "\t/seconds : set the playback time to seconds from the end\n"
    "\t           of the song.\n"
    "    Examples : xmmsctrl time 30, xmmsctrl time +10."
  },
  {
    "track" ,
    set_track ,
    COLOR"track <n>"NORMAL"  : set the n'th track in the play list as the current track.\n"
    COLOR" track last"NORMAL" : set the last track in the play list as the current track."
  },
  {
    "vol" ,
    set_vol ,
    COLOR"vol [+|-]percent"NORMAL", with the following effects\n"
    "\t percent : set the volume to percent,\n"
    "\t+percent : increase the volume with percent,\n"
    "\t-percent : decrease the volume with percent.\n"
    "    Examples : xmmsctrl vol 40, xmmsctrl vol +5, xmmsctrl vol -5."
  },
  {
    "print" ,
    print ,
    COLOR"print <string>"NORMAL" : general formatted print, similar to printf. The codes are:\n"
    "\t%n : new line,\n"
    "\t%t : tab,\n"
    "\t%F : current filename,\n"
    "\t%T : current title,\n"
    "\t%P : current position,\n"
    "\t%s : playback time in sec,\n"
    "\t%m : playback time in min:sec,\n"
    "\t%S : song length in sec,\n"
    "\t%M : song length in min:sec,\n"
    "\t%v : master volume,\n"
    "\t%l : playlist length,\n"
    "\t%p : playback time in percent,\n"
    "\t%(x): where x is a decimal number, evalutation string x depending if the current\n"
    "\tprinted song == (eq) or != (neq) the currently played song (see playlist2html.sh).\n"
    "\tIn addition, if the code is preceded by a number (say i), then the song i is\n"
    "\ttaken instead of the current one. If * is used then print evaluates on all the\n"
    "\tsongs. Examples: \"%*P: %*T (%*M)%n\" prints the whole playlist,\n"
    "\t\"%T (%m:%M)%n\" prints only the current song, and \"%3F%n\" prints the 3rd file."
  },
  {
    "eq",
    def_eq,
    COLOR"eq"NORMAL" : add evaluation string for the case treated song == current played song."
  },
  {
    "neq",
    def_neq,
    COLOR"neq"NORMAL" : add evaluation string for the case treated song != current played song."
  },
  {
    "+file" ,
    add_file ,
    COLOR"+file <file>"NORMAL" : add the file/directory to the playlist without clearing the playlist.\n"
    "\tYou can use 'xmmsctrl +file something uniq' or 'xmmsctrl +file BEGIN dir1 dir2 END uniq' for\n"
    "\ta cleaner result."
  },
  {
    "-file" ,
    remove_file ,
    COLOR"-file <file>"NORMAL" : remove the file/directory from the playlist. You can use\n"
    "\t'xmmsctrl uniq -file something' for a cleaner result."
  }
};


/* sizes of the lists */
#define NCOM (sizeof(com)/sizeof(Command))
#define NTST (sizeof(test)/sizeof(Test))
#define NTOG (sizeof(toggle)/sizeof(ToggleCommand))
#define NARG (sizeof(argcom)/sizeof(ArgCommand))


/*
 * Deprecated printing functions: only callbacks now.
 * Kept only for backward compatibility.
 */
static void print_length(gint session)       { print(session, "%l%n"); }
static void print_current(gint session)      { print(session, "%F%n"); }
static void print_volume(gint session)       { print(session, "%v%n"); }
static void print_time(gint session)         { print(session, "%s%n"); }
static void print_current_title(gint session){ print(session, "%T%n"); }
static void print_playlist(gint session)     { print(session, "%*P%t%*T%n"); }
static void print_playfiles(gint session)    { print(session, "%*P%t%*F%n"); }
static void print_current_pos(gint session)  { print(session, "%P%n"); }


/*
 * returns OK if xmms is playing a stream
 */
static gboolean is_stream_playing(gint session) {
  return !strncmp("http://",
		  xmms_remote_get_playlist_file(session,
						 xmms_remote_get_playlist_pos(session)),
		  7);
}


/*
 * go to previous song, wrap to the last song if the position
 * in the play list is the first. Note that in case of CD,
 * the first position is 1 otherwise it is 0.
 */
static void play_prev (gint session) {
  const gchar DEV[] = "/dev/";
  gint first = 0;

  if (!strncmp(DEV,
	       xmms_remote_get_playlist_file(session,
					     xmms_remote_get_playlist_pos(session)),
	       5)) {
    /*puts ("It appears you're playing a CD");*/
    first = 1;
  }

  if (first != xmms_remote_get_playlist_pos (session))
    xmms_remote_playlist_prev(session);
  else
    xmms_remote_set_playlist_pos(session, xmms_remote_get_playlist_length(session) - 1);
}


/*
 * wrap function to remove currently playing mp3 from playlist
 */
static void remove_from_playlist(gint session) {
  xmms_remote_playlist_delete(session,
                              xmms_remote_get_playlist_pos(session));
}


/*
 * track command: needs a track number or "last"
 */
static void set_track(gint session, char *arg) {
  int pos;

  if (!strcmp(arg, "last"))
    pos = xmms_remote_get_playlist_length(session);
  else
    pos = atoi(arg);

  if (pos > 0)
    xmms_remote_set_playlist_pos(session, pos-1);
  else
    fprintf(stderr, "Invalid play list position (%d), must be >= 1\n", pos);
}


/*
 * vol command: needs number | +number | -number
 */
static void set_vol(gint session, char *arg) {
  gint vol = xmms_remote_get_main_volume( session );

  switch( arg[0] ) {

  case '+': /* argument is positive */
  case '-': /* argument is negative */
    /* no test on the validity of the argument,
     * not critical: in the worst case 0 is returned */
    vol += atoi(arg);
    break;

  default:
    vol = atoi(arg);
  }

  /* check bounds */
  if (vol<0)
    vol = 0;
  else if (vol>100)
    vol = 100;
  
  xmms_remote_set_main_volume( session, vol );
}


/*
 * time command: needs number | +number | -number | /number
 */
static void set_time(gint session, char *arg) {
  gint ptime = xmms_remote_get_output_time( session );
  gint pos = xmms_remote_get_playlist_pos( session );
  gint length = xmms_remote_get_playlist_time( session, pos );
	
  switch( arg[0] ) {

  case '+': /* argument is positive */
  case '-': /* argument is negative */
    /* no test on the validity of the argument,
     * not critical: in the worst case 0 is returned */
    ptime += atoi(arg) * 1000;
    break;

  case '/':
    /* if arg[i] is a recognized string argument, by
     * definition it has at least one character, therefore
     * argv[i]+1 is at worst \0 and atoi returns in the
     * worst case 0 */
    ptime = length - atoi(arg+1) * 1000;
    break;

  default:
    ptime = atoi(arg) * 1000;
  }

  /* check bounds */
  if ( ptime < 0 )
    ptime = 0;
  else if ( ptime > length )
    ptime = length;
  
  xmms_remote_jump_to_time( session, ptime );
}


/*
 * +file command
 */
static void add_file(gint session, char *arg) {
  xmms_remote_playlist_add_url_string(session, arg);
}


/*
 * clear command
 */
static void clear_playlist(gint session) {
  xmms_remote_playlist_clear(session);
}


/*
 * dir command: needs a string as argument
 */
static void set_dir(gint session, char *arg) {
  xmms_remote_playlist_clear(session);
  xmms_remote_playlist_add_url_string(session, arg);
}


/*
 * ensure capacity of the string table
 */
static void ensure_capa(int i) {
  if (i >= nbCondStrings) {
    condStrings = (CondString*) realloc(condStrings, (i+10)*sizeof(CondString));
    memset(&condStrings[i], 0, (i+10-nbCondStrings)*sizeof(CondString));
    nbCondStrings = i+10;
  }
}


/*
 * add eq string
 */
static void def_eq(gint unused __attribute__((unused)), char *arg) {
  ensure_capa(nbEq);
  condStrings[nbEq++].eq = arg;
}

/*
 * add neq string
 */
static void def_neq(gint unused __attribute__((unused)), char *arg) {
  ensure_capa(nbNEq);
  condStrings[nbNEq++].neq = arg;
}


/*
 * general print command
 */
static void exec_print(gint session, char *arg, int depth) {
  gint start = xmms_remote_get_playlist_pos( session );
  gint end = start, i;
  gint current = start;
  char *s = arg;

  if (depth > 1000) {
    fprintf(stderr, "Possibly infinite recursive calls detected. Print aborted.\n");
    return;
  }

  /* run for all? */
  while (*s && !(s[0] == '%' && s[1] == '*')) s++;
  if (*s) {
    start = 0;
    end = xmms_remote_get_playlist_length(session) - 1;
  }

  for (i = start; i <= end ; ++i) {
    s = arg;
    while(*s) {

      if (*s == '%') {
	gint pos = current;
	char *info;
	gint x;

	if (!*++s) break;
	if (*s == '*') {
	  pos = i;
	  if (!*++s) break;
	}
	else if (*s >= '0' && *s <= '9') {
	  pos = *s - '0';
	  s++;
	  while(*s >= '0' && *s <= '9') {
	    pos = 10*pos + *s - '0';
	    s++;
	  }
	  if (--pos < 0) pos = 0;
	  if (!*s) break;
	}

	switch(*s) {
	case 'n': /* newline */
	  putchar('\n');
	  break;
	case 't': /* tab */
	  putchar('\t');
	  break;
	case 'T': /* title */
	  info = xmms_remote_get_playlist_title(session, pos);
	  if (info) {
	    printf("%s", info);
	    free(info);
	  }
	  break;
	case 'F': /* file */
	  info = xmms_remote_get_playlist_file(session, pos);
	  if (info) {
	    printf("%s", info);
	    free(info);
	  }
	  break;
	case 'P': /* pos */
	  printf("%d", pos+1);
	  break;
	case 's': /* current sec */
	  printf("%d", xmms_remote_get_output_time(session) / 1000);
	  break;
	case 'S': /* total sec */
	  printf("%d", xmms_remote_get_playlist_time(session, pos) / 1000);
	  break;
	case 'm': /* current min:sec */
	  x = xmms_remote_get_output_time(session) / 1000;
	  printf("%d:%-2.2d", x/60, x%60);
	  break;
	case 'M': /* total min:sec */
	  x = xmms_remote_get_playlist_time(session, pos) / 1000;
	  printf("%d:%-2.2d", x/60, x%60);
	  break;
	case 'v': /* volume */
	  printf("%d", xmms_remote_get_main_volume(session));
	  break;
	case 'l': /* playlist length */
	  printf("%d", xmms_remote_get_playlist_length(session));
	  break;
	case 'p': /* percent */
	  x = xmms_remote_get_playlist_time(session, pos);
	  if (x) {
	    printf("%d%%", (100*xmms_remote_get_output_time(session)) / x);
	  }
	  break;
	case '(':
	  s++;
	  x = 0;
	  while(*s && *s >= '0' && *s <= '9') {
	    x = x*10 + *s - '0';
	    s++;
	  }
	  x--;
	  if (i == current) {
	    if (x >= 0 && x < nbEq && condStrings[x].eq) exec_print(session, condStrings[x].eq, depth+1);
	  } else {
	    if (x >= 0 && x < nbNEq && condStrings[x].neq) exec_print(session, condStrings[x].neq, depth+1);
	  }
	  if (*s != ')') {
	    fprintf(stderr, "Missing ')' in expression.\n");
	    return;
	  }
	  break;
	default:
	  putchar(*s);
	}

      } else {
	putchar(*s);
      }
      s++;
    }
  }
}

static void print(gint session, char *arg) {
  exec_print(session, arg, 0);
}


/*
 * print xmmsctrl help
 * the dummy variable is used just for convenience
 */
static void print_help(__attribute__ ((unused)) gint dummy) {
  unsigned int i;

  /* The string is cut to conform to ISO C89 */
  puts(COLOR"XMMSCTRL version "VERSION NORMAL" (C) Alexandre David <adavid@cs.aau.dk>.\n"
       "'xmmsctrl' is a simple tool designed to be used at the shell level,\n"
       "typically in a small shell script associated to a keyboard shortcut. There\n"
       "are 4 different command types:\n"
       "- simple commands, e.g. \"xmmsctrl play\", which perform a simple task,\n"
       "- commands with a flag argument, e.g. \"xmmsctrl main 1\", which set\n"
       "  a particular state,");
  puts("- condition testing, e.g. \"xmmsctrl playing\", which can be used in\n"
       "  if statements in shells. Something to notice: this was designed to be\n"
       "  used simply, which is, directly in if statements: if <command>; then\n"
       "  <command>; else <command>; fi. There you put directly \"xmmsctrl playing\"\n"
       "  to test if xmms is playing. Notice how the if statement works: if the\n"
       "  command succeeds, it returns a 0, which means OK, otherwise it returns\n"
       "  an error code,\n"
       "- more specific commands with particular arguments.");

  /** simple commands, 2 special and the rest from the list */
  puts("\n"
       "The simple commands are:\n"
       " "COLOR"launch"NORMAL" : launch a xmms instance if none is running\n"
       " "COLOR"not"NORMAL" : negate the next condition test");
  for ( i = 0 ; i < NCOM ; i++ )
    printf(" "COLOR"%s"NORMAL" : %s\n", com[i].name, com[i].help);

  /** toggle commands from the list **/
  puts("\n"
       "The flag setting commands are used with 0 or 1:");
  for ( i = 0 ; i< NTOG ; i++ )
    printf(" "COLOR"%s"NORMAL" : %s\n", toggle[i].name, toggle[i].help);

  /** test commands from the list **/
  puts("\n"
       "The condition testing commands are:");
  for ( i = 0 ; i < NTST ; i++ )
    printf(" "COLOR"%s"NORMAL" : %s\n", test[i].name, test[i].help);

  /** argument commands, one special and the rest from the list **/
  puts("\n"
       "The other specific commands are:\n"
       " "COLOR"session number"NORMAL" : use the session number 'number', xmmsctrl looks\n"
       "                  automatically for the first working session.\n");
  for ( i = 0 ; i < NARG ; i++ )
    printf(" %s\n\n", argcom[i].help); /* special custom format here */

  /** batch mode */
  printf("Except for 'session', these command now support a\n"
	 "batch mode. You can give a list of arguments beginning\n"
	 "with BEGIN and ending with END, e.g.,\n"
	 " \txmmsctrl +file BEGIN dir1 dir2 song1 song2 END\n");

  /** examples **/
  puts("\n"
       "Examples of shell scripts to define simple functions:\n"
       " Play/Stop :\n"
       " \tsh -c \"if xmmsctrl playing;\\\n"
       " \t       then xmmsctrl stop;\\\n"
       " \t       else xmmsctrl play; fi\"\n"
       " Play/Pause :\n"
       " \tsh -c \"if xmmsctrl playing;\\\n"
       " \t       then xmmsctrl pause;\\\n"
       " \t       else xmmsctrl play; fi\"\n"
       "(with xmmsctrl in your path). See more examples in the bindings\n"
       "for twm in the distribution.\n"
       "Have fun. Alexandre");
}


/*
 * launch a new xmms and return the session number
 * exit if error
 */
static gint launch_xmms(void) {
  gint session;
  unsigned int tries;

  switch( fork() ) {

  case -1:
    perror("fork");
    exit(1);

  case 0:
    execlp("xmms", "xmms", NULL);
    fprintf(stderr, "xmms not found!\n");
    exit(0);

  default:
    for( tries = 0 ; tries < 10 ; tries++ ) {
      usleep( 500000 ); /* in usec */
      for( session = 0 ; session < 16 ; session++ )
	if ( xmms_remote_is_running( session ) )
	  return session;
    }
    exit(1); /* if no session found, abort */
  }
}


int main(int argc, char *argv[]) {
  int i = 1;
  unsigned int negate = 0;
  gint session;

  if ( argc == 1 ) {
    print_help(0);
    return 1;
  }

  /* try to find automatically xmms session. */
  for( session = 0 ; session < 16 ; session++ )
    if ( xmms_remote_is_running( session ) )
      break;

  if (session == 16) {  /* no session found     */
    if (strcmp( argv[1], "launch" )) {
      return 1;         /* error return = false */
    } else {
      i++;
      session = launch_xmms();
    }
  }

  for( ; i < argc ; i++ ) {

    /* special command tests first */
    /* use a given session number */
    if ( !strcmp( argv[i], "session" ) ) {

       /* if argument left */
      if ( ++i < argc )
        session = atoi( argv[i] );
      else /* no argument left */
        fprintf(stderr, "Command usage: session number\n");

    }
    /* negation handling */
    else if ( !strcmp( argv[i], "not" ) ) {
      negate ^= 1;
    /* handle generic commands if the command is not launch */
    } else if ( strcmp( argv[i], "launch" ) ) {
      unsigned int j;
      /* I don't need this, but this improves readability since
       * I avoid 3 nested if statements with it */
      int matched = 0;

      /* test functions */
      for( j = 0 ; j < NTST ; j++ )
        if ( !strcmp( argv[i], test[j].name ) )
          return (!test[j].test( session ))^negate;

      /* simple commands, we are here only if no test was found */
      for( j = 0 ; j < NCOM ; j++ )
        if ( !strcmp( argv[i], com[j].name ) ) {

          com[j].command( session );

	  matched = 1;
          break; /* command matched -> stop for loop */
        }

      if ( !matched )
	/* toggle commands */
        for( j = 0 ; j < NTOG ; j++ ) {
          if ( !strcmp( argv[i], toggle[j].name ) ) {

            if ( ++i < argc ) /* if argument left */
              toggle[j].command( session, atoi( argv[i] ) );
            else
              fprintf(stderr, "Command usage: %s 0|1\n",
                      toggle[j].name);

	    matched = 1;
            break; /* command matched -> stop for loop */
          }
	}

      if ( !matched )
	/* argument commands */
	for( j = 0 ; j < NARG ; j++ ) {
	  if ( !strcmp( argv[i], argcom[j].name ) ) {

	    if ( ++i < argc ) { /* if argument left */
	      if (!strcmp(argv[i], "BEGIN")) {
		/* enter batch mode */
		while (++i < argc && strcmp(argv[i], "END"))
		  argcom[j].command( session, argv[i] );
	      } else {
		argcom[j].command( session, argv[i] ); /* i < argc here implies argv[i] != NULL */
	      }
	    } else {
	      fprintf(stderr, "Command %s needs an argument. Usage:\n%s\n",
		      argcom[j].name, argcom[j].help);
	    }

	    matched = 1;
	    break;
	  }
	}
      
      if ( !matched )
	  fprintf(stderr, "Invalid command '%s' is ignored\n", argv[i] );
    }
  }
  
  if (condStrings) free(condStrings);

  return 0; /* OK result */
}
