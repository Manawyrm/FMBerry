#include "fmberryd.h"
#include <stdlib.h>
#include <syslog.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
/*
FMBerry - an cheap and easy way of transmitting music with your Pi.
Written 2013 by Tobias Mädel (t.maedel@alfeld.de)

Thanks to Rsoft for helping me preserving my sanity in spite of my non-existant knowledge of C (writing of Makefile and C-Headers)
Thanks to Paul Griffiths (1999) for his TCP socket helper.
*/

int main(int argc, char **argv)
{
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

	ListenTCP();

}


int ListenTCP()
{
	/* Socket erstellen - TCP, IPv4, keine Optionen */
	int lsd = socket(AF_INET, SOCK_STREAM, 0);
 
	/* IPv4, Port: 1111, jede IP-Adresse akzeptieren */
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT);
	saddr.sin_addr.s_addr = htons(INADDR_ANY);
 
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