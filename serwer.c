#define _REENTRANT

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h> //bzero warning

//socket
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <signal.h>

//wątki
#include <pthread.h>
//X11
#include <X11/Xlib.h>
#include <X11/X.h>



typedef struct{
	int userId;
	XImage *image;
} dane;

dane d1;
dane daneSerwer;


int userId = 0;
struct sockaddr_in userTable[100]; //uzytkownicy
int sd, clen, i;

void *sendValues(){
	while(1){
		//printf("userID:%d\n", userId);
		//wyslanie dla kazdego usera
		
		for (i = 0; i < userId; i++) {
			//printf("..próba wysłania do: %d",i);
			sendto(sd, &(daneSerwer), sizeof(daneSerwer), 0, (struct sockaddr *) &userTable[i], clen);
			//printf("..wyslalem do: %d\n",i);
    		}
		usleep((5000000));
	}
}


void main(){
	struct sockaddr_in sad, cad;
	sd = socket( AF_INET, SOCK_DGRAM, 0 );
	bzero( (char *) &sad, sizeof(sad) );

	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = htonl( INADDR_ANY );
	sad.sin_port = htons((ushort)5000);


	bind( sd, (struct sockaddr *) &sad, sizeof(sad) );
	clen = sizeof(cad);

	pthread_t tid;
	pthread_create(&tid, NULL, sendValues, NULL);
	while (1)
	{
		printf("czekanie...\n");
		recvfrom( sd, (char*)&(d1), sizeof(d1), 0, (struct sockaddr*)&cad, &clen );
		printf("otrzymalem sygnal!\n");
		//buff = ntohl(buff);
		//buff = htonl(ntohl(buff));
		//d1.userId = htonl(ntohl(d1.userId));


		if(d1.userId==-1){
			if(userId==0){
				daneSerwer.image = d1.image;		
			}
			userTable[userId] = cad;
			daneSerwer.userId= userId;
			userId++;
		}else{
			daneSerwer = d1;
		}
	}
 

}
