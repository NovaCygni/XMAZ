#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>

#include <netdb.h>

#include <signal.h>

#include <curses.h>

#define DATA_SIZE 64
#define MESSAGE_SIZE 32
#define PORT_IN 1307
#define PORT_OUT PORT_IN
#define IP_SIZE 4*4
#define SECRET_SIZE 66
#define PUBKEY_SIZE 133

#include "packet.h"
#include "partner.h"
#include "ecdhe.h"

char endAll=0;

void end(int param){
	endAll=1;
}

int hostname2ip(char * hostname , char* ip)
{
 struct addrinfo *result, *rp, hints;

 memset(&hints, 0, sizeof(hints));
 hints.ai_socktype = SOCK_STREAM; // TCP

 int tmp = getaddrinfo(hostname, NULL, &hints, &result);
 if (tmp != 0) {
  fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(tmp));
  return 1;
 }

 for (rp = result;rp != NULL;rp = rp->ai_next) {
  char buf[IP_SIZE];
	for(int i=0; i<IP_SIZE; i++)
		buf[i]=0;
   struct in_addr* a4 = & ((struct sockaddr_in*) rp->ai_addr)->sin_addr;
   inet_ntop(rp->ai_family, a4, buf, sizeof(buf));
	for(int i=0; i<IP_SIZE; i++)
		ip[i]=buf[i];
 }

 freeaddrinfo(result);
 return 0;
}

int exchangeKeys(int sock_fd, struct sockaddr_in *peer, Partner *partner)
{
  int ret;
  ssize_t bytes;
  unsigned char input_buffer[PUBKEY_SIZE];
	char output_message[MESSAGE_SIZE];
  struct pollfd fds[2];

  fds[0].fd = 0;
  fds[1].fd = sock_fd;
  fds[0].events = POLLIN | POLLPRI;
  fds[1].events = POLLIN | POLLPRI;

	int ok=1;
	printf("Generating keys...\n");
	ECDHE ecdhe;
	newECDHE(&ecdhe);
	unsigned char pubKey[PUBKEY_SIZE];
	getPubKey(ecdhe.keys,pubKey);
	printf("Your public key : ");
	for(int i=0; i<PUBKEY_SIZE; i++)
		printf("%02x:",pubKey[i]);
	printf("\n");
	unsigned char secret[SECRET_SIZE];
	printf("Waiting for partners public key...\n");
	printf("Note that this step is most vulnerable and you have to trust/verify the partners public key\n");
	printf("Press any key to try to send your public key to partner\n");
	int sended=0;
  while (1) {
    ret = poll(fds, 2, -1);

    if (ret < 0) {
      printf("Error - poll returned error: %s\n", strerror(errno));
      break;
    }
    if (ret > 0) {

      if (fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
        printf("Error - poll indicated stdin error\n");
        break;
      }
      if (fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) {
        printf("Error - poll indicated socket error\n");
        break;
      }

      if (fds[0].revents & (POLLIN | POLLPRI)) {
	if(ok){
	fgets(output_message,sizeof(output_message),stdin);
        bytes = sendto(sock_fd, pubKey, PUBKEY_SIZE, 0,
                       (struct sockaddr *)peer, sizeof(struct sockaddr_in));
        if (bytes < 0) {
          printf("Error - sendto error: %s\n", strerror(errno));
          break;
        }
	sended=1;
	printf("Wait for transmission, if too long, then exit and try another time\n");
/*	char ok=0;
	for(int i=0; i<11; i++){
		sleep(1);
		switch(status){
			case 0:
				ok=1;
			break;
			case 1:
				printf("RESENDING DATA\n");
				bytes = sendto(sock_fd, output_buffer, DATA_SIZE, 0,
	                       (struct sockaddr *)peer, sizeof(struct sockaddr_in));
			        if (bytes < 0) {
				          printf("Error - sendto error: %s\n", strerror(errno));
				          break;
			        }
			break;
			default:
				printf("WAITING FOR ACK...\n");
		}
		if(ok)
			break;
	}
*/
	ok=0;
      }
	else return 4;
}

      if (fds[1].revents & (POLLIN | POLLPRI)) {
	ok=1;
        bytes = recvfrom(sock_fd, input_buffer, sizeof(input_buffer),
                         0, NULL, NULL);
	const unsigned char *cnstBuff=input_buffer;
        if (bytes < 0) {
          printf("Error - recvfrom error: %s\n", strerror(errno));
          break;
        }
        if (bytes > 0) {
		printf("RECEIVED PARTNERS PUBLIC KEY : ");
		for(int i=0; i<PUBKEY_SIZE; i++)
			printf("%02x:",input_buffer[i]);
		printf("\n");
		if(!sended){
		printf("SENDING THE PARTNER YOUR PUBLIC KEY...\n");
	        bytes = sendto(sock_fd, pubKey, PUBKEY_SIZE, 0,
	                       (struct sockaddr *)peer, sizeof(struct sockaddr_in));
	        if (bytes < 0) {
	          printf("Error - sendto error: %s\n", strerror(errno));
	          break;
        	}
		}
		printf("COMPUTING SECRET...\n");
		computeSecret(ecdhe.keys,cnstBuff,secret);
/*
		printf("SECRET : ");
		for(int i=0; i<SECRET_SIZE; i++)
			printf("%02x",secret[i]);
		printf("\n");
*/
		cnstBuff=NULL;
		for(int i=0; i<DATA_SIZE; i++){
			partner->key[i]=secret[i+2];
			secret[i]=0;
		}
		secret[64]=0;
		secret[65]=0;
		ECDHE_free(ecdhe.keys);
		nextKey(partner);
		nextKey(partner);
		update(partner);
		printf("SECRET ESTABLISHED\n");
		return 0;
	}
	}
      }
    }
	return 4;
  }

