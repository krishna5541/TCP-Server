#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h>
#include<sys/types.h>
#include <string.h> 
#define PORT 7000
#define MAX_LEN 100

int recv_record();

int sock = 0, valread; 
struct sockaddr_in serv_addr; 

int main() 
{ 

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}
	printf("[CONNECTION SUCCESSFULL]\n");
	char order[MAX_LEN + 4];
	printf(">>>Enter Fruit name and quantity(space seperated): ");
	scanf("%[^\n]",order);
	send(sock , order, strlen(order) , 0 ); 
	printf(">>>Order sent\n");
	char buffer[1024] = {0};
	read(sock,buffer,1024);
	printf("[SERVER]: %s\n",buffer);
	recv_record();
	return 0; 
}
int recv_record(){
	uint32_t temp;
	u_int16_t ptemp;
	printf("\nServer Transection Records:\n");
	while(recv(sock,&temp,4,0) && temp){
		uint8_t * p =(uint8_t *)(&temp);
		printf("IP: %d.%d.%d.%d",p[0],p[1],p[2],p[3]);
		recv(sock,&ptemp,2,0);
		u_int16_t port=ntohs(ptemp);
		printf("\tPORT: %u\n",ptemp);
	}
	printf("<<<<<-------->>>>>>\n");
	return 0;
}

