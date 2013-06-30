#include "fmberryd.h"
#include "rpi_pin.h"
#include "ns741.h"

#include <poll.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <confuse.h>
#include <sys/stat.h>
#include <netinet/in.h>

#define RPI_REVISION RPI_REV2

// RDS interrupt pin
int rdsint = 17;

// LED pin number
int ledpin = -1;

mmr70_data_t mmr70;

static cfg_t *cfg;
static volatile int run = 1;
static int start_daemon = 1;

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

	// check for non-daemon mode for debugging
	for(int i = 1; i < argc; i++) {
		if (str_is(argv[i], "nodaemon")) {
			start_daemon = 0;
			break;
		}
	}

	if (start_daemon) {
		//Init daemon, can be replaced with daemon() call
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
	}
	else {
		// open syslog for non-daemon mode
		openlog(argv[0],LOG_NOWAIT|LOG_PID,LOG_USER);
	}

	//Read configuration file
	cfg_opt_t opts[] =
	{
		CFG_INT("i2cbus", 1, CFGF_NONE),
		CFG_INT("frequency", 99800, CFGF_NONE),	    
		CFG_BOOL("stereo", 1, CFGF_NONE),
		CFG_BOOL("rdsenable", 1, CFGF_NONE),    
		CFG_BOOL("poweron", 1, CFGF_NONE),    
		CFG_BOOL("tcpbindlocal", 1, CFGF_NONE),  
		CFG_INT("tcpport", 42516, CFGF_NONE),    
		CFG_INT("txpower", 3, CFGF_NONE),    
		CFG_INT("rdspin", 17, CFGF_NONE),
		CFG_STR("rdsid", "", CFGF_NONE),
		CFG_STR("rdstext", "", CFGF_NONE),
		CFG_INT("ledpin", 27, CFGF_NONE),
		CFG_END()
	};
	
	cfg = cfg_init(opts, CFGF_NONE);
	if (cfg_parse(cfg, "/etc/fmberry.conf") == CFG_PARSE_ERROR)
		return 1;

	// get LED pin number
	int led = 1; // led state
	ledpin = cfg_getint(cfg, "ledpin");
	rdsint = cfg_getint(cfg, "rdspin");

	// Init I2C bus and transmitter
	if (ns741_init(cfg_getint(cfg, "i2cbus")) == -1)
	{
		syslog(LOG_ERR, "Init failed! Double-check hardware and try again!\n");
		exit(EXIT_FAILURE);
	}
	syslog(LOG_NOTICE, "Successfully initialized ns741 transmitter.\n");

	int nfds;
	struct pollfd  polls[2];
	
	// open TCP listener socket, will exit() in case of error
	int lst = ListenTCP(cfg_getint(cfg, "tcpport"));
	polls[0].fd = lst;
	polls[0].events = POLLIN;
	nfds = 1;

	// initialize data structure for 'status' command
	bzero(&mmr70, sizeof(mmr70));
	mmr70.frequency = cfg_getint(cfg, "frequency");
	mmr70.power     = cfg_getbool(cfg, "poweron");
	mmr70.txpower   = cfg_getint(cfg, "txpower");
	mmr70.mute      = 0;
	mmr70.stereo    = cfg_getbool(cfg, "stereo");
	mmr70.rds       = cfg_getbool(cfg, "rdsenable");
	strncpy(mmr70.rdsid, cfg_getstr(cfg, "rdsid"), 8);
	strncpy(mmr70.rdstext, cfg_getstr(cfg, "rdstext"), 64);
	
	// Set initial frequency and state.
	ns741_set_frequency(mmr70.frequency);
	ns741_txpwr(mmr70.txpower);

	if (mmr70.power)
	{
		ns741_power(1);
	}
	
	if (!mmr70.stereo)
		ns741_stereo(0);

	ns741_rds_set_progname(mmr70.rdsid);
	ns741_rds_set_radiotext(mmr70.rdstext);

	// Use RPI_REV1 for earlier versions of Raspberry Pi
	rpi_pin_init(RPI_REVISION);

	// Get file descriptor for RDS handler
	polls[1].revents = 0;
	if (mmr70.rds)
	{
		int rds = rpi_pin_poll_enable(rdsint, EDGE_FALLING);
	    if (rds < 0) {
	        printf("Couldn't enable RDS support\n");
	        run = 0;
	    }
		polls[1].fd = rds;
		polls[1].events = POLLPRI;
		nfds = 2;
		ns741_rds(1);
		if (ledpin > 0) {
			rpi_pin_export(ledpin, RPI_OUTPUT);
			rpi_pin_set(ledpin, led);
		}
		ns741_rds_start();
	}

	// main polling loop
	int ledcounter = 0;
	while(run) {
		if (poll(polls, nfds, -1) < 0)
			break;

		if (polls[1].revents) {
			rpi_pin_poll_clear(polls[1].fd);
			ProcessRDS();
			// flash LED if enabled on every other RDS refresh cycle
			if (ledpin > 0) {
				ledcounter++;
				if (!(ledcounter % 80)) {
					led ^= 1;
					rpi_pin_set(ledpin, led);
				}
			}
		}

		if (polls[0].revents)
			ProcessTCP(lst, &mmr70);
	}

	// clean up at exit
	ns741_power(0);
	if (mmr70.rds)
		rpi_pin_unexport(rdsint);

	if (ledpin > 0) {
		rpi_pin_set(ledpin, 0);
		rpi_pin_unexport(ledpin);
	}
	
	close(lst);
	closelog();

	return 0;
}

