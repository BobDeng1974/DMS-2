#include "client-proxy.h"
#include <unistd.h>

#define CLIENT_PROXY_SERVER_PORT	6666
#define LENGTH_OF_LISTEN_QUEUE		20
#define MSG_BUF_SIZE 128

void* client_proxy_thread(void* arg);

bool init_client_proxy(struct Data* data) {
	pthread_t pth_id;
	int error=pthread_create(&pth_id,NULL,client_proxy_thread,(void*)data);
	if(error)
	{
		printf("client proxy thread init fail...\n");
		return false;
	}
	return true;
}

bool getIPAddress() {
	int inet_sock;
        struct ifreq ifr;
        inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

        strcpy(ifr.ifr_name, "wlp2s0");
        if (ioctl(inet_sock, SIOCGIFADDR, &ifr) <  0) {
                perror("get IP address failed by ioctl");
		return false;
	}
	char* ip_addr = inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);
//        printf("Get IP adress: %s\n", ip_addr);
	if(!strcmp(ip_addr,"192.168.43.216"))
		return true;
	return false;
}

void* client_proxy_thread(void* arg) {
	struct Data* data = (struct Data*)arg;
	while(!getIPAddress())
		sleep(5);
	struct sockaddr_in server_addr;
    	bzero(&server_addr,sizeof(server_addr));
    	server_addr.sin_family = AF_INET;
    	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    	server_addr.sin_port = htons(CLIENT_PROXY_SERVER_PORT);
	int server_socket = socket(PF_INET,SOCK_STREAM,0);
	if( server_socket < 0) {
		printf("Create Socket Failed!");
        	exit(1);
	}
	int opt =1;
	setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	if(bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    	{
        	printf("Server Bind Port : %d Failed!", CLIENT_PROXY_SERVER_PORT); 
        	exit(1);
    	}

	if(listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
    	{
        	printf("Server Listen Failed!"); 
        	exit(1);
    	}

	while(true) {

		struct sockaddr_in client_addr;
        	socklen_t length = sizeof(client_addr);
 
        	int new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);
        	if (new_server_socket < 0)
        	{
            		printf("Server Accept Failed!\n");
            		break;
        	}

		printf("Get client: %s connection!\n",inet_ntoa(client_addr.sin_addr));         	
		//only support 1 client for now
		bool client_connect = true;
		while(client_connect) {
        		char r_buffer[MSG_BUF_SIZE];
        		bzero(r_buffer, MSG_BUF_SIZE);
        		length = recv(new_server_socket,r_buffer,MSG_BUF_SIZE,0);
        		if (length < 0)
        		{
            			printf("Server Recieve Data Failed!\n");
            			client_connect = false;
        		}

			//process msg from clinet
			char *msg[2] ;
			char * s = strtok(r_buffer, ":");
			if(s)			{
				msg[0] = s;
				s = strtok(NULL, ".");
				if(s)
					msg[1] = s;
				else
					printf("recevice message format error!\n");
			}
			if(!msg[0])
				break;
			if(!strcmp(msg[0],"speed")){
				printf("Get client speed: %s \n",msg[1]);
				data->c_speed = std::atoi(msg[1]);
			}
			else if(!strcmp(msg[0],"ldw")){
				printf("Set ldw feature status: %s \n",msg[1]);
				if(strlen(msg[1])==3) {
					data->enable_ldw = true;
				} else {
					data->enable_ldw = false;
				}
			}
			else if(!strcmp(msg[0],"fcw")){
				if(strlen(msg[1])==3)
                                        data->enable_fcw = true;
                                else
                                        data->enable_fcw = false;
                                printf("Set fcw feature status: %s \n",msg[1]);
			}
			else if(!strcmp(msg[0],"dsm")){
				if(strlen(msg[1])==3)
                                        data->enable_dsm = true;
                                else
                                        data->enable_dsm = false;
                                printf("Set dsm feature status: %s \n",msg[1]);
			}
			else if(!strcmp(msg[0],"disconnect")){
				printf("disconnect from client!\n");
				break;
			}
			//send msg to client
			char s_buffer[MSG_BUF_SIZE];
                        bzero(s_buffer, MSG_BUF_SIZE);
			sprintf(s_buffer,"%.1f\n",data->fcw_distance);
			if(send(new_server_socket,s_buffer,MSG_BUF_SIZE,0)<0)
                	{
                    		printf("Send message to cleint Failed\n");
                    		client_connect = false;
                	}
		}
		
		close(new_server_socket);

	}

	close(server_socket);

}
