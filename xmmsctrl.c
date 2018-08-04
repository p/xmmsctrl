/* xmmsctrl : simple small program
 * to control xmms from the command line
 * xmms provides some basic control command
 * but nothing to control the sound volume,
 * thus this program.
 * 
 * author: Alexandre David
 * e-mail: adavid@docs.uu.se
 * http://www.docs.uu.se/~adavid
 * license: GPL
 *
 * changelog :
 * 0.1: creation
 *      => release 1.0
 * 1.1 modification suggested by
 *      Marc Prud'hommeaux <marc@apocalypse.org>
 * to get current playing song.
 *      small cleaning
 * forgot to release :(
 * 1.2: added quit command
 *      => release 1.2
 * 1.3: time +/- seconds addition by
 *      Christian J. Robinson <infynity@onewest.net>
 * 1.4: /seconds option suggested by
 *      Volker Moell <volker@die-moells.de>
 *      help, --help, launch suggested by
 *      Jean Delvare <devare@ensicaen.ismra.fr>
 *      Help modified, code cleanup, launch+not added
 * 1.5: License fix, thanks to Janet Casey
 *      usleep Warning fix
 *      Makefile improvements by Jean Delvare
 *

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
#include <xmms/xmmsctrl.h>


typedef struct {
  const char *name;               /* command line argument function */
  void (*command)(gint);          /* xmms API function to use       */
  const char *help;               /* help for this command          */
} Command;


/* This declaration seems to be missing on my system */
void usleep(unsigned long usec);

static void print_current(gint);   /* print the current playing song */
static void print_help(gint);      /* print help                     */


Command com[]={             /* commands list                  */
  {
    "play" ,
    xmms_remote_play ,
    "xmms play command, play the current song"
  },
  {
    "pause" ,
    xmms_remote_pause ,
    "xmms pause command, pause the playing song"
  },
  {
    "stop" ,
    xmms_remote_stop ,
    "xmms stop command, stop playing"
  },
  {
    "pref" ,
    xmms_remote_show_prefs_box ,
    "open xmms preference window (as Ctrl-P)"
  },
  { "eject" ,
    xmms_remote_eject ,
    "open xmms \"Load file(s)\" dialog window"
  },
  { "prev" ,
    xmms_remote_playlist_prev ,
    "xmms previous song command, go to the previous song"
  },
  { "next" , 
    xmms_remote_playlist_next ,
    "xmms next song command, go to the next song"
  },
  {
    "repeat" , 
    xmms_remote_toggle_repeat ,
    "toggle xmms repeat flag"
  },
  {
    "shuffle" , 
    xmms_remote_toggle_shuffle ,
    "toggle xmms shuffle flag"
  },
  { "cur" , 
    print_current ,
    "print the current mp3 song file"
  },
  {
    "quit", 
    xmms_remote_quit ,
    "terminate xmms"
  },
  {
    "help",
    print_help ,
    "print this help message"
  },
  {
    "--help",
    print_help ,
    "print this help message"
  }
};


typedef struct {
  const char *name;
  gboolean (*test)(gint);
  const char *help;
} Test;

Test test[]={
  {
    "playing" , 
    xmms_remote_is_playing ,
    "returns OK if xmms is playing a song"
  },
  {
    "paused" , 
    xmms_remote_is_paused ,
    "returns OK if xmms is paused"
  },
  {
    "running" , 
    xmms_remote_is_running ,
    "returns OK if xmms is running"
  },
  {
    "is_main" , 
    xmms_remote_is_main_win ,
    "returns OK if xmms has its main window open"
  },
  {
    "is_play_list" , 
    xmms_remote_is_pl_win ,
    "returns OK if xmms has its playing list window open"
  },
  {
    "is_equalizer" , 
    xmms_remote_is_eq_win ,
    "returns OK if xmms has its equalizer window open"
  }
};


typedef struct {
  const char *name;
  void (*command)(gint,gboolean);
  const char *help;
} ToggleCommand;

ToggleCommand toggle[]={
  {
    "main" , 
    xmms_remote_main_win_toggle ,
    "hide/show xmms main window"
  },
  {
    "play_list" , 
    xmms_remote_pl_win_toggle ,
    "hide/show xmms playing list window"
  },
  {
    "equalizer" ,
    xmms_remote_eq_win_toggle ,
    "hide/show xmms equalizer window"
  }
};

#define NCOM sizeof(com)/sizeof(Command)
#define NTST sizeof(test)/sizeof(Test)
#define NTOG sizeof(toggle)/sizeof(ToggleCommand)


/*
 * wrap function to print the current played mp3 file
 */
static void print_current(gint session) {
  printf("%s\n",
         xmms_remote_get_playlist_file(session,
                                       xmms_remote_get_playlist_pos(session)));
}


/*
 * print xmmsctrl help
 * the dummy variable is used just for convenience
 */