int ListenTCP(uint16_t port)
{
	/* Socket erstellen - TCP, IPv4, keine Optionen */
	int lsd = socket(AF_INET, SOCK_STREAM, 0);
 
	/* IPv4, Port: 1111, jede IP-Adresse akzeptieren */
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
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
	if (bind(lsd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0) {
  		//whoops. Could not listen
  		syslog(LOG_ERR, "Could not bind to TCP port! Terminated.\n");
  		exit(EXIT_FAILURE);
	 }

  	syslog(LOG_NOTICE, "Successfully started daemon\n");
	/* Auf Socket horchen (Listen) */
	listen(lsd, 10);
	
	return lsd;
}

// for 'status' command
static float txpower[4] = { 0.5, 0.8, 1.0, 2.0 };

int ProcessTCP(int sock, mmr70_data_t *pdata)
{
	/* Puffer und Strukturen anlegen */
	struct sockaddr_in clientaddr;
	socklen_t clen = sizeof(clientaddr);
	char buffer[512];
	bzero(buffer, sizeof(buffer));
 	
	/* Auf Verbindung warten, bei Verbindung Connected-Socket erstellen */
	int csd = accept(sock, (struct sockaddr *)&clientaddr, &clen);

	struct pollfd  pol;
	pol.fd = csd;
	pol.events = POLLRDNORM;

	// just to be on a safe side check if data is available
	if (poll(&pol, 1, 1000) <= 0) {
		close(csd);
		return -1;
	}

	int len  = recv(csd, buffer, sizeof(buffer) - 2, 0);
	buffer[len] = '\0';
	char *end = buffer + len - 1;
	// remove any trailing spaces
	while((end != buffer) && (*end <= ' ')) {
		*end-- = '\0';
	}

	do {
		const char *arg;
	
		if (str_is_arg(buffer, "set freq", &arg))
		{
			int frequency = atoi(arg);

			if ((frequency >= 76000) && (frequency <= 108000))
			{
				syslog(LOG_NOTICE, "Changing frequency...\n");
				ns741_set_frequency(frequency);
				pdata->frequency = frequency;
			}
			else
			{
				syslog(LOG_NOTICE, "Bad frequency.\n");
			}
			break;
		}

		if (str_is(buffer, "poweroff"))
		{
			ns741_power(0);
			pdata->power = 0;
			break;
		}

		if (str_is(buffer, "poweron"))
		{
			ns741_power(1);
			ns741_rds_start();
			pdata->power = 1;
			break;
		}

		if (str_is(buffer, "muteon"))
		{
			ns741_mute(1);
			pdata->mute = 1;
			break;
		}

		if (str_is(buffer, "muteoff"))
		{
			ns741_mute(0);
			pdata->mute = 0;
			break;
		}

		if (str_is_arg(buffer, "set stereo", &arg))
		{
			if (str_is(arg, "on"))
			{
				syslog(LOG_NOTICE, "Enabling stereo signal...\n");
				ns741_stereo(1);
				pdata->stereo = 1;
				break;
			}
			if (str_is(arg, "off"))
			{
				syslog(LOG_NOTICE, "Disabling stereo signal...\n");
				ns741_stereo(0);
				pdata->stereo = 0;
			}
			break;
		}

		if (str_is_arg(buffer, "set txpwr", &arg))
		{
			int txpwr = atoi(arg);

			if ((txpwr >= 0) && (txpwr <= 3))
			{
				syslog(LOG_NOTICE, "Changing transmit power...\n");
				ns741_txpwr(txpwr);
				pdata->txpower = txpwr;
			}
			else
			{
				syslog(LOG_NOTICE, "Bad transmit power. Range 0-3\n");
			}
			break;
		}

		if (str_is_arg(buffer, "set rdstext", &arg))
		{
			strncpy(pdata->rdstext, arg, 64);
			ns741_rds_set_radiotext(pdata->rdstext);
			break;
		}

		if (str_is_arg(buffer, "set rdsid", &arg))
		{
			bzero(pdata->rdsid, sizeof(pdata->rdsid));
			strncpy(pdata->rdsid, arg, 8);
			// ns741_rds_set_progname() will pad rdsid with spaces if needed
			ns741_rds_set_progname(pdata->rdsid);
			break;
		}

		if (str_is(buffer, "die") || str_is(buffer, "stop"))
		{
			run = 0;
			syslog(LOG_NOTICE, "Shutting down.\n");
			break;
		}

		if (str_is(buffer, "status"))
		{
			bzero(buffer, sizeof(buffer));
			sprintf(buffer, "freq: %dKHz txpwr: %.2fmW power: '%s' mute: '%s' stereo: '%s' rds: '%s' rdsid: '%s' rdstext: '%s'\n", 
				pdata->frequency,
				txpower[pdata->txpower],
				pdata->power ? "on" : "off",
				pdata->mute ? "on" : "off",
				pdata->stereo ? "on" : "off",
				pdata->rds ? "on" : "off",
				pdata->rdsid, pdata->rdstext);
			write(csd, buffer, strlen(buffer) + 1);
			break;
		}

	} while(0);
 
	close(csd);
	return 0;
}

// helper string compare functions
int str_is(const char *str, const char *is)
{
	if (strcmp(str, is) == 0)
		return 1;
	return 0;
}

int str_is_arg(const char *str, const char *is, const char **arg)
{
	size_t len = strlen(is);
	if (strncmp(str, is, len) == 0) {
		str = str + len;
		// remove any leading spaces from the arg
		while(*str && (*str <= ' '))
			str++;
		*arg = str;
		return 1;
	}
	return 0;
}
