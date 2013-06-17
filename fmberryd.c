#include "fmberryd.h"
#include <stdlib.h>
#include <syslog.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

/*
FMBerry - an cheap and easy way of transmitting music with your Pi.
Written 2013 by Tobias Mädel (t.maedel@alfeld.de)

Versions: 
06.06.2013 - Added RDS Support
07.06.2013 - Added configuration file

Thanks to Rsoft for helping me preserving my sanity in spite of my non-existant knowledge of C (writing of Makefile and C-Headers)
Thanks to Paul Griffiths (1999) for his TCP socket helper.
*/

int main(int argc, char **argv)
{
	//Check if user == root
	if(geteuid() != 0)
	{
	  puts("Please run this software as root!");
	  exit(EXIT_FAILURE);
	}

	//Init deamon
	pid_t pid;

	pid = fork();
	if (pid < 0)
	{
		exit(EXIT_FAILURE);
	}
	if (pid > 0)
	{
		exit(EXIT_SUCCESS);
	}

	umask(0);

	//We are now running as the forked child process.
	openlog(argv[0],LOG_NOWAIT|LOG_PID,LOG_USER);

	//closelog();

	pid_t sid;
	sid = setsid();
	if (sid < 0)
	{
		syslog(LOG_ERR, "Could not create process group\n");
		exit(EXIT_FAILURE);
	}

	if ((chdir("/")) < 0) 
	{
		syslog(LOG_ERR, "Could not change working directory to /\n");
		exit(EXIT_FAILURE);
	}

	//Read configuration file
	cfg_opt_t opts[] =
	{
		CFG_INT("frequency", 99800, CFGF_NONE),	    
		CFG_BOOL("rdsenable", 1, CFGF_NONE),    
		CFG_BOOL("poweron", 1, CFGF_NONE),    
		CFG_BOOL("tcpbindlocal", 1, CFGF_NONE),  
		CFG_INT("tcpport", 42516, CFGF_NONE),    
		CFG_INT("txpower", 3, CFGF_NONE),    
		CFG_STR("rdsid", "", CFGF_NONE),
		CFG_STR("rdstext", "", CFGF_NONE),
		CFG_END()
	};
	
	cfg = cfg_init(opts, CFGF_NONE);
	if(cfg_parse(cfg, "/etc/fmberry.conf") == CFG_PARSE_ERROR)
		return 1;
	
	//close(STDIN_FILENO);
	//close(STDOUT_FILENO);
	//close(STDERR_FILENO);

	//Init I2C bus and transmitter
	if (ns741_init() == -1)
	{
		syslog(LOG_ERR, "Init failed! Double-check hardware and try again!\n");
		exit(EXIT_FAILURE);
	}
	syslog(LOG_NOTICE, "Successfully initialized ns741 transmitter.\n");
	
	//Set inital frequency and state.
	ns741_set_frequency(cfg_getint(cfg, "frequency"));
	ns741_txpwr(cfg_getint(cfg, "txpower"));


	if (cfg_getbool(cfg, "poweron"))
	{
		ns741_power(1);
	}
	
	ns741_rds_set_progname(cfg_getstr(cfg, "rdsid"));
	ns741_rds_set_radiotext(cfg_getstr(cfg, "rdstext"));


	//Create and Start Threads for RDS interrupt handler and TCP listener
	pthread_t RDSThread, TCPThread;

	int rc;
	if (cfg_getbool(cfg, "rdsenable"))
	{
		rc = pthread_create( &RDSThread, NULL, &TransmitRDS, NULL );
	    if( rc != 0 ) {
	        printf("Couldn't create RDS Thread.\n");
	        return EXIT_FAILURE;
	    }
	}
	

    rc = pthread_create( &TCPThread, NULL, &ListenTCP, NULL );
    if( rc != 0 ) {
        printf("Couldn't create TCP Thread.\n");
        return EXIT_FAILURE;
    }

    //Wait for both threads to end (which is not going to happen)
    if (cfg_getbool(cfg, "rdsenable"))
	{
   		pthread_join( RDSThread, NULL );
   	}

	pthread_join( TCPThread, NULL );
	return;
}
void *TransmitRDS(void *arg) 
{
    while (1)
    {
		// Read interrupt pin. (LOW when waiting for data.)
		uint8_t value = bcm2835_gpio_lev(RDSINT);
		if (value == 0)
		{
			RDSINT_vect();
		}
		delay(10);
    }
}


