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

	// Остальные страницы (не специальные) - это просто файл
	FILE *f = fopen( fname, "rb" );

	// А есть ли такой файл?
	if (f == NULL)
	{
		sprintf( content, "<html><body>File %s not found</html></body>", fname );

		resp.addResponce( 404, "not found", content );
		resp.WriteToSocket( sockio );
		sockio.writestr( content );

		return;
	}

	// Что за файл (картинка или хтмл)
	if (strstr( fname, ".htm" ) != NULL)
		resp.contentType = "text/html";
	else if (strstr( fname, ".jp" ) != NULL)
		resp.contentType = "image/jpeg";

	fseek( f, 0, SEEK_END );
	resp.addResponce( 200, "OK", ftell(f) );
	resp.WriteToSocket( &sockio );

	fseek( f, 0, SEEK_SET );

	unsigned char buf[16384];
	int r;

	// Блоками перекиыдваем файл
	while (!feof(f))
	{
		r = fread( &buf[0], 1, sizeof(buf), f );
		sockio.sendData( (char*) buf, r, 0 );

		if (r != sizeof(buf))
			break;
	}

	fclose(f);

	// Все
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale ( LC_ALL, "Russian" );

    printf( "Запускаем сервер\n" );

	InitializeSSL();

    SOCKET server;
    sockaddr_in local;

    WSADATA wsaData;
	int wsaret=WSAStartup(0x101,&wsaData);
	if(wsaret!=0) return 1;

    local.sin_family=AF_INET; 
    local.sin_addr.s_addr=INADDR_ANY; 
    local.sin_port=htons((u_short)443); // HTTPS port

    if((server=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) ==INVALID_SOCKET) return 1;			

    if(bind(server,(sockaddr*)&local,sizeof(local))!=0) return 1;

    if(listen(server,10)!=0) return 1;

    sockaddr_in from;
    int fromlen=sizeof(from);

	printf( "Запустили, слушаем\n" );

	SOCKET sock;

    while(true)
    { // Пришло новое соединение
		sock = accept(server, (struct sockaddr*)&from,&fromlen);
        printf( "Соединение от %s\n", inet_ntoa(from.sin_addr) );

		// Запуск цепочки
		_beginthread( SocketThread, 0, (void*) sock );		
    }

    //closesocket() closes the socket and releases the socket descriptor
    closesocket(server);

    //originally this function probably had some use
    //currently this is just for backward compatibility
    //but it is safer to call it as I still believe some
    //implementations use this to terminate use of WS2_32.DLL 
    WSACleanup();

	return 0;
}

