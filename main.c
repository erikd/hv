/*
** Copyright (C) 2003-2016 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<curses.h>
#include	<signal.h>

#include	"hexdump.h"


#define		LINE_SIZE	16

#define		VERSION_MAJOR	1
#define		VERSION_MINOR	4


/*--------------------------------------------------------------------
** Typedefs.
*/

typedef	void (*ScreenFunc) (WINDOW *win, HEXDIFF_DATA*) ;

/*--------------------------------------------------------------------
** Static data.
*/
static	char ProgName [] = "hv" ;
static	HEXDIFF_DATA data ;

/*--------------------------------------------------------------------
** Function Prototypes.
*/
static	void sigint_handler (int sig) ;

static	void hexdiff_fill_screen (WINDOW *win, HEXDIFF_DATA *data) ;
static	void lines_fill_screen (WINDOW *win, HEXDIFF_DATA *data) ;

static	void int_fill_screen (WINDOW *win, HEXDIFF_DATA *data) ;
static	void short_diff_fill_screen (WINDOW *win, HEXDIFF_DATA *data) ;

static	void float_fill_screen (WINDOW *win, HEXDIFF_DATA *data) ;
static	void double_fill_screen (WINDOW *win, HEXDIFF_DATA *data) ;

static void atexit_handler (void) ;

/*--------------------------------------------------------------------
*/

int
main	(int argc, char *argv [])
{	ScreenFunc	fill_screen ;
	WINDOW		*main_win ;
	struct		sigaction sigdata ;
	int			ch = 0, k, ch_mem = 0 ;

	printf ("%s - %d.%02d\n", ProgName, VERSION_MAJOR, VERSION_MINOR) ;

	atexit (atexit_handler) ;

	open_files (&data, argc, argv) ;

	if (data.file_count < 1)
	{	printf ("No files.\n") ;
		return 0 ;
		} ;

	fill_screen = hexdiff_fill_screen ;

	/* 	Set up signal handler to restore the normal screen
	**	operation if the program is killed.
	*/
	sigdata.sa_handler = sigint_handler ;
	sigaction (SIGINT, &sigdata, NULL) ;

	/* Do curses stuff. */
	main_win = initscr () ;				/* initialize the curses library */
	intrflush (stdscr, FALSE) ;			/* ????????????? */

	keypad (stdscr, TRUE) ;				/* enable keyboard mapping */
	cbreak () ;							/* take input chars one at a time, no wait for \n */
	noecho () ;							/* don't echo input */
	curs_set (0) ;						/* hide the cursor. */
	nonl () ;

    if (has_colors ())
    {	start_color () ;

		/* Assignment colours. */
		init_pair (COLOR_WHITE, COLOR_WHITE, COLOR_BLACK) ;
		init_pair (COLOR_GREEN, COLOR_GREEN, COLOR_BLACK) ;
		init_pair (COLOR_RED, COLOR_RED, COLOR_BLACK) ;
		init_pair (COLOR_CYAN, COLOR_CYAN, COLOR_BLACK) ;
		init_pair (COLOR_WHITE, COLOR_WHITE, COLOR_BLACK) ;
		init_pair (COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK) ;
		init_pair (COLOR_BLUE, COLOR_BLUE, COLOR_BLACK) ;
		init_pair (COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK) ;
		} ;


	werase (main_win) ;

	keypad (main_win, TRUE) ;

	/* Main program loop. */
	while (ch != 'q')
	{
		if (data.offset < 0)
			data.offset = 0 ;

		fill_screen (main_win, &data) ;
		wrefresh (main_win) ;

		ch = wgetch (main_win) ;

		switch (ch)
		{	case KEY_DOWN :
					data.offset += data.block_length ;
					break ;

			case KEY_UP :
					data.offset -= data.block_length ;
					break ;

			case KEY_NPAGE :
			case ' ' :
					data.offset += (LINES / data.block_lines) * data.block_length ;
					break ;

			case KEY_PPAGE :
					data.offset -= (LINES / data.block_lines) * data.block_length ;
					break ;

			case KEY_HOME :
					data.offset = 0 ;
					break ;

			case KEY_END :
					data.offset = data.min_file_len - (data.min_file_len & 0xf) ;
					break ;

			case KEY_RESIZE :
					clear () ;
					break ;

			case KEY_LEFT:
					if (data.file_count != 2)
						break ;

					if (data.fd [1].offset > 0)
						data.fd [1].offset -- ;
					else
						data.fd [0].offset ++ ;

					clear () ;
					break ;

			case KEY_RIGHT :
					if (data.file_count != 2)
						break ;

					if (data.fd [0].offset > 0)
						data.fd [0].offset -- ;
					else
						data.fd [1].offset ++ ;

					clear () ;
					break ;

			case 'h' :
					werase (main_win) ;
					data.block_length = 16 ;
					fill_screen = hexdiff_fill_screen ;
					clear () ;
					break ;

			case 'l' :
					werase (main_win) ;
					fill_screen = lines_fill_screen ;
					clear () ;
					break ;

			case 's' :
					werase (main_win) ;
					data.block_length = 8 ;
					fill_screen = short_diff_fill_screen ;
					clear () ;
					break ;

			case 'i' :
					werase (main_win) ;
					data.block_length = 8 ;
					fill_screen = int_fill_screen ;
					clear () ;
					break ;

			case 'd' :
					werase (main_win) ;
					data.block_length = 16 ;
					fill_screen = double_fill_screen ;
					clear () ;
					break ;

			case 'f' :
					werase (main_win) ;
					data.block_length = 16 ;
					fill_screen = float_fill_screen ;
					clear () ;
					break ;

			case 'r' :
				for (k = 0 ; k < data.file_count ; k++)
					data.fd [k].offset = 0 ;
				clear () ;
				break ;

			default :
				if (ch > 255)
					ch_mem = 0 ;
				else
					ch_mem = (ch_mem << 8) | ch ;
				break ;
			} ;

		switch (ch_mem)
		{	case 0x1b4f3244 :
					data.offset ++ ;
					ch_mem = 0 ;
					break ;

			case 0x1b4f3243 :
					data.offset -- ;
					ch_mem = 0 ;
					break ;

				break ;

			default : break ;
			} ;
		} ;

	/* Call exit () so atexit_handler () gets called. */
	exit (0) ;

	return 0 ;
} /* main */

