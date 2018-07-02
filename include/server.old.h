#ifndef SERVER_H
#define SERVER_H
#define DEF_MAXWEBQUEUE 10
#ifdef _MSC_VER
#define SERVER_EXPORTS

#ifdef SERVER_EXPORTS
#define SERVERAPI __declspec(dllexport)
#else
#define SERVERAPI __declspec(dllimport)
#endif

#define SERVERCALL __cdecl
#else
#define SERVERAPI
#define SERVERCALL
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct  HttpClient
{
	unsigned char* ptr;
	int ref;
	long size;
};

static const struct 
{
	const char* dotType;
	const char* mimeType;
} MimeType[] = {
	{".jpg", "image/jpg"},
	{".jpeg", "image/jpeg"},
	{".html", "text/html"}
};

SERVERAPI bool SERVERCALL RunServer(int portNumber);

int net_tcp_connect(char *host, unsigned long port);
int net_udp_connect(char *host, unsigned long port);
int net_unix_sock(const char *path);
int net_sock_nonblock(int sockfd);
int net_sock_cork(int fd, int state);
int net_send16(int fd, uint16_t n);
int net_send32(int fd, uint32_t n);

#ifdef __cplusplus
} // __cplusplus defined.
#endif
#endif