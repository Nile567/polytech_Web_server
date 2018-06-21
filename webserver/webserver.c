// webserver.cpp: ���������� ����� ����� ��� ����������� ����������.
//

#include "stdafx.h"
#include <process.h>
#include <locale.h>
#include <Windows.h>
#include "socketio.h"

// ������� �������
void SocketThread( void *data )
{
	// �����������
	TSocketIO sockio( (SOCKET) data, true );
	THttpHeader req, resp;

	// ������ ���������
	req.ReadFromSocket( sockio );
	req.parseReq();

	// ������� ��� �������
	printf( "Method %s url %s (%s)\n", req.method, req.url, req.httpver );

	char content[1000];

	// �����-�� ������
	if ( (req.method == NULL) ) // || (strcmp( req.method, "GET" ) != 0) )
	{
		sprintf( content, "<html><body>unsupported</body></html>" );

		resp.addResponce( 500, "Unsupported method", content );
		resp.WriteToSocket( sockio );
		sockio.writestr( content );

		return;
	}

	// �������� �� ���������
	if (strcmp( req.url, "/" ) == 0)
		req.url = "/index.html";

	char fname[100];
	sprintf( fname, "www\\%s", &req.url[1] );

	// �������� � ������ �����
	if (strcmp( req.url, "/cookie" ) == 0)
	{
		// ���� ����
		char *t = req.getCookie( "cnt" );
		int cnt = 0;

		// ���� ���� - ������ �� ��� �����
		if (t != NULL)
			sscanf( t, "%d", &cnt );

		cnt++;

		// �������� (���������� �� �������)
		sprintf( content, "<html><body>���� �����<br/>"
			"�� ���� �� ���� �������� %d ���</body></html>", cnt );

		resp.addResponce( 200, "OK", content );
		// ���������� ���� �������
		resp.addLine( "Set-Cookie: cnt=%d", cnt );
		resp.WriteToSocket( sockio );
		sockio.writestr( content );

		return;
	}

	// ��������� ��������
	if (strcmp( req.url, "/secret" ) == 0)
	{
		// ����� ���� user
		char *t = req.getCookie( "user" );

		// ���� �� ��� - ������
		if (t == NULL)
		{
			char *content = "<html><body><h1>������ ��������</h1></body></html>";

			resp.addResponce( 403, "access denied", content );
			resp.WriteToSocket( sockio );
			sockio.writestr( content );

			return;
		}

		// ������� ������ ����������
		char content[1000];
		sprintf( content, "<html><body><h1>��������� ������</h1>������ %s<br/>"
			"<a href=\"/logout\">�����</a></body></html>", t );

		resp.addResponce( 200, "Ok", content );
		resp.WriteToSocket( sockio );
		sockio.writestr( content );

		return;
	}