static	void
hexdiff_fill_screen (WINDOW *win, HEXDIFF_DATA *data)
{	int count, file, k, save_offset ;
	size_t m ;
	unsigned char	ch ;

	save_offset = data->offset ;
	count = LINES / data->block_lines ;

	wmove (win, 0, 0) ;

	for (k = 0 ; k < count ; k++)
	{	get_current_block (data) ;

		for (file = 0 ; file < data->file_count ; file++)
		{	if (file == 0 || (data->fd [0].offset != data->fd [file].offset))
			{	wattrset (win, COLOR_PAIR (3)) ;
				wprintw (win, "%08X:", data->offset + data->fd [file].offset) ;
				}
			else
				wprintw (win, "         ") ;

			wattrset (win, COLOR_PAIR (1)) ;
			for (m = 0 ; m < data->block_length ; m++)
			{	wattrset (win, COLOR_PAIR (0)) ;
				waddch (win, ' ') ;
				if ((m % 8) == 0)
					waddch (win, ' ') ;
				wattrset (win, COLOR_PAIR (data->diffs.nd [2*m])) ;
				waddch (win, data->fd [file].data.n [2*m]) ;
				wattrset (win, COLOR_PAIR (data->diffs.nd [2*m+1])) ;
				waddch (win, data->fd [file].data.n [2*m+1]) ;
				} ;

			wattrset (win, COLOR_PAIR (0)) ;
			wprintw (win, "  ") ;

			for (m = 0 ; m < data->block_length ; m++)
			{	if ((m % 8) == 0)
				{	wattrset (win, COLOR_PAIR (0)) ;
					waddch (win, ' ') ;
					} ;
				wattrset (win, COLOR_PAIR (data->diffs.cd [m])) ;
				ch = data->fd [file].data.c [m] ;
				waddch (win, isprint (ch) ? ch : '.') ;
				} ;
			waddch (win, '\n') ;
			} ;

		if (data->file_count > 1)
			waddch (win, '\n') ;
		data->offset += data->block_length ;
		} ;

	data->offset = save_offset ;

	return ;
}	/* hexdiff_fill_screen */

