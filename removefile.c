/* -*- mode: C; c-basic-offset: 2; -*- */
/* removefile.c: function for xmmsctrl
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
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xmms/xmmsctrl.h>  /* provided by xmms-devel */

/* This file implements: */
void remove_file(gint session, char *filename);
void unique(gint);

/* Not always prototyped */
int lstat(const char *file_name, struct stat *buf);

/* a song entry */
typedef struct {
  char *filename;
  gint index;
} song_t;


/* search for a song filename and add its index to pos if match found
 */
static void add_index(int root_len, const char *pwd, const char *filename,
		      const song_t *start_songs, gint nb_songs,
		      gint *pos, gint *nb_pos)
{
  /*printf("add_index(%s,%s)\n", pwd, filename);*/

  if (*nb_pos < nb_songs) {
    int len_path = strlen(pwd);
    int len_all = len_path + strlen(filename) + 2;

    if (len_all <= 16384) { /* suspect BAD things if more */

      int offset = root_len;
      char song_file[len_all];
      strcpy(song_file, pwd);
      if (len_path == 1 && *pwd == '/') len_path--;
      song_file[len_path] = '/';
      strcpy(song_file+len_path+1, filename);
    
      /* search song_file, relative + absolute path */
      do {
	gint nb = nb_songs;
	const song_t *start = start_songs;

	while(nb) {
	  gint middle = nb >> 1;
	  int cmp = strcmp(song_file+offset, start[middle].filename);
	  /*printf("strcmp(%s,%s)->%d\n",song_file+offset, start[middle].filename,cmp);*/

	  if (cmp < 0) {
	    nb = middle;
	  } else if (cmp > 0) {
	    nb -= middle+1;
	    start += middle+1;
	  } else { /* match */
	    /*printf("Found: %s\n", start[middle].filename);*/
	    pos[(*nb_pos)++] = start[middle].index;
	    return;
	  }
	}

	offset -= root_len;
      } while(offset >= 0);

      /* no match found */
      /*printf("NOT found!\n");*/
    }
  }
}

/* recursive traversal of directories */
static void remove_dirname(int root_len, const char *dirname,
			   const song_t *start_songs, gint nb_songs,
			   gint *pos, gint *nb_pos)
{
  if (chdir(dirname)) {
    perror(dirname);
  } else {
    DIR *dir = opendir(".");
    if (dir) {
      /* xmms uses absolute paths */
      char *pwd = get_current_dir_name();
      struct dirent *entry;
      struct stat file_stat;

      /*printf("remove_dirname(%s) @ %s\n", dirname, pwd);*/
      
      readdir(dir); /* . */
      readdir(dir); /* .. */
      while((entry = readdir(dir)) != NULL) {
	if (lstat(entry->d_name, &file_stat)) {
	  perror(entry->d_name);
	} else {
	  if (S_ISDIR(file_stat.st_mode)) {
	    remove_dirname(root_len, entry->d_name, start_songs, nb_songs, pos, nb_pos);
	  } else if (S_ISREG(file_stat.st_mode)) {
	    add_index(root_len, pwd, entry->d_name, start_songs, nb_songs, pos, nb_pos);
	  }
	}
      }

      free(pwd);
      closedir(dir);
    }
    chdir("..");
  }
}

/* to sort song filenames */
static int song_compare(const void *p1, const void *p2) {
  const song_t *s1 = (song_t*) p1;
  const song_t *s2 = (song_t*) p2;
  int cmp = strcmp(s1->filename, s2->filename);
  return cmp ? cmp : (s1->index < s2->index ? -1 : 1); /* never index == */
}

/* to sort indices */
static int index_compare(const void *p1, const void *p2) {
  const gint *i1 = (gint*) p1;
  const gint *i2 = (gint*) p2;
  return *i1 < *i2 ? -1 : (*i1 > *i2 ? 1 : 0);
}


void remove_file(gint session, char *filename) {
  struct stat file_stat;

  if (lstat(filename, &file_stat)) {
    perror(filename);
  } else {
    gint list_size;
    song_t *play_list;

    if (*filename == '/') { /* absolute path */
      char *s = filename;
      char *last = s;
      /* last occurence of '/' */
      while(*++s) if (*s == '/') last = s;
      *last = '\0';
      /* absolute to relative */
      if (chdir(filename)) {
	perror(filename);
	*last = '/';
	return;
      }
      *last = '/';
      filename = last+1;
    }

    /* read once the file list : malloc because
     * this can be arbitrarily large
     */
    list_size = xmms_remote_get_playlist_length(session);
    play_list = (song_t*) malloc(list_size*sizeof(song_t));

    if (play_list) {
      gint *remove_pos = (gint*) malloc(list_size*sizeof(gint));

      if (remove_pos) {
	char *pwd = get_current_dir_name();
	int root_len = strlen(pwd)+1; /* +1: jump appended / */
	gint i;

	for (i = 0; i < list_size; ++i) {
	  play_list[i].filename = xmms_remote_get_playlist_file(session, i);
	  play_list[i].index = i;
	  /*printf("%d: %s\n", i, play_list[i].filename);*/
	}

	/* sort to get search of filenames in O(log(list_size)) */
	qsort(play_list, list_size, sizeof(song_t), song_compare);

	i = 0;
	if (S_ISDIR(file_stat.st_mode)) {
	  remove_dirname(root_len, filename, play_list, list_size, remove_pos, &i);
	} else if (S_ISREG(file_stat.st_mode)) {
	  add_index(root_len, pwd, filename, play_list, list_size, remove_pos, &i);
	}

	if (i) {
	  /* remove from end, otherwise indices change */
	  qsort(remove_pos, i, sizeof(gint), index_compare);
	  do {
	    xmms_remote_playlist_delete(session, remove_pos[--i]);
	  } while(i);
	}

	free(remove_pos);
	for (i = 0; i < list_size; ++i)
	  free(play_list[i].filename);
	free(pwd);
      }
      free(play_list);
    }
  }
}

void unique(gint session) {
  gint list_size = xmms_remote_get_playlist_length(session);
  song_t *play_list = (song_t*) malloc(list_size*sizeof(song_t));
  if (play_list) {
    gint *remove_pos = (int*) malloc(list_size*sizeof(int));

    if (remove_pos) {
      gint i, n, current;
      for (i = 0; i < list_size; ++i) {
	play_list[i].filename = xmms_remote_get_playlist_file(session, i);
	play_list[i].index = i;
      }
      qsort(play_list, list_size, sizeof(song_t), song_compare);
      current = xmms_remote_get_playlist_pos(session);

      /* duplicates are consecutive since the list is sorted */
      n = 0;
      for (i = 0; i < list_size-1; ++i) {
	if (!strcmp(play_list[i].filename, play_list[i+1].filename)) {
	  /* remove index of song[i+1] except if it is playing */
	  remove_pos[n++] = (current == play_list[i+1].index) ?
	    play_list[i].index : play_list[i+1].index;
	}
      }

      /* remove from the end, otherwise positions change */
      if (n) {
	qsort(remove_pos, n, sizeof(gint), index_compare);
	do {
	  xmms_remote_playlist_delete(session, remove_pos[--n]);
	} while(n);
      }

      free(remove_pos);
      for (i = 0; i < list_size; ++i)
	free(play_list[i].filename);
    }
    free(play_list);
  }
}