int chat(int sock_fd, struct sockaddr_in *peer, Partner *partner)
{
	int name=0;
	int my=0;
	int your=1;
	int y=1;
	int j=0;
	int ch;
	int prex;
	int prey;
	char *self="MESSAGE";
	WINDOW *chat, *input, *status;
        initscr();
	signal(SIGINT,end);
	start_color();
	init_pair(1,COLOR_GREEN,COLOR_BLACK);
	init_pair(2,COLOR_YELLOW,COLOR_BLACK);
	init_pair(3,COLOR_RED,COLOR_BLACK);
        clear();
        chat=newwin(LINES-3, COLS, 0, 0);
	status=newwin(3,COLS-(MESSAGE_SIZE+strlen(self)+4+3), LINES-3,MESSAGE_SIZE+strlen(self)+4+3);
        input=newwin(3, MESSAGE_SIZE+strlen(self)+4+3, LINES-3, 0);
        box(chat, 0, 0);
	box(status,0,0);
        box(input, 0, 0);
        halfdelay(1);
        	wattron(status, A_BOLD);
        	mvwprintw(status, 1, 1, "STATUS : Ready, be sure that the other side is ready as well, type Ctrl-C to exit");
        	wattroff(status, A_BOLD);
		wrefresh(status);

  ssize_t bytes;
  unsigned char input_buffer[DATA_SIZE];
  unsigned char output_buffer[DATA_SIZE];
        char input_message[MESSAGE_SIZE+1];
        char output_message[MESSAGE_SIZE+1];

//unsigned char for file transfer

  struct pollfd fds[2];
//otherwise poll will bug

	int ret;
  fds[0].fd = 0;
  fds[0].events = POLLIN | POLLPRI;
  fds[1].fd = sock_fd;
  fds[1].events = POLLIN | POLLPRI;
	char received=1;
	char answered=0;
        do
        {
                wattron(input, A_BOLD);
                mvwprintw(input, 1, MESSAGE_SIZE+strlen(self)+4, ":");
                mvwprintw(input, 1, MESSAGE_SIZE+strlen(self)+4+1, ":");
                mvwprintw(input, 1, 1, "%s : ", self);
                wattroff(input, A_BOLD);
                wrefresh(chat);
                wrefresh(input);
                wmove(input, 1, strlen(self)+4);
                j=0;
                while(1)
                {
                                getyx(input, prey, prex);
                wattron(input, A_BOLD);
                mvwprintw(input, 1, MESSAGE_SIZE+strlen(self)+4, ":");
                mvwprintw(input, 1, MESSAGE_SIZE+strlen(self)+4+1, ":");
                wattroff(input, A_BOLD);
                wmove(input, prey, prex);
                wrefresh(input);
			ret=poll(fds,2,-1);
			if(ret>0){
			if (fds[1].revents & (POLLIN | POLLPRI)){
				answered=1;
			}
			}
                        box(input, 0, 0);
                        box(chat, 0, 0);
			box(status,0,0);
			if(endAll){
			       endwin();
			        for(int i=0;i<MESSAGE_SIZE;i++){
			                output_message[i]=0;
			                input_message[i]=0;
			        }
			        for(int i=0;i<DATA_SIZE;i++){
			                input_buffer[i]=0;
			                output_buffer[i]=0;
			        }
			        return 0;
			}
//BEGIN OK
			if(received){
			ch=wgetch(input);
                                wmove(chat, LINES-6, 0);
                                wclrtoeol(chat);
                        if(ch=='\n')
                                break;
			if((j>=MESSAGE_SIZE)&&(ch!=127)){
				j--;
				ch=127;
			}
                        if((ch==127)&&(j!=0))
                        {
                                j--;
                                getyx(input, prey, prex);
                                if(prex>(strlen(self)+3))
                                {
                                        mvwdelch(input, prey, prex-3);
                                        mvwdelch(input, prey, prex-3);
                                        mvwdelch(input, prey, prex-3);
                                        wclrtoeol(input);
                                }
                        }

                        else if((ch==127)&&(j==0))
			{
                                getyx(input, prey, prex);
                                mvwdelch(input, prey, prex-2);
                                mvwdelch(input, prey, prex-2);
                                wclrtoeol(input);
                        }

                        if((ch!=-1)&&(ch!=127))
                                output_message[j++]=ch;
			}
//END OK//
                        if(answered==1)
                        {
			unsigned char ack=0;
////////RETRIVING///////
received=1;
        bytes = recvfrom(sock_fd, input_buffer, sizeof(input_buffer),
                         0, NULL, NULL);
        if (bytes < 0) {
          printf("Error - recvfrom error: %s\n", strerror(errno));
          break;
        }
        if (bytes > 0) {
        ack=deformPacket(input_buffer,input_message,partner);
        switch(ack){
        case 0:
                partner->flags=1;
                formPacket(output_message,output_buffer,partner);
                for(int i=0;i<DATA_SIZE;i++)
                        partner->prevData[i]=output_buffer[i];
                update(partner);
//output_buffer[13]=7;
                bytes = sendto(sock_fd, output_buffer, DATA_SIZE, 0,
                               (struct sockaddr *)peer, sizeof(struct sockaddr_in));
if (bytes < 0) {
                  printf("Error - sendto error: %s\n", strerror(errno));
                  break;
                }
        break;
        case 1:
		wclear(status);
		box(status,0,0);
        	wattron(status, A_BOLD);
        	wattron(status, COLOR_PAIR(1));
        	mvwprintw(status, 1, 1, "STATUS : Sended");
        	wattroff(status, COLOR_PAIR(1));
        	wattroff(status, A_BOLD);
        	wrefresh(status);
		wrefresh(input);
                wmove(input, 1, strlen(self)+4);
        break;
        case 2:
//                printf("Transmission corruption detected, reACKing...\n");
		wclear(status);
		box(status,0,0);
        	wattron(status, A_BOLD);
        	wattron(status, COLOR_PAIR(3));
        	mvwprintw(status, 1, 1, "STATUS : Transmission re-established after corruption detection");
        	wattroff(status, COLOR_PAIR(3));
        	wattroff(status, A_BOLD);
        	wrefresh(status);
		wrefresh(input);
                wmove(input, 1, strlen(self)+4);
                for(int i=0;i<DATA_SIZE;i++)
                        output_buffer[i]=partner->prevData[i];
                bytes = sendto(sock_fd, output_buffer, DATA_SIZE, 0,
                               (struct sockaddr *)peer, sizeof(struct sockaddr_in));
                if (bytes < 0) {
                  printf("Error - sendto error: %s\n", strerror(errno));
                  break;
                }
        break;
        case 3:
//                printf("Received Previous ACK! Transmission corrupted! Quiting... Try another time later\n");
//        fgets(output_message,sizeof(output_message),stdin);
        endwin();
	for(int i=0;i<MESSAGE_SIZE;i++){
        	input_message[i]=0;
        	output_message[i]=0;
	}
	for(int i=0;i<DATA_SIZE;i++){
  		input_buffer[i]=0;
  		output_buffer[i]=0;
	}
                return 1;
        break;
        default:
//                printf("Bad Packet Received! Quiting... Try another time later\n");
//        fgets(output_message,sizeof(output_message),stdin);
        endwin();
	for(int i=0;i<MESSAGE_SIZE;i++){
        	input_message[i]=0;
        	output_message[i]=0;
	}
	for(int i=0;i<DATA_SIZE;i++){
  		input_buffer[i]=0;
  		output_buffer[i]=0;
	}
                return 1;
        }
      }
///////END RETRIVING/////
				answered=0;
				if(ack!=1){
                                wattron(chat, A_BOLD);
				if(name!=your){
					mvwaddstr(chat, y++, 2, "\n");
					mvwaddstr(chat, y, 2, "<in> ");
					mvwaddstr(chat, y++, 2+5, partner->name);
				}
                                wattroff(chat, A_BOLD);
				input_message[MESSAGE_SIZE]=0;
                                mvwprintw(chat, y++, 2, "%s", input_message);
                                box(chat, 0, 0);
                                wrefresh(chat);
                                wrefresh(input);
                                name=your;
				}
                        }
                        if(y>LINES-6)
                        {
                                wclear(chat);
                                box(chat, 0, 0);
                                wrefresh(chat);
                                y=1;
                        }
                }
		output_message[j]=0;
                if(j>0)
                {
                        wattron(chat, A_BOLD);
                        if(name!=my)
                        {
				mvwaddstr(chat, y++, 2, "\n");
				mvwaddstr(chat, y++, 2, "<out> ");
                        }
                        wattroff(chat, A_BOLD);
                        mvwaddstr(chat, y++, 2, output_message);
                        box(chat, 0, 0);
                        wmove(input, 1, 4);
                        wclrtoeol(input);
                        name=my;
//////////SENDING/////////
if(received){
        partner->flags=0;
        formPacket(output_message,output_buffer,partner);
//output_buffer[13]=7;
        bytes = sendto(sock_fd, output_buffer, DATA_SIZE, 0,
                       (struct sockaddr *)peer, sizeof(struct sockaddr_in));
        if (bytes < 0) {
          printf("Error - sendto error: %s\n", strerror(errno));
          break;
        }
        received=0;
	wclear(status);
	box(status,0,0);
        wattron(status, A_BOLD);
        wattron(status, COLOR_PAIR(2));
        mvwprintw(status, 1, 1, "STATUS : Sending...");
        wattroff(status, COLOR_PAIR(2));
        wattroff(status, A_BOLD);
        wrefresh(status);
        wrefresh(input);
        wmove(input, 1, strlen(self)+4);
        }
        else{
//                printf("Transmission corrupted! Quiting... Try another time later\n");
///        fgets(output_message,sizeof(output_message),stdin);
        endwin();
	for(int i=0;i<MESSAGE_SIZE;i++){
        	input_message[i]=0;
        	output_message[i]=0;
	}
	for(int i=0;i<DATA_SIZE;i++){
  		input_buffer[i]=0;
  		output_buffer[i]=0;
	}
                return 1;
	}
                }
///////////END SEND/////////
        }while(1);
        endwin();
	for(int i=0;i<MESSAGE_SIZE;i++){
		output_message[i]=0;
        	input_message[i]=0;
	}
	for(int i=0;i<DATA_SIZE;i++){
  		input_buffer[i]=0;
  		output_buffer[i]=0;
	}
	return 0;
}