static	void
lines_fill_screen (WINDOW *win, HEXDIFF_DATA *data)
{	int count, file, k, save_offset ;
	size_t m ;
	unsigned char	ch ;

	save_offset = data->offset ;
	count = LINES / data->block_lines ;

	wmove (win, 0, 0) ;

	for (k = 0 ; k < count ; k++)
	{	get_current_block (data) ;

		for (file = 0 ; file < data->file_count ; file++)
		{	if (file == 0 || (data->fd [0].offset != data->fd [file].offset))
			{	wattrset (win, COLOR_PAIR (3)) ;
				wprintw (win, "%08X:", data->offset + data->fd [file].offset) ;
				}
			else
				wprintw (win, "         ") ;

			wattrset (win, COLOR_PAIR (1)) ;
			for (m = 0 ; m < data->block_length ; m++)
			{	wattrset (win, COLOR_PAIR (0)) ;
				waddch (win, ' ') ;
				if ((m % 8) == 0)
					waddch (win, ' ') ;
				wattrset (win, COLOR_PAIR (data->diffs.nd [2*m])) ;
				waddch (win, data->fd [file].data.n [2*m]) ;
				wattrset (win, COLOR_PAIR (data->diffs.nd [2*m+1])) ;
				waddch (win, data->fd [file].data.n [2*m+1]) ;
				} ;

			wattrset (win, COLOR_PAIR (0)) ;
			wprintw (win, "  ") ;

			for (m = 0 ; m < data->block_length ; m++)
			{	if ((m % 8) == 0)
				{	wattrset (win, COLOR_PAIR (0)) ;
					waddch (win, ' ') ;
					} ;
				wattrset (win, COLOR_PAIR (data->diffs.cd [m])) ;
				ch = data->fd [file].data.c [m] ;
				waddch (win, isprint (ch) ? ch : '.') ;
				} ;
			waddch (win, '\n') ;
			} ;

		if (data->file_count > 1)
			waddch (win, '\n') ;
		data->offset += data->block_length ;
		} ;

	data->offset = save_offset ;

	return ;
} /* lines_fill_screen */

static void
short_diff_fill_screen (WINDOW *win, HEXDIFF_DATA *data)
{	int count, file, k, save_offset ;
	size_t m ;
	unsigned char ch ;

	save_offset = data->offset ;
	count = LINES / data->block_lines ;

	wmove (win, 0, 0) ;

	for (k = 0 ; k < count ; k++)
	{	get_current_block (data) ;

		for (file = 0 ; file < data->file_count ; file++)
		{
			/* Print the hex address but only for the first file (eg, when in multi-file diff mode) */
			if (file == 0 || (data->fd [0].offset != data->fd [file].offset))
			{	wattrset (win, COLOR_PAIR (3)) ;
				wprintw (win, "%08X:", data->offset + data->fd [file].offset) ;
				}
			else
				wprintw (win, "         ") ;

			/* 16 bit hex dump. */
			for (m = 0 ; m < data->fd [file].line_length / sizeof (short) / 2 ; m++)
			{	wattrset (win, COLOR_PAIR (0)) ;
				waddch (win, ' ') ;
				wattrset (win, COLOR_PAIR (data->diffs.sd [m])) ;
				wprintw (win, "%04X", 0xFFFF & data->fd [file].data.s [m]) ;
				} ;

			wattrset (win, COLOR_PAIR (0)) ;
			for ( ; m < data->block_length / sizeof (short) ; m++)
				wprintw (win, "     ") ;

			/* Space between 16 bit hex and character display. */
			wattrset (win, COLOR_PAIR (0)) ;
			wprintw (win, "  ") ;

			for (m = 0 ; m < (size_t) data->fd [file].line_length / 2 ; m++)
			{	if ((m % 8) == 0)
					waddch (win, ' ') ;
				wattrset (win, COLOR_PAIR (data->diffs.cd [m])) ;
				ch = data->fd [file].data.c [m] ;
				waddch (win, isprint (ch) ? ch : '.') ;
				} ;

			wattrset (win, COLOR_PAIR (data->diffs.cd [m])) ;
			for ( ; m < data->block_length  ; m++)
				waddch (win, ' ') ;

			wattrset (win, COLOR_PAIR (0)) ;
			waddch (win, ' ') ;

			/* Short int display */
			for (m = 0 ; m < data->fd [file].line_length / sizeof (short) / 2 ; m++)
			{	wattrset (win, COLOR_PAIR (data->diffs.sd [m])) ;
				wprintw (win, " %7d", data->fd [file].data.s [m]) ;
				}

			waddch (win, '\n') ;
			} ;

		if (data->file_count > 1)
			waddch (win, '\n') ;
		data->offset += data->block_length ;
		} ;

	data->offset = save_offset ;

	return ;
}	/* short_diff_fill_screen */