void *ListenTCP(void *arg)
{
	/* Socket erstellen - TCP, IPv4, keine Optionen */
	int lsd = socket(AF_INET, SOCK_STREAM, 0);
 
	/* IPv4, Port: 1111, jede IP-Adresse akzeptieren */
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(cfg_getint(cfg, "tcpport"));
	if (cfg_getbool(cfg, "tcpbindlocal"))
	{
		syslog(LOG_NOTICE, "Binding to localhost.\n");
    	saddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
	else
	{
		syslog(LOG_NOTICE, "Binding to any interface.\n");
    	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

	//Important! Makes sure you can restart the daemon without any problems! 
	const int       optVal = 1;
	const socklen_t optLen = sizeof(optVal);

	int rtn = setsockopt(lsd, SOL_SOCKET, SO_REUSEADDR, (void*) &optVal, optLen);

	//assert(rtn == 0);   /* this is optional */
 
	/* Socket an Port binden */
	if( bind(lsd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0) {
  		//whoops. Could not listen
  		syslog(LOG_ERR, "Could not bind to TCP port! Terminated.\n");
  		exit(EXIT_FAILURE);
	 }

  	syslog(LOG_NOTICE, "Successfully started daemon\n");
	/* Auf Socket horchen (Listen) */
	listen(lsd, 10);
 
	while(1) {
		/* Puffer und Strukturen anlegen */
		struct sockaddr_in clientaddr;
		char buffer[1000];
		bzero(buffer, sizeof(buffer));
 
		/* Auf Verbindung warten, bei Verbindung Connected-Socket erstellen */
		socklen_t clen = sizeof(clientaddr);
		int csd = accept(lsd, (struct sockaddr*) &clientaddr, &clen);
 
		/* Vom Client lesen und ausgeben */
		int bytes = recv(csd, buffer, sizeof(buffer), 0);
		char *cmd;
		

		cmd = "set freq";
		if (strncmp(buffer, cmd, strlen(cmd)) == 0)
		{
			int frequency;
			sscanf(buffer, "set freq %d", &frequency);

			if ((frequency >= 76000) && (frequency <= 108000))
			{
				syslog(LOG_NOTICE, "Changing frequency...\n");
				ns741_set_frequency(frequency);
			}
			else
			{
				syslog(LOG_NOTICE, "Bad frequency.\n");
			}
		}


		cmd = "poweroff";
		if (strncmp(buffer, cmd, strlen(cmd)) == 0)
		{
			ns741_power(0);
		}

		cmd = "poweron";
		if (strncmp(buffer, cmd, strlen(cmd)) == 0)
		{
			ns741_power(1);
		}

		cmd = "muteon";
		if (strncmp(buffer, cmd, strlen(cmd)) == 0)
		{
			ns741_mute(1);
		}

		cmd = "muteoff";
		if (strncmp(buffer, cmd, strlen(cmd)) == 0)
		{
			ns741_mute(0);
		}

		cmd = "set txpwr";
		if (strncmp(buffer, cmd, strlen(cmd)) == 0)
		{
			int txpwr;
			sscanf(buffer, "set txpwr %d", &txpwr);

			if ((txpwr >= 0) && (txpwr <= 3))
			{
				syslog(LOG_NOTICE, "Changing transmit power...\n");
				ns741_txpwr(txpwr);
			}
			else
			{
				syslog(LOG_NOTICE, "Bad transmit power. Range 0-3\n");
			}
		}

		cmd = "set rdstext";
		if (strncmp(buffer, cmd, strlen(cmd)) == 0)
		{
			ns741_rds_set_radiotext((buffer + 12));
			//printf((buffer + 12));
		}

		cmd = "set rdsid";
		if (strncmp(buffer, cmd, strlen(cmd)) == 0)
		{
			ns741_rds_set_progname((buffer + 10));
		}


		cmd = "die";
		if (strncmp(buffer, cmd, strlen(cmd)) == 0)
		{
			syslog(LOG_NOTICE, "Shutting down.\n");
			close(csd);
			close(lsd);
			exit(EXIT_SUCCESS);
		}
		
 		//printf("%s\n", buffer);
		/* Verbindung schließen */
		close(csd);
	}
 
	close(lsd);
	return EXIT_SUCCESS;
}