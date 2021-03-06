/*
 * This file is part of the Tycoon-2 system.
 *
 * The Tycoon-2 system is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (Version 2).
 *
 * The Tycoon-2 system is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with the Tycoon-2 system; see the file LICENSE.
 * If not, write to AB 4.02, Softwaresysteme, TU Hamburg-Harburg
 * D-21071 Hamburg, Germany. http://www.sts.tu-harburg.de
 * 
 * Copyright (c) 1996-1998 Higher-Order GmbH, Hamburg. All rights reserved.
 *
 */
/*
 * This file is part of the Tycoon-2 system.
 *
 * The Tycoon-2 system is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (Version 2).
 *
 * The Tycoon-2 system is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with the Tycoon-2 system; see the file LICENSE.
 * If not, write to AB 4.02, Softwaresysteme, TU Hamburg-Harburg
 * D-21071 Hamburg, Germany. http://www.sts.tu-harburg.de
 * 
 * Copyright (c) 1996-1998 Higher-Order GmbH, Hamburg. All rights reserved.
 *
 */
/*
 * Copyright 1993, 1995 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */
 
# if defined ( NT ) || defined ( WIN95 ) || defined ( WIN98 )

# ifdef __BORLANDC__
# include <dir.h>
# include <dos.h>
# undef FILENAME	/* cpp namespace collision */
# define _finddata_t ffblk
# endif

# include "jam.h"
# include "filesys.h"

# include <io.h>
# include <sys/stat.h>

/*
 * filent.c - scan directories and archives on NT
 *
 * External routines:
 *
 *	file_dirscan() - scan a directory for files
 *	file_time() - get timestamp of file, if not done by file_dirscan()
 *	file_archscan() - scan an archive for files
 *
 * File_dirscan() and file_archscan() call back a caller provided function
 * for each file found.  A flag to this callback function lets file_dirscan()
 * and file_archscan() indicate that a timestamp is being provided with the
 * file.   If file_dirscan() or file_archscan() do not provide the file's
 * timestamp, interested parties may later call file_time().
 *
 * 07/10/95 (taylor)   Findfirst() returns the first file on NT.
 * 05/03/96 (seiwald)  split apart into pathnt.c
 * 04/10/98 (willomat) Error by _findfirst with 'c:\\*' or 'c:\/*'
 *                     under Windows 95/98
 */

/*
 * file_dirscan() - scan a directory for files
 */

void
file_dirscan( dir, func )
char	*dir;
void	(*func)();
{
	FILENAME f;
	char filespec[ MAXJPATH ];
	char filename[ MAXJPATH ];
	long handle;
	int ret;
	struct _finddata_t finfo[1];

	/* First enter directory itself */

	memset( (char *)&f, '\0', sizeof( f ) );

	f.f_dir.ptr = dir;
	f.f_dir.len = strlen(dir);

	dir = *dir ? dir : ".";

 	/* Special case \ or d:\ : enter it */
 
 	if( f.f_dir.len == 1 && f.f_dir.ptr[0] == '\\' )
 	    (*func)( dir, 0 /* not stat()'ed */, (time_t)0 );
 	else if( f.f_dir.len == 3 && f.f_dir.ptr[1] == ':' )
 	    (*func)( dir, 0 /* not stat()'ed */, (time_t)0 );

	/* Now enter contents of directory */
	if (f.f_dir.ptr[f.f_dir.len - 1] == '\\')
	   sprintf( filespec, "%s*", dir );
	else
           sprintf( filespec, "%s\\*", dir );

	if( DEBUG_BINDSCAN )
	    printf( "scan directory %s\n", dir );

# ifdef __BORLANDC__
	if ( ret = findfirst( filespec, finfo, FA_NORMAL | FA_DIREC ) )
	    return;

	while( !ret )
	{
	    time_t time_write = finfo->ff_fdate;

	    time_write = (time_write << 16) | finfo->ff_ftime;
	    f.f_base.ptr = finfo->ff_name;
	    f.f_base.len = strlen( finfo->ff_name );

	    file_build( &f, filename );

	    (*func)( filename, 1 /* stat()'ed */, time_write );

	    ret = findnext( finfo );
	}
# else
	handle = _findfirst( filespec, finfo );

	if( ret = ( handle < 0L ) )
	    return;

	while( !ret )
	{
	    f.f_base.ptr = finfo->name;
	    f.f_base.len = strlen( finfo->name );

	    file_build( &f, filename, 0 );

	    (*func)( filename, 1 /* stat()'ed */, finfo->time_write );

	    ret = _findnext( handle, finfo );
	}

	_findclose( handle );
# endif

}

