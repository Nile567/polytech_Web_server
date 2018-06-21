// webserver.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <process.h>
#include <locale.h>
#include <Windows.h>
#include "socketio.h"

// Рабочая цепочка
void SocketThread( void *data )
{
	// Соединяемся
	TSocketIO sockio( (SOCKET) data, true );
	THttpHeader req, resp;

	// Читаем параметры
	req.ReadFromSocket( sockio );
	req.parseReq();

	// Выводим для отладки
	printf( "Method %s url %s (%s)\n", req.method, req.url, req.httpver );

	char content[1000];

	// Какая-то ошибка
	if ( (req.method == NULL) ) // || (strcmp( req.method, "GET" ) != 0) )
	{
		sprintf( content, "<html><body>unsupported</body></html>" );

		resp.addResponce( 500, "Unsupported method", content );
		resp.WriteToSocket( sockio );
		sockio.writestr( content );

		return;
	}

	// Страница по умолчанию
	if (strcmp( req.url, "/" ) == 0)
		req.url = "/index.html";

	char fname[100];
	sprintf( fname, "www\\%s", &req.url[1] );

	// Страница с тестом куков
	if (strcmp( req.url, "/cookie" ) == 0)
	{
		// Ищем куку
		char *t = req.getCookie( "cnt" );
		int cnt = 0;

		// Если есть - читаем из нее число
		if (t != NULL)
			sscanf( t, "%d", &cnt );

		cnt++;

		// Страница (генерируем на сервере)
		sprintf( content, "<html><body>Тест куков<br/>"
			"Вы были на этой странице %d раз</body></html>", cnt );

		resp.addResponce( 200, "OK", content );
		// Записываем куку обратно
		resp.addLine( "Set-Cookie: cnt=%d", cnt );
		resp.WriteToSocket( sockio );
		sockio.writestr( content );

		return;
	}

	// Секретная страница
	if (strcmp( req.url, "/secret" ) == 0)
	{
		// Нужна кука user
		char *t = req.getCookie( "user" );

		// Если ее нет - ошибка
		if (t == NULL)
		{
			char *content = "<html><body><h1>доступ запрещен</h1></body></html>";

			resp.addResponce( 403, "access denied", content );
			resp.WriteToSocket( sockio );
			sockio.writestr( content );

			return;
		}

		// Выводим тайную информацию
		char content[1000];
		sprintf( content, "<html><body><h1>Секретный раздел</h1>Привет %s<br/>"
			"<a href=\"/logout\">выход</a></body></html>", t );

		resp.addResponce( 200, "Ok", content );
		resp.WriteToSocket( sockio );
		sockio.writestr( content );

		return;
	}
