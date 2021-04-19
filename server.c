#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include<time.h>
#define PORT 7000
# define TOTAL_ITEM 4	
#define MAX_LEN 100	// Name of fruits
#define Q_LEN 10	//max 10 client at a time in the queue
#define NUM 1000	//user record array length
typedef struct fruit_info fruit_info;
typedef struct user_req user_req;
typedef struct user_info user_info;
user_req interpret(char *,int);
int process_order(user_req);
int send_record();

struct user_req{
	char fruit_name[MAX_LEN];
	int quantity;
};

struct fruit_info{

	char name[MAX_LEN];
	int quantity;
	char last_sold[100];	//Timestamp in string format
};

struct user_info{
	uint32_t ip;
	u_int16_t port;
};



struct tm* ptr;
time_t lt;
fruit_info list[TOTAL_ITEM]={{"Mango",10,"Never\n"},{"Grapes",1000,"Never\n"},{"Apple",100,"Never\n"},{"Strawberry",50,"Never\n"}};
user_info transec_record[NUM];	//Maximum NUM Transection Record can be kept.
int total_record=0;
int server_fd, new_socket; 
struct sockaddr_in address; 
int addrlen = sizeof(address); 

int main(){ 

	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	// Binding port no 7000 to socket
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, Q_LEN) < 0){ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	}
	for(int count=0;1;count++){
		printf("\n\n\n[LISTENING...]\n");
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){ 
			perror("accept"); 
			exit(EXIT_FAILURE); 
		}
		u_int32_t c_ip=ntohl(address.sin_addr.s_addr);
		uint8_t * p=(uint8_t *)(&c_ip);
		printf("[CONNECTION REQUEST ACCEPTED] client ip: %d.%d.%d.%d\n",p[3],p[2],p[1],p[0]);
		char buffer[1024] = {0};
		if(read(new_socket,buffer,1024)<=0){
			perror("read");
			exit(EXIT_FAILURE);
		}
		user_req order = interpret(buffer,1024);
		printf(">>>Order for %d piece of %s is recieved\n",order.quantity,order.fruit_name);
		int left = process_order(order);
		if(left >= 0){
			strcpy(buffer,"Transection compelete.");
			send(new_socket,buffer,strlen(buffer)+1,0);
			printf("<<<<Request completed.>>>>>\n");
		}
		else{
			strcpy(buffer,"Sorry! We don't have required quantity.");
			send(new_socket,buffer,strlen(buffer)+1,0);
			printf("[WARNING] Request cannot be completed Becouse We only have %d piece of %s\n",order.quantity+left,order.fruit_name);
		}
		send_record();
		printf("Type 'stop' to shut down: ");
		fflush(stdin);
		char cmd[10];
		scanf("%s",cmd);
		if(strcmp(cmd,"stop")==0)
			break;

	}
	printf("ITEM LEFT:\n");
	for(int i =0;i<(sizeof(list)/sizeof(fruit_info));i++){
		printf("NAME: %s\tQUANTITY LEFT: %d\t LAST SOLD:%s",list[i].name,list[i].quantity,list[i].last_sold);
	}
	return 0; 
} 

user_req interpret(char * buffer, int size){
	user_req ret={"",0};
	char flag=0;
	for(int i=0;i<size && buffer[i]!='\0';i++){
		if(flag==0 && buffer[i]!=' '){
			if(i==0 && buffer[i]>=97 && buffer[i]<=122){
				buffer[i] -=32; 
			}
			else if(i>0 && buffer[i]>=65 && buffer[i]<=90){
				buffer[i]+=32;
			}
			ret.fruit_name[i]=buffer[i];
		}
		else if(flag==0 && buffer[i]==' '){
			ret.fruit_name[i]='\0';
			flag=1;
			ret.quantity=0;
		}
		else{
			ret.quantity = ret.quantity*10 + buffer[i] - 48;
		}
	}
	return ret;
}

int process_order(user_req order){
	int len = sizeof(list)/sizeof(fruit_info);
	int ret=0-order.quantity;
	for(int i=0, flag=1;i<len && flag;i++){
		if(strcmp(order.fruit_name,list[i].name)==0 && (list[i].quantity >= order.quantity)){
			lt = time(NULL);
    		ptr = localtime(&lt);
			strcpy(list[i].last_sold, asctime(ptr));
			list[i].quantity = list[i].quantity - order.quantity;
			ret=list[i].quantity;

			transec_record[total_record].ip = ntohl(address.sin_addr.s_addr);
			transec_record[total_record].port = ntohs(address.sin_port);
			total_record=(total_record+1)%NUM;	//It will overwrite in case number of transection >NUM
			flag=0;

		}
		else if(strcmp(order.fruit_name,list[i].name)==0 ){
			ret=ret+list[i].quantity;
		}
	}
	return ret;
}

int send_record(){
	uint32_t temp;
	u_int16_t ptemp;
	for(int i=0;i<total_record;i++){
		temp = htonl(transec_record[i].ip);
		send(new_socket,&temp,4,0);
		ptemp=htons(transec_record[i].port);
		send(new_socket,&ptemp,2,0);
	}
	temp=0;
	send(new_socket,&temp,4,0);
	return 0;
}