int main()
{
	int number=partnersNumber();
	Partner partner;
	char restart=1;
	char input[DATA_SIZE];
	int choice=2;
	printf("******* WELCOME *******\n");
	while(restart){
	if(number>=0){
		printf("Choose an action to perform by number :\n");
		printf("0 - Start communication with someone\n");
		printf("1 - Change information of someone\n");
		printf("2 - Add someone to communicate\n");
//		printf("X - Wait for communication from someone\n");
		for(int i=0;i<DATA_SIZE;i++)
			input[i]=0;
		fgets(input,sizeof(input),stdin);
		choice=atoi(input);
	}
	switch(choice){
		case 0:
	printf("Choose someone by number :\n");
	for(int i=0; i<=number;i++){
		getPartner(&partner,i);
		printf("%d - %s\n",i,partner.name);
	}
	fgets(input,sizeof(input),stdin);
	choice=atoi(input);
	if(choice>=0 && choice<=number)
		setPartner(&partner,choice);
	else
		printf("Wrong selection!\n");
/*
	printf("You choosed %s\n",partner.name);
	printf("IP : %s\n",partner.ip);
	printf("Flags : %d\n",partner.flags);
	printf("CURRENT KEY : ");
	for(int i=0;i<DATA_SIZE;i++)
		printf("%02x",partner.key[i]);
	printf("\n");
	printf("PREVIOUS KEY : ");
	for(int i=0;i<DATA_SIZE;i++)
		printf("%02x",partner.prevKey[i]);
	printf("\n");
	printf("PREVIOUS DATA : ");
	for(int i=0;i<DATA_SIZE;i++)
		printf("%02x",partner.prevData[i]);
	printf("\n");
*/
		restart=0;
		break;
		case 1:
	printf("Choose someone by number :\n");
	for(int i=0; i<=number;i++){
		getPartner(&partner,i);
		printf("%d - %s\n",i,partner.name);
	}
	fgets(input,sizeof(input),stdin);
	choice=atoi(input);
	if(choice>=0 && choice<=number)
		setPartner(&partner,choice);
	else
		printf("Wrong selection!\n");
	printf("You choosed %s\n",partner.name);
	printf("Choose what to change by number :\n");
	printf("0 - Name\n");
	printf("1 - IP\n");
//	printf("X - Delete information\n");
	fgets(input,sizeof(input),stdin);
	choice=atoi(input);
	for(int i=0;i<DATA_SIZE;i++)
		input[i]=0;
	switch(choice){
		case 0:
			printf("Enter new name : ");
			fgets(input,sizeof(input)-1,stdin);
			strtok(input,"\n");
			for(int i=0; i<DATA_SIZE;i++)
				partner.name[i]=input[i];
			update(&partner);
			printf("Updated\n");
		break;
		case 1:
			printf("Enter new IPv4 or domain name : ");
			fgets(input,sizeof(input),stdin);
			strtok(input,"\n");
// veritfy IP
			for(int i=0; i<DATA_SIZE;i++)
				partner.ip[i]=input[i];
			update(&partner);
			printf("Updated\n");
		break;
/*		case X:
		printf("Confirm the deletetion of %s ([Yy]es/[Nn]o):",partner.name);
		char confirm[1];
		fgets(confirm,sizeof(confirm),stdin);
		if(confirm[1]=='y' || confirm[1]=='Y'){
			delete(&partner);
			printf("Deleted\n");
		}
		else
			printf("Leaving...\n");
		break;
*/
		default:
			printf("Wrong selection!\n");
	}
		break;
/*		case X:
			printf("Waiting...");
		break;
*/
		case 2:
		printf("Enter a name of new partner you wish to communicate : ");
		for(int i=0;i<DATA_SIZE;i++)
			input[i]=0;
		fgets(input,sizeof(input)-1,stdin);
		strtok(input,"\n");
		for(int i=0; i<DATA_SIZE;i++)
			partner.name[i]=input[i];
		printf("Enter an IPv4 or domain name of new partner you wish to communicate : ");
		for(int i=0;i<DATA_SIZE;i++)
			input[i]=0;
		fgets(input,sizeof(input),stdin);
		strtok(input,"\n");
		for(int i=0; i<DATA_SIZE;i++)
			partner.ip[i]=input[i];
		add(&partner);
		restart=0;
		break;
		default:
			printf("Wrong choice!\n");
	}
	}
  unsigned long local_port=PORT_IN;
  unsigned long remote_port=PORT_OUT;
  int sock_fd;
  struct sockaddr_in server_addr;
  struct sockaddr_in peer_addr;

  peer_addr.sin_family = AF_INET;
  peer_addr.sin_port = htons(remote_port);
        char ip[IP_SIZE];
	for(int i=0; i<IP_SIZE; i++)
		ip[i]=0;
        if(!hostname2ip(partner.ip , ip)){
  if (inet_aton(ip, &peer_addr.sin_addr) == 0) {
    printf("Error - invalid remote address %s\n",partner.ip);
    return 1;
  }
	}
	else{
  if (inet_aton(partner.ip, &peer_addr.sin_addr) == 0) {
    printf("Error - invalid remote address %s\n",partner.ip);
    return 1;
  }
	}

  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock_fd < 0) {
    printf("Error - failed to open socket: %s\n", strerror(errno));
    return 1;
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(local_port);
  if (bind(sock_fd, (struct sockaddr *)(&server_addr),
           sizeof(server_addr)) < 0) {
    printf("Error - failed to bind socket: %s\n", strerror(errno));
    return 1;
  }

	int newPartner=0;
	for(int i=0; i<DATA_SIZE; i++){
		if(partner.key[i]==0) newPartner++;
	}
	if(newPartner==DATA_SIZE) newPartner=1;
	else newPartner=0;
  if(newPartner) exchangeKeys(sock_fd, &peer_addr, &partner);
	int status=chat(sock_fd, &peer_addr, &partner);
	switch(status){
		case 1:
                	printf("!!! Received Previous ACK! Transmission corrupted! Try another time later !!!\n");
		break;
		case 2:
                	printf("!!! Bad Packet Received! Try another time later !!!\n");
		break;
		case 3:
                	printf("!!! Transmission corrupted! Try another time later !!!\n");
		break;
	}

  close(sock_fd);
	partner.number=0;
	partner.flags=0;
	for(int i=0; i<DATA_SIZE;i++){
		partner.name[i]=0;
		partner.key[i]=0;
		partner.prevKey[i]=0;
		partner.prevData[i]=0;
	}
	for(int i=0; i<IP_SIZE;i++){
		partner.ip[i]=0;
	}
	partner.ip[0]='0';
	partner.ip[1]='.';
	partner.ip[2]='0';
	partner.ip[3]='.';
	partner.ip[4]='0';
	partner.ip[5]='.';
	partner.ip[6]='0';
	inet_aton(partner.ip, &peer_addr.sin_addr);
	printf("******* GOOD BYE *******\n");

  return status;
}

