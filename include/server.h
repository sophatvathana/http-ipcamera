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

SERVERAPI bool SERVERCALL RunServer(int portNumber);

#ifdef __cplusplus
} // __cplusplus defined.
#endif
#endif