/*  
 *
 * Additions (c) Stefan Rick (Max2Play), 2016 under the same license terms as Squeezelite
 *   - Added Communication with connected Squeezebox Server
 */

#include "squeezelite.h"

#if CONTROLSBS
int sendCLICommandPlayPauseSwitch_running = 0;
extern log_level loglevel;
extern u8_t pmac[6];

void sendCLICommandVolume(int localvol, in_addr_t slimproto_ip){
	int port = 9090;
	
	//sbscliport may be set from command line
	if(sbscliport)
		port = sbscliport;
		
	ssize_t n;
	struct sockaddr_in serv_addr;
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = slimproto_ip;
	serv_addr.sin_port = htons(port);
	
	int sockcli = socket(AF_INET, SOCK_STREAM, 0);

	set_nonblock(sockcli);
	set_nosigpipe(sockcli);

	if (connect_timeout(sockcli, (struct sockaddr *) &serv_addr, sizeof(serv_addr), 5) != 0) {
		LOG_DEBUG("Unable to connect to CLI with port %d on SB-Server", port);		
	}else{
		LOG_DEBUG("Successful connected to CLI on SB-Server");
	}
	char volume[100];					
	
	sprintf(volume, "%02x:%02x:%02x:%02x:%02x:%02x mixer volume %d\n", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5], localvol);
	
	LOG_DEBUG("Set Volume from ALSA: %02x:%02x:%02x:%02x:%02x:%02x mixer volume %d", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5], localvol);	
	
	n = send(sockcli,(u8_t *)volume, strlen(volume), MSG_NOSIGNAL);
	if (n <= 0) {		
		LOG_DEBUG("Failed writing to CLI-socket: %s", strerror(last_error()));
	}else{
		LOG_DEBUG("SUCCESS writing to CLI-socket: %zu", n);
	}
	closesocket(sockcli);
	return;
}

void sendCLICommandPower(int setpower, in_addr_t slimproto_ip){
	int port = 9090;
	
	//sbscliport may be set from command line
	if(sbscliport)
		port = sbscliport;
	
	ssize_t n;
	struct sockaddr_in serv_addr;

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = slimproto_ip;
	serv_addr.sin_port = htons(port);
	
	int sockcli = socket(AF_INET, SOCK_STREAM, 0);

	set_nonblock(sockcli);
	set_nosigpipe(sockcli);

	if (connect_timeout(sockcli, (struct sockaddr *) &serv_addr, sizeof(serv_addr), 5) != 0) {
		LOG_DEBUG("Unable to connect to CLI with port %d on SB-Server", port);		
	}else{
		LOG_DEBUG("Successful connected to CLI on SB-Server");
	}
	char clistring[100];					
	
	sprintf(clistring, "%02x:%02x:%02x:%02x:%02x:%02x power %d\n", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5], setpower);
	
	LOG_DEBUG("Set Power Status: %02x:%02x:%02x:%02x:%02x:%02x power %d", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5], setpower);	
	
	n = send(sockcli,(u8_t *)clistring, strlen(clistring), MSG_NOSIGNAL);
	if (n <= 0) {		
		LOG_DEBUG("Failed writing to CLI-socket: %s", strerror(last_error()));
	}else{
		LOG_DEBUG("SUCCESS writing to CLI-socket: %zu", n);
	}
	closesocket(sockcli);
	return;
}

void sendCLICommandPlayPauseSwitch(in_addr_t slimproto_ip){
	if(sendCLICommandPlayPauseSwitch_running == 1)
		return;
		
	int port = 9090;
	char server_reply[100];
	sendCLICommandPlayPauseSwitch_running = 1;
	
	//sbscliport may be set from command line
	if(sbscliport)
		port = sbscliport;
	
	ssize_t n;
	struct sockaddr_in serv_addr;

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = slimproto_ip;
	serv_addr.sin_port = htons(port);
	
	int sockcli = socket(AF_INET, SOCK_STREAM, 0);

	//set_nonblock(sockcli);
	//set_nosigpipe(sockcli);

	if (connect_timeout(sockcli, (struct sockaddr *) &serv_addr, sizeof(serv_addr), 5) != 0) {
		LOG_DEBUG("Unable to connect to CLI with port %d on SB-Server", port);		
	}else{
		LOG_DEBUG("Successful connected to CLI on SB-Server");
	}
	char clistring[100];					

	sprintf(clistring, "%02x:%02x:%02x:%02x:%02x:%02x mode ?\n", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
	
	LOG_DEBUG("Get current Player Status: %02x:%02x:%02x:%02x:%02x:%02x mode ?", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);	
	
	n = send(sockcli,(u8_t *)clistring, strlen(clistring), MSG_NOSIGNAL);
	if (n <= 0) {		
		LOG_DEBUG("Failed writing to CLI-socket: %s", strerror(last_error()));
	}else{
		LOG_DEBUG("SUCCESS writing to CLI-socket: %zu", n);		
		memset(server_reply, '0',sizeof(server_reply));
	    if ( (n = read(sockcli, server_reply, sizeof(server_reply)-1)) > 0)
	    { 
	      server_reply[n] = 0;      
	    }
	    LOG_DEBUG("Current Player Status: %s", server_reply);
	    
	    // Now Play or Stop
	    char needle[] = "mode play"; 
	    if(strstr(server_reply, needle)) {
	    	sprintf(clistring, "%02x:%02x:%02x:%02x:%02x:%02x mode stop\n", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
	    	n = send(sockcli,(u8_t *)clistring, strlen(clistring), MSG_NOSIGNAL);
	    	LOG_DEBUG("Player Stopped...");
	    }else{
	        sprintf(clistring, "%02x:%02x:%02x:%02x:%02x:%02x mode play\n", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
	    	n = send(sockcli,(u8_t *)clistring, strlen(clistring), MSG_NOSIGNAL);
	    	LOG_DEBUG("Player Started...");
	    }
	}
	closesocket(sockcli);
	
	// Block multiple calls!
	sendCLICommandPlayPauseSwitch_running = 0;
	return;
}

#endif