/*
 * file_time() - get timestamp of file, if not done by file_dirscan()
 */

int
file_time( filename, time )
char	*filename;
time_t	*time;
{
	/* On NT this is called only for C:/ */

	struct stat statbuf;

	if( stat( filename, &statbuf ) < 0 )
	    return -1;

	*time = statbuf.st_mtime;

	return 0;
}

/*
 * file_archscan() - scan an archive for files
 */

/* Straight from SunOS */

#define	ARMAG	"!<arch>\n"
#define	SARMAG	8

#define	ARFMAG	"`\n"

struct ar_hdr {
	char	ar_name[16];
	char	ar_date[12];
	char	ar_uid[6];
	char	ar_gid[6];
	char	ar_mode[8];
	char	ar_size[10];
	char	ar_fmag[2];
};

# define SARFMAG 2
# define SARHDR sizeof( struct ar_hdr )

void
file_archscan( archive, func )
char *archive;
void (*func)();
{
	struct ar_hdr ar_hdr;
	char *string_table = 0;
	char buf[ MAXJPATH ];
	long offset;
	int fd;

	if( ( fd = open( archive, O_RDONLY | O_BINARY, 0 ) ) < 0 )
	    return;

	if( read( fd, buf, SARMAG ) != SARMAG ||
	    strncmp( ARMAG, buf, SARMAG ) )
	{
	    close( fd );
	    return;
	}

	offset = SARMAG;

	if( DEBUG_BINDSCAN )
	    printf( "scan archive %s\n", archive );

	while( read( fd, &ar_hdr, SARHDR ) == SARHDR &&
	       !memcmp( ar_hdr.ar_fmag, ARFMAG, SARFMAG ) )
	{
	    long    lar_date;
	    long    lar_size;
	    char    *name = 0;
 	    char    *endname;
	    char    *c;

	    sscanf( ar_hdr.ar_date, "%ld", &lar_date );
	    sscanf( ar_hdr.ar_size, "%ld", &lar_size );

	    lar_size = ( lar_size + 1 ) & ~1;

	    if (ar_hdr.ar_name[0] == '/' && ar_hdr.ar_name[1] == '/' )
	    {
		/* this is the "string table" entry of the symbol table,
		** which holds strings of filenames that are longer than
		** 15 characters (ie. don't fit into a ar_name
		*/

		string_table = malloc(lar_size);
		if (read(fd, string_table, lar_size) != lar_size)
		    printf("error reading string table\n");
		offset += SARHDR + lar_size;
		continue;
	    }
	    else if (ar_hdr.ar_name[0] == '/' && ar_hdr.ar_name[1] != ' ')
	    {
		/* Long filenames are recognized by "/nnnn" where nnnn is
		** the offset of the string in the string table represented
		** in ASCII decimals.
		*/

		name = string_table + atoi( ar_hdr.ar_name + 1 );
		endname = name + strlen( name );
	    }
	    else
	    {
		/* normal name */
		name = ar_hdr.ar_name;
		endname = name + sizeof( ar_hdr.ar_name );
	    }

	    /* strip trailing space, slashes, and backslashes */

	    while( endname-- > name )
		if( *endname != ' ' && *endname != '\\' && *endname != '/' )
		    break;
	    *++endname = 0;

	    /* strip leading directory names, an NT specialty */

	    if( c = strrchr( name, '/' ) )
		name = c + 1;
	    if( c = strrchr( name, '\\' ) )
		name = c + 1;

	    sprintf( buf, "%s(%.*s)", archive, endname - name, name );
	    (*func)( buf, 1 /* time valid */, (time_t)lar_date );

	    offset += SARHDR + lar_size;
	    lseek( fd, offset, 0 );
	}

	close( fd );
}

# endif /* NT */
