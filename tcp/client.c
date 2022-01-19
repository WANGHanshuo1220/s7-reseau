#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#define SERVER_PORT 8888

int main(int argc, char const *argv[])
{
	int iSocketClient;
	struct sockaddr_in tSocketServerAddr;
	int iRet;
	int iSendLen;
	int iRecvLen;
	unsigned char ucBuffer[999];
	unsigned char ucRecvBuffer[999];

	if(argc != 2)
	{
		printf("Usage: %s <Client_ip> \n", argv[0]);
		return -1;
	}

	//Créer un socket client
	iSocketClient = socket(AF_INET , SOCK_STREAM, 0);
	if(-1 == iSocketClient)
 	{
 		printf("socket error!\n");
 		return -1;
 	}

 	//Définir les informations client
 	tSocketServerAddr.sin_family      = AF_INET;
 	tSocketServerAddr.sin_port        = htons(SERVER_PORT);
 	if(0 == inet_aton(argv[1], &tSocketServerAddr.sin_addr))
 	{
 		printf("invalid server_ip\n");
 		return -1;
 	}
 	memset(tSocketServerAddr.sin_zero, 0, 8);

 	//Demande d'établissement d'un lien
 	iRet = connect(iSocketClient, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
 	if(-1 == iRet)
 	{
 		printf("connect error\n");
 		return -1;
 	}

 	while(1)
 	{
 		//Commandes client à partir de l'entrée standard
 		if(fgets(ucBuffer, 999, stdin))
 		{
 			//Envoyer des informations au client
 			iSendLen = send(iSocketClient, ucBuffer, strlen(ucBuffer), 0);
 			if(iSendLen <= 0)
 			{
 				printf("send error!\n");
 				close(iSocketClient);
 				return -1;
 			}
 		}

 		//Recevoir le message du client
 		iRecvLen = recv(iSocketClient, ucRecvBuffer, 999, 0);
 		if (iRecvLen > 0)
 		{
 			ucRecvBuffer[iRecvLen] = '\0';
 			printf("%s\n", ucRecvBuffer);
 		}
 	}

	return 0;
}