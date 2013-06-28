#ifndef __FMBERRYD_H_FMBERRY_
#define __FMBERRYD_H_FMBERRY_

#include "ns741.h"
#include <sys/types.h>    // für socket()
#include <sys/socket.h>   // für socket()
#include <netinet/in.h>   // für socket()
#include <assert.h>       // für assert()
#include <stdlib.h>       // für calloc()
#include <netdb.h>        // für getprotobyname()
#include <unistd.h>       // für close()
#include <arpa/inet.h>    // für inet_ntop()
#include <netinet/in.h>   // für INET_ADDRSTRLEN
#include <string.h>       // für memset()
#include <stdio.h>
#include <confuse.h>

typedef struct _mmr70_data
{
	int frequency;
	int power;
	int mute;
	int txpower;
	int rds;
	char rdsid[10];   // max length of id is 8, but need spare null for printing
	char rdstext[66]; // same here, max 64 chars, 2 nulls are extra (need only 1, but I do not like odd numbers)
}mmr70_data_t;

extern mmr70_data_t mmr70;

int main(int argc, char **argv);

// true if string is equal to the second argument
int str_is(const char *str, const char *is);
int str_is_arg(const char *str, const char *is, const char **arg);

void *TransmitRDS();
int ListenTCP(uint16_t port);
int ProcessTCP(int sock, mmr70_data_t *pdata);
#endif
