// webserver.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <process.h>
#include <locale.h>
#include <Windows.h>
#include "socketio.h"

	// Страница логина (после логина попадаем сюда и ставим куку)
	if (strcmp( req.url, "/login" ) == 0)
	{
		char formdata[100];

		// Читаем форму (метод - post)
		char *t = req.getField( "Content-Length" );
		int l;

		sscanf( t, "%d", &l );

		memset( formdata, 0, sizeof(formdata) );
		sockio.recvData( formdata, l );		

		// Разбиваем форму
		char *user, *pass;

		user = strtok_s( formdata, "&", &t );
		pass = strtok_s( NULL, "&", &t );

		// Проверяем параметры
		if ( (strcmp( user, "user=admin" ) == 0) & (strcmp( pass, "pass=123" ) == 0) )
		{
			// Успех
			char *content = "<html><body>Здравствуйте</body><br/>"
				"<a href=\"/secret\">секретная страница</a></html>";

			resp.addResponce( 200, "Ok", content );
			// Ставим куку
			resp.addLine( "Set-Cookie: user=admin" );
			resp.WriteToSocket( sockio );
			sockio.writestr( content );
		}
		else
		{ // Неуспех - выводим ошибку
			char *content = "<html><body>Логин или пароль не опознаны</body></html>";

			resp.addResponce( 403, "Access denied", content );
			resp.WriteToSocket( sockio );
			sockio.writestr( content );
		}

		return;
	}

	// Окошко вывода
	if (strcmp( req.url, "/logout" ) == 0)
	{
		char *content = "<html><body>Досвиданья</body></html>";

		resp.addResponce( 200, "Ok", content );

		// Стираем куку (устарела много лет назад)
		resp.addLine( "Set-Cookie: user=deleted; path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT" );
		resp.WriteToSocket( sockio );
		sockio.writestr( content );

		return;
	}

	

