// webserver.c: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <process.h>
#include <locale.h>
#include <Windows.h>
#include "socketio.h"

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

