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
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>

#include	"hexdump.h"


static size_t
get_file_length (FILE *file)
{	size_t	len ;

	fseek (file, 0, SEEK_SET) ;
	fseek (file, 0, SEEK_END) ;
	len = ftell (file) ;
	fseek (file, 0, SEEK_SET) ;

	return len ;
}	/* get_file_length */


void
open_files (HEXDIFF_DATA *data, int argc, char **argv)
{	size_t	file_len ;
	int		k ;

	/* Initialize the HEXDIFF_DATA structure. */
	memset (data, 0, sizeof (HEXDIFF_DATA)) ;
	data->block_length = 16 ;

	for (k = 1 ; k < argc && data->file_count < MAX_FILES ; k++)
	{	if (strcmp (argv [k], "-same") == 0)
		{	data->mode = MODE_SAME ;
			continue ;
			}
		else if (strcmp (argv [k], "-nib_diff") == 0)
		{	data->mode = MODE_DIFF ;
			continue ;
			}

		data->fd [data->file_count].file = fopen (argv [k], "rb") ;
		if (! data->fd [data->file_count].file)
			continue ;

		/* Find and store the file length of the first file or
		** the Nth file if that is shorter than the first.
		*/
		file_len = get_file_length (data->fd [data->file_count].file) ;
		if (data->file_count == 0 || data->min_file_len > file_len)
			data->min_file_len = file_len ;

		data->file_count ++ ;
		} ;

	data->block_lines = (data->file_count > 1) ? data->file_count + 1 : 1 ;

	return ;
} /* open_files */

void
close_files (HEXDIFF_DATA *data)
{	int		k ;

	for (k = 0 ; k < data->file_count ; k++)
	{	if (data->fd [k].file)
			fclose (data->fd [k].file) ;
		data->fd [k].file = NULL ;
		} ;

	/* Initialize the HEXDIFF_DATA structure. */
	memset (data, 0, sizeof (HEXDIFF_DATA)) ;

	return ;
} /* close_files */

static void
generate_nibbles (HEXDIFF_DATA *data)
{	static	char HexStr [16] = "0123456789ABCDEF" ;
	int 	k, m ;


	for (k = 0 ; k < data->file_count ; k++)
	{	memset (data->fd [k].data.n, ' ', sizeof (data->fd [k].data.n)) ;

		for (m = 0 ; m < data->fd [k].line_length ; m++)
		{	data->fd [k].data.n [2 * m] = HexStr [(data->fd [k].data.c [m] >> 4) & 0xF] ;
			data->fd [k].data.n [2 * m + 1] = HexStr [data->fd [k].data.c [m] & 0xF] ;
			} ;
		} ;

	return ;
} /* generate_nibbles */

static void
generate_differences (HEXDIFF_DATA *data)
{	int 	k, m ;

	if (data->block_length > MAX_LINE_LENGTH)
	{	/* Bad karma. Not sure how to handle this. */
		exit (0) ;
		} ;

	/* Calculate the diffs. */
	for (m = 0 ; m < ARRAY_LEN (data->fd [k].data.c) ; m++)
	{	data->diffs.cd [m] = 0 ;
		for (k = 1 ; k < data->file_count ; k++)
			if (data->fd [k].data.c [m] != data->fd [0].data.c [m])
				data->diffs.cd [m] = 1 ;
		} ;

	for (m = 0 ; m < ARRAY_LEN (data->fd [k].data.n) ; m++)
	{	data->diffs.nd [m] = 0 ;
		for (k = 1 ; k < data->file_count ; k++)
			if (data->fd [k].data.n [m] != data->fd [0].data.n [m])
				data->diffs.nd [m] = 1 ;
		} ;

	for (m = 0 ; m < ARRAY_LEN (data->fd [k].data.s) ; m++)
	{	data->diffs.sd [m] = 0 ;
		for (k = 1 ; k < data->file_count ; k++)
			if (data->fd [k].data.s [m] != data->fd [0].data.s [m])
				data->diffs.sd [m] = 1 ;
		} ;

	return ;
} /* generate_differences */

void
get_current_block (HEXDIFF_DATA *data)
{	int k ;

	for (k = 0 ; k < data->file_count ; k++)
	{	memset (data->fd [k].data.c, ' ', sizeof (data->fd [k].data.c)) ;
		fseek (data->fd [k].file, data->offset + data->fd [k].offset, SEEK_SET) ;
		data->fd [k].line_length = fread (data->fd [k].data.c, 1, MAX_LINE_LENGTH, data->fd [k].file) ;

		memcpy (data->fd [k].data.s, data->fd [k].data.c, sizeof (data->fd [k].data.c)) ;
		memcpy (data->fd [k].data.i, data->fd [k].data.c, sizeof (data->fd [k].data.c)) ;
		memcpy (data->fd [k].data.f, data->fd [k].data.c, sizeof (data->fd [k].data.c)) ;
		memcpy (data->fd [k].data.d, data->fd [k].data.c, sizeof (data->fd [k].data.c)) ;
		} ;

	generate_nibbles (data) ;
	generate_differences (data) ;

	return ;
} /* get_current_block */

