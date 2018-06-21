// webserver.cpp: ���������� ����� ����� ��� ����������� ����������.
//

#include "stdafx.h"
#include <process.h>
#include <locale.h>
#include <Windows.h>
#include "socketio.h"

	// �������� ������ (����� ������ �������� ���� � ������ ����)
	if (strcmp( req.url, "/login" ) == 0)
	{
		char formdata[100];

		// ������ ����� (����� - post)
		char *t = req.getField( "Content-Length" );
		int l;

		sscanf( t, "%d", &l );

		memset( formdata, 0, sizeof(formdata) );
		sockio.recvData( formdata, l );		

		// ��������� �����
		char *user, *pass;

		user = strtok_s( formdata, "&", &t );
		pass = strtok_s( NULL, "&", &t );

		// ��������� ���������
		if ( (strcmp( user, "user=admin" ) == 0) & (strcmp( pass, "pass=123" ) == 0) )
		{
			// �����
			char *content = "<html><body>������������</body><br/>"
				"<a href=\"/secret\">��������� ��������</a></html>";

			resp.addResponce( 200, "Ok", content );
			// ������ ����
			resp.addLine( "Set-Cookie: user=admin" );
			resp.WriteToSocket( sockio );
			sockio.writestr( content );
		}
		else
		{ // ������� - ������� ������
			char *content = "<html><body>����� ��� ������ �� ��������</body></html>";

			resp.addResponce( 403, "Access denied", content );
			resp.WriteToSocket( sockio );
			sockio.writestr( content );
		}

		return;
	}

	// ������ ������
	if (strcmp( req.url, "/logout" ) == 0)
	{
		char *content = "<html><body>����������</body></html>";

		resp.addResponce( 200, "Ok", content );

		// ������� ���� (�������� ����� ��� �����)
		resp.addLine( "Set-Cookie: user=deleted; path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT" );
		resp.WriteToSocket( sockio );
		sockio.writestr( content );

		return;
	}

	

