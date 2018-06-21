#include "stdafx.h"
#include "socketio.h"
#include <string.h>
#include <WinSock2.h>

// #include <openssl/applink.c>

void TSocketIO::readstr( char *buf )
{
	int r;
	char ch;

	buf[0] = 0;

	while (true)
	{
		if ((r = recvData( &ch, 1, 8 /* MSG_WAITALL */ )) <= 0)
			return;

		if (ch == '\r')
		{
			r = recvData( &ch, 1, 8 );
			break;
		}
		if (ch == '\n')
			break;

		*(buf++) = ch;
	}

	*buf = 0;
}

void TSocketIO::writestr( char *buf )
{
	char bufp[]="\r\n";

	sendData( buf, strlen(buf) );
	sendData( &bufp[0], 2 );
}

char* alloccpy( char *buf )
{
	char *s = (char*) malloc( strlen( (char*) buf)+1 );
	strcpy( s, (char*) buf );
	return s;
}

char *skipLeadingSpaces( char *s )
{
	while (*s == ' ')
		s++;
	return s;
}


int TSocketIO::recvData( char *buf, int size, int flags )
{
	if (ssl)
	{
		int r = SSL_read( cSSL, buf, size );

		if (r < 0)
			printSSLError( "SSL_read" );

		return r;
	}
	else 
		return ::recv( sock,buf, size, flags );
}

void TSocketIO::sendData( char *buf, int size, int flags )
{
	if (ssl)
	{
		int r = SSL_write( cSSL, buf, size );
		if (r < 0)
			printSSLError( "SSL_write" );
	}
	else
		::send( sock, buf, size, flags );
}

void InitializeSSL()
{
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
}

void DestroySSL()
{
    ERR_free_strings();
    EVP_cleanup();
}

void ShutdownSSL()
{
    /* SSL_shutdown(cSSL);
    SSL_free(cSSL); // */
}