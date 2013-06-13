#ifndef __FMBERRYD_H_FMBERRY_
#define __FMBERRYD_H_FMBERRY_

#include "ns741.h"
#include <sys/types.h>    // für socket()
#include <sys/socket.h>   // für socket()
#include <netinet/in.h>   // für socket()
#include <assert.h>       // für assert()
#include <stdlib.h>       //für calloc()
#include <netdb.h>        // für getprotobyname()
#include <unistd.h>       // für close()
#include <arpa/inet.h>    //für inet_ntop()
#include <netinet/in.h>   //für INET_ADDRSTRLEN
#include <string.h>       // für memset()
#include <stdio.h>
#include <confuse.h>

cfg_t *cfg;
int main(int argc, char **argv);
void *TransmitRDS();
void *ListenTCP();
#endif