static void
int_fill_screen (WINDOW *win, HEXDIFF_DATA *data)
{	int k, save_offset, *iptr ;
	size_t m ;

	save_offset = data->offset ;
	wmove (win, 0, 0) ;

	for (k = 0 ; k < LINES ; k++)
	{	wattrset (win, COLOR_PAIR (3)) ;
		wprintw (win, "%08X: ", data->offset) ;

		get_current_block (data) ;

		wattrset (win, COLOR_PAIR (0)) ;
		iptr = data->fd [0].data.i ;
		for (m = 0 ; m < data->block_length / sizeof (int) ; m++)
			wprintw (win, "%08X ", iptr [m]) ;

		wattrset (win, COLOR_PAIR (0)) ;
		wprintw (win, " ") ;

		for (m = 0 ; m < data->block_length / sizeof (int) ; m++)
			wprintw (win, "%12d ", iptr [m]) ;

		waddch (win, '\n') ;

		data->offset += data->block_length ;
		} ;

	data->offset = save_offset ;

	return ;
}	/* int_fill_screen */

static void
float_fill_screen (WINDOW *win, HEXDIFF_DATA *data)
{	int k, save_offset, *iptr ;
	size_t m ;
	float *fptr ;

	save_offset = data->offset ;
	wmove (win, 0, 0) ;

	for (k = 0 ; k < LINES ; k++)
	{	wattrset (win, COLOR_PAIR (3)) ;
		wprintw (win, "%08X: ", data->offset) ;

		get_current_block (data) ;

		wattrset (win, COLOR_PAIR (0)) ;
		fptr = data->fd [0].data.f ;
		iptr = data->fd [0].data.i ;
		for (m = 0 ; m < data->block_length / sizeof (int) ; m++)
			wprintw (win, "%08X ", iptr [m]) ;

		wattrset (win, COLOR_PAIR (0)) ;
		wprintw (win, " ") ;

		for (m = 0 ; m < data->block_length / sizeof (float) ; m++)
			wprintw (win, "%14g ", fptr [m]) ;

		waddch (win, '\n') ;

		data->offset += data->block_length ;
		} ;

	data->offset = save_offset ;

	return ;
}	/* float_fill_screen */

static void
double_fill_screen (WINDOW *win, HEXDIFF_DATA *data)
{	int k, save_offset, *iptr ;
	size_t m ;
	double *dptr ;

	save_offset = data->offset ;
	wmove (win, 0, 0) ;

	for (k = 0 ; k < LINES ; k++)
	{	wattrset (win, COLOR_PAIR (3)) ;
		wprintw (win, "%08X: ", data->offset) ;

		get_current_block (data) ;

		wattrset (win, COLOR_PAIR (0)) ;
		dptr = data->fd [0].data.d ;
		iptr = data->fd [0].data.i ;
		for (m = 0 ; m < data->block_length / sizeof (int) ; m+= sizeof (double) / sizeof (int))
			wprintw (win, "%08X ", iptr [m]) ;

		wattrset (win, COLOR_PAIR (0)) ;
		wprintw (win, " ") ;

		for (m = 0 ; m < data->block_length / sizeof (double) ; m++)
			wprintw (win, "%+16g ", dptr [m]) ;

		waddch (win, '\n') ;

		data->offset += data->block_length ;
		} ;

	data->offset = save_offset ;

	return ;
}	/* double_fill_screen */

static void
sigint_handler (int sig)
{
	sig = sig ;
}	/* sigint_handler */

static void
atexit_handler (void)
{
	endwin () ;

	close_files (&data) ;
} /* atexit_handler */

