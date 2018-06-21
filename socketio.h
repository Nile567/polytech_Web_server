#ifndef _messages_h
#define _messages_h

#include "stdafx.h"
// #include <Windows.h>
#include <WinSock2.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <vector>

using namespace std;

// ������ ��� ������ � ������ ����� � �����

#define CERTNAME "localhost.pem"

struct TSocketIO
{
	// public:
		SOCKET sock;
		SSL_CTX *sslctx;
		SSL *cSSL;
		bool ssl;

		void printSSLError( char *func )
		{
			int err = ERR_get_error();
			printf( "%s error %d: %s\n", func, err, ERR_error_string( err, NULL ) );
		}

		TSocketIO( SOCKET socket, bool useSsl )
		{
			sock = socket;

			ssl = useSsl;

			if (ssl)
			{
				sslctx = SSL_CTX_new( SSLv23_server_method());
				SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);
				int use_cert = SSL_CTX_use_certificate_file(sslctx, CERTNAME, SSL_FILETYPE_PEM);			
				if (use_cert == 0)
				{
					printSSLError( "SSL_CTX_use_certificate_file" );
					return;
				}

				int use_prv = SSL_CTX_use_PrivateKey_file(sslctx, CERTNAME, SSL_FILETYPE_PEM);				
				if (use_prv == 0)
				{
					printSSLError( "SSL_CTX_use_PrivateKey_file" );
					return;
				}

				cSSL = SSL_new(sslctx);
				SSL_set_fd(cSSL, sock );

				//Here is the SSL Accept portion.  Now all reads and writes must use SSL
				int ssl_err = SSL_accept(cSSL);
				if(ssl_err <= 0)
				{
					printf( "SSL error: %x", ssl_err );
					return;
				}
			}
		}

		~TSocketIO()
		{
			closesocket( sock );
		}

		// ������� ���� ��������� ������
		void readstr( char *buf );

		// ����� � ����� ������ � �������
		void writestr( char *buf );

		// ������� ������ � ������
		int recvData( char *buf, int size, int flags = 8 );
		void sendData( char *buf, int size, int flags = 0 );
};

// ���������� ��������� �������
char *skipLeadingSpaces( char *s );

char* alloccpy( char *buf );

// ����-�������� (��� ���������� � �����)
class TKeyVal
{
public:
	char* key;
	char* val;

	TKeyVal()
	{
		key = val = NULL;
	}

	// ��������� �� ������ � ������������ (key=value)
	TKeyVal( char *str, char *delim )
	{
		char *t = strstr( str, delim );

		if (t != NULL)
		{
			*t = 0;
			t++;
		}

		key = skipLeadingSpaces( str );
		val = skipLeadingSpaces(t);
	}
};

// HTTP-���������
class THttpHeader
{
public:
	// ��� ������� ���������
	vector<char*> strs;

	// ���� ���������
	vector<TKeyVal*> fields;

	// ����
	vector<TKeyVal*> cookies;

	// MIME-��� ����������� (��� ������)
	char *contentType;

	THttpHeader()
	{
		method = NULL;
		url = NULL;
		httpver = NULL;
		contentType = NULL;
	}

	~THttpHeader()
	{
	}

	// ��������� ��������� �� ������
	void ReadFromSocket( TSocketIO *sock )
	{
		char buf[1000];

		while (true)
		{
			sock->readstr( buf );

			// ������� ��������� - ������ ������
			if (buf[0] == 0)
				break;

			strs.push_back( alloccpy( buf ) );	
		}
	}

	void ReadFromSocket( TSocketIO &sock )
	{
		ReadFromSocket	( &sock );
	}

	// �������� ��������� � �����
	void WriteToSocket( TSocketIO *sock )
	{
		for( int i = 0; i < strs.size(); i++ )
			sock->writestr( strs[i] );

		// ������� ���������
		sock->writestr( "" );
	}

	void WriteToSocket( TSocketIO &sock )
	{
		WriteToSocket( &sock );
	}

	// �������� ����� �������
	void addLine( char *fmt, ... )
	{
		char buf[1000];

	    va_list argptr;
		va_start(argptr, fmt);
	    vsprintf(buf, fmt, argptr);
		va_end(argptr);

		strs.push_back( alloccpy( buf ) );
	}

	// �������� ��������� ������
	void addResponce( int code, char *descr, int contentSize )
	{
		addLine( "HTTP/1.1 %d %s", code, descr );
		if (contentType == NULL)
			addLine( "Content-Type: text/html" );
		else
			addLine( "Content-Type: %s", contentType );
		addLine( "Connection: Closed" );
		addLine( "Content-Length: %d", contentSize );
	}
	
	void addResponce( int code, char *descr, char *content )
	{
		addResponce( code, descr, strlen(content) );
	}

	// �����, �����, ������ ���������
	char *method, *url, *httpver;

	// ��������� ������
	void parseReq()
	{
		if (strs.size() == 0) return;

		char *t;

		// ������ ������� - ������
		method = strtok_s( strs[0], " ", &t );
		url = strtok_s( NULL, " ", &t );
		httpver = strtok_s( NULL, " ", &t );

		// ��������� - ������ ���������
		for( int i = 1; i < strs.size(); i++ )
		{
			TKeyVal *v = new TKeyVal( strs[i], ":" );
			fields.push_back( v );

			// � �� ���� �� ���?
			if (strcmp( v->key, "Cookie" ) == 0)
			{
				char *c, *t2;

				// ��������� ����
				c = strtok_s( v->val, ";", &t2 );

				while ( c != NULL )
				{
					TKeyVal *v2 = new TKeyVal( c, "=" );
					cookies.push_back( v2 );

					c = strtok_s( NULL, ";", &t2 );
				}
			}
		}
	}

	// ����� ���� �� �����
	char *getCookie( char *name )
	{
		for( int i = 0; i < cookies.size(); i++ )
			if (strcmp(cookies[i]->key, name) == 0)
				return cookies[i]->val;

		return NULL;
	}

	// ����� �������� �� �����
	char *getField( char *name )
	{
		for( int i = 0; i < fields.size(); i++ )
			if (strcmp(fields[i]->key, name) == 0)
				return fields[i]->val;

		return NULL;
	}
};

void InitializeSSL();
void DestroySSL();

#endif // _messages_h