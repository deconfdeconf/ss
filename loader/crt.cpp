/* CRT library Analogue using win32 API. (c) Egoist [ni0s] */
/* ======================================================= */
/* TODO:  proper detection of hex digits by isxdigit()
 *
 */

#include <limits.h>
#include <errno.h>
#include "loader.h"

int  errno ;
/* ======================================================= */

void *memset( void *dest, int c, size_t count )
{
   register char *p_src = (char*) dest;
 
   if ( count ) 
   {
     for ( count++; --count > 0; *p_src++ = c );
   }
   return dest;
}

int memcmp ( const void *_src1, const void *_src2, size_t _num ) 
{
   register const unsigned char *p_str1 = (unsigned char*) _src1, *p_str2 = (unsigned char*) _src2;
 
   if ( _num > 0 ) 
   {
     for ( _num; ( _num > 0 ) && ( *p_str1++ == *p_str2++ ); --_num );
 
     return ( *--p_str1 - *--p_str2 );
   }
 
   return 0;
}

char *lstrstr ( const char *_src1, const char *_src2 ) 
{
	register WORD plen = lstrlen( _src2 );

	while( plen && *_src1 )
	{
		if( *_src1 == *_src2 )
			if(memcmp( _src1, _src2, plen ) == 0)
				return (char *) _src1;
		_src1++;
	}

	return NULL;
}

int strncmp ( const char *_src1, const char *_src2, size_t _num ) 
{
   if ( _num > 0 ) 
   {
     while ( _num-- > 0 ) 
     {
       if ( *_src1 != *_src2++ ) break;
       if ( *_src1++ == '\0' )   return 0;
     }

     if ( _num > 0 ) 
     {
       if ( *_src1   == '\0' ) return -1;
       if ( *--_src2 == '\0' ) return 1;
 
       return ( ( ( unsigned char ) *_src1 ) - ( ( unsigned char ) *_src2 ) );
     }
   }
 
   return 0;
}

int isspace( int c )
{
	if( (c > (int) 0x08 && c < (int) 0x0E) || c == (int) 0x20)
		return 1;
	return 0;
}
int isalpha (int _c)
{
	return IsCharAlpha((TCHAR) _c);
}
int isupper (int _c)
{
	return IsCharUpper((TCHAR) _c);
}
int isdigit( int c )
{
	return IsCharAlphaNumeric((TCHAR) c);
}

int isxdigit( int c )
{
	return IsCharAlphaNumeric((TCHAR) c);
}

int atoi  (  const char *  str   )   
{
   return ( (int)( strtol (str, (char **) NULL, 10) ) );
}

long strtol( const char *str, char **endptr, int base ) 
 {
   signed long result;
   signed long stop;
 
   const char *cursor = str;
   int i_char, sign = 0, track, threshold;
 
   do 
   {
     i_char = *cursor++;
   } while ( isspace( i_char & 0xff ) );
 
   if ( i_char == '-' ) 
   {
     sign   = 1;
     i_char = *cursor++;
   } else if ( i_char == '+' ) 
   {
     i_char = *cursor++;
   }/* end of if else */
 
   if (( base == 0 || base == 16 ) && ( i_char == '0')
     && ( *cursor == 'x' || *cursor == 'X')) 
   {
     i_char = cursor[1];
     cursor += 2;
     base = 16;
   }/* end if */
 
   if ( base == 0 )
     base = ( ( i_char == '0' ) ? 8 : 10 );
 
   stop       = ( sign ? ( - ( signed long )LONG_MIN) : LONG_MAX );
   threshold  = ( stop % ( unsigned long )base );
   stop      /= ( unsigned long )base;
 
   result = 0; track = 0;
 
   for ( ; ; ( i_char = *cursor++ ), ( i_char &= 0xff ) ) 
   {
     if ( isdigit( i_char ) ) 
     {
       i_char -= '0';
     } else if ( isalpha( i_char ) ) 
     {
       i_char -= ( isupper(i_char) ? ( 'A' - 10 ) : ( 'a' - 10 ) );
     } else 
     {
       break;
     }/* end of if elseif else */
 
     if ( i_char >= base )
       break;
 
     if ( ( track < 0 ) || ( result > stop ) || 
       ( ( result == stop ) && ( i_char > threshold ) )) 
     {
         track = -1;
       } else 
     {
         track = 1;
         result *= base;
         result += i_char;
       }/* end of if else */
   }/* for ( ; ; ( i_char = *cursor++ ), ( i_char &= 0xff ) ) */
 
   if ( track < 0 ) 
   {
     result = ( sign ? LONG_MIN : LONG_MAX );
     errno  = ERANGE;
   } else if ( sign ) 
   {
     result = -result;
   }
 
   /*if ( endptr != 0 )
     *endptr = ( track ? ( fixit( cursor, char * ) - 1 ) : 
                 fixit( str, char * ) );*/
 
  return result;
}
