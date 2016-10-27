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


#define		MAX_FILES		16
#define		MAX_LINE_LENGTH	16

#define		ARRAY_LEN(x)	((int) (sizeof (x) / sizeof ((x) [0])))

enum
{	MODE_DIFF = 1,
	MODE_SAME = 2
} ;

typedef struct
{	u_char	n [MAX_LINE_LENGTH * 2] ;
	u_char	c [MAX_LINE_LENGTH] ;
	short	s [MAX_LINE_LENGTH / sizeof (short)] ;
	int		i [MAX_LINE_LENGTH / sizeof (int)] ;
	float	f [MAX_LINE_LENGTH / sizeof (float)] ;
	double	d [MAX_LINE_LENGTH / sizeof (double)] ;
} TYPE_DATA ;

typedef struct
{	char	nd [MAX_LINE_LENGTH * 2] ;
	char	cd [MAX_LINE_LENGTH] ;
	char	sd [MAX_LINE_LENGTH / sizeof (short)] ;
	char	id [MAX_LINE_LENGTH / sizeof (int)] ;
	char	fd [MAX_LINE_LENGTH / sizeof (float)] ;
	char	dd [MAX_LINE_LENGTH / sizeof (double)] ;
} DIFF_DATA ;

typedef struct
{	FILE		*file ;

	int			line_length ;
	int			offset ;

	TYPE_DATA 	data ;
} FILE_DATA ;

typedef struct
{	FILE_DATA	fd [MAX_FILES] ;

	DIFF_DATA	diffs ;

	int		file_count ;	/* Number of files we're processing. */
	size_t	min_file_len ;	/* Length of the smallest file. */

	int		block_lines ; 	/* Number of lines required to print wack block. */
	size_t	block_length ; 	/* Number of items on a single line. */

	int		offset ;		/* Current file offset. */
	int 	mode ;			/* Mark differences or similarities. */
} HEXDIFF_DATA ;

void	open_files (HEXDIFF_DATA *data, int argc, char **argv) ;
void	close_files (HEXDIFF_DATA *data) ;
void	get_current_block (HEXDIFF_DATA *data) ;