static void print_help(__attribute__ ((unused)) gint dummy) {
  unsigned int i;

  printf("XMMSCTRL version "VERSION", main author Alexandre David <adavid@docs.uu.se>:\n"
	 "xmmsctrl is a simple tool designed to be used at the shell level,\n"
	 "typically in a small shell script associated to a shortkey. There are\n"
	 "4 ways to use xmmsctrl :\n"
	 "- simple commands, e.g. \"xmmsctrl play\", which perform a simple task\n"
	 "- commands with a flag argument, e.g. \"xmmsctrl main 1\", which set\n"
	 "  a particular state\n"
	 "- condition testing, e.g. \"xmmsctrl playing\", which can be used in\n"
	 "  if statements in shells. Something to notice: this was designed to be\n"
	 "  used simply, which is, directly in if statements: if <command>; then\n"
         "  <command>; else <command>; fi. There you put directly \"xmmsctrl playing\"\n"
	 "  to test if xmms is playing. Notice how the if statement works: if the\n"
	 "  command succeeds, it returns a 0, which means OK, otherwise it returns\n"
	 "  an error code.\n"
	 "- more specific commands with particular arguments\n"
	 "\n"
	 "The simple commands are\n"
	 " launch : launch a xmms instance if none is running\n"
	 " not : negate the next condition test\n");
  for ( i = 0 ; i < NCOM ; i++ )
    printf(" %s : %s\n", com[i].name, com[i].help);

  printf("\n"
	 "The flag setting commands are used with 0 or 1\n");
  for ( i = 0 ; i< NTOG ; i++ )
    printf(" %s : %s\n", toggle[i].name, toggle[i].help);

  printf("\n"
	 "The condition testing commands are\n");
  for ( i = 0 ; i < NTST ; i++ )
    printf(" %s : %s\n", test[i].name, test[i].help);

  printf("\n"
	 "The other specific commands are\n"
	 " vol [+|-]percent, with the following effects\n"
	 " \t percent : set the volume to percent\n"
	 " \t+percent : increase the volume with percent\n"
	 " \t-percent : decrease the volume with percent\n"
	 " \texamples : xmmsctrl vol 40, xmmsctrl vol +5, xmmsctrl vol -5\n"
	 " time [+|-|/]seconds, with the following effects\n"
	 " \t seconds : set the playback time to seconds\n"
	 " \t+seconds : advance the playback by seconds\n"
	 " \t-seconds : rewind the playback by seconds\n"
	 " \t/seconds : set the playback time to seconds from the end\n"
	 " \t           of the song\n"
	 " \texamples : xmmsctrl time 30, xmmsctrl time +10\n"
	 " session number : use the session number 'number', default\n"
	 "                  is the first valid one\n"
	 "\n"
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
	 "Have fun. Alexandre\n");
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


int main( int argc, char *argv[] ) {
  unsigned int i = 1;
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
    if (strcmp( argv[1], "launch" ))
      return 1;         /* error return = false */
    else {
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
    /* else volume setting */
    else if ( !strcmp( argv[i], "vol" ) ) {

      /* if argument left */
      if ( ++i < argc ) {
        gint vol = xmms_remote_get_main_volume( session );

        switch( argv[i][0] ) {

        case '+': /* argument is positive */
        case '-': /* argument is negative */
	  /* no test on the validity of the argument,
	   * not critical: in the worst case 0 is returned */
	  vol += atoi( argv[i] );
	  break;

        default:
	  vol = atoi(argv[i]);
        }

	/* check bounds */
        if (vol<0)
	  vol = 0;
        else if (vol>100)
	  vol = 100;

        xmms_remote_set_main_volume( session, vol );

      } else /* no argument left */
        fprintf(stderr,"Command usage: vol [+|-]percent\n");

    } /* else time position setting */
    else if ( !strcmp( argv[i], "time" ) ) {

      /* if argument left */
      if ( ++i <argc ) {
        gint ptime = xmms_remote_get_output_time( session );
	gint pos = xmms_remote_get_playlist_pos( session );
        gint length = xmms_remote_get_playlist_time( session, pos );
	
        switch( argv[i][0] ) {

        case '+': /* argument is positive */
        case '-': /* argument is negative */
	  /* no test on the validity of the argument,
	   * not critical: in the worst case 0 is returned */
	  ptime += atoi( argv[i] ) * 1000;
	  break;

	case '/':
	  /* if arg[i] is a recognized string argument, by
	   * definition it has at least one character, therefore
	   * argv[i]+1 is at worst \0 and atoi returns in the
	   * worst case 0 */
	  ptime = length - atoi( argv[i]+1 ) * 1000;
	  break;

        default:
	  ptime = atoi( argv[i] ) * 1000;
        }

	/* check bounds */
        if ( ptime < 0 )
	  ptime = 0;
	else if ( ptime > length )
	  ptime = length;

        xmms_remote_jump_to_time( session, ptime );

      } else /* no argument left */
        fprintf(stderr, "Command usage: time [+|-]seconds\n");

    } else if ( !strcmp( argv[i], "not" ) )
      negate ^= 1;
    else if ( strcmp( argv[i], "launch" ) ) {
      /* launch is valid but ignored at this point */
      /* generic commands */

      unsigned int j;

      /* test functions */
      for( j = 0 ; j < NTST ; j++ )
        if ( !strcmp( argv[i], test[j].name ) )
          return (!test[j].test( session ))^negate;

      /* simple commands, we are here only if no test was found */
      for( j = 0 ; j < NCOM ; j++ )
        if ( !strcmp( argv[i], com[j].name ) ) {
          com[j].command( session );
          break; /* command matched -> stop for loop */
        }

      if ( j == NCOM ) { /* if no simple command was found */
        for( j = 0 ; j < NTOG ; j++ )
          if ( !strcmp( argv[i], toggle[j].name ) ) {
            if ( ++i <argc ) /* if argument left */
              toggle[j].command( session, atoi( argv[i] ) );
            else /* no argument left */
              fprintf(stderr, "Command usage: %s 0|1\n",
                      toggle[j].name);
            break; /* command matched -> stop for loop */
          }

	if ( j == NTOG ) /* if no command with argument was found */
	  fprintf(stderr, "Invalid command '%s' is ignored\n", argv[i] );
      }
    }
  }
  
  return 0; /* OK result */
}
