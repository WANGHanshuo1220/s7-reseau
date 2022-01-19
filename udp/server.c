#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>


#define SERVER_PORT 8888	//Le port par défaut est 8888
#define BACKLOG     10		 //Autoriser au plus un utilisateur à se connecter au serveur en même temps

/*Fonction check() vérifie les commandes saisies par le client 
  *et stocke les informations respectivement dans les infos [5]. 
  *La fonction renvoie la commande client comprend plusieurs parties.
  */
 int  check(char infos[5][10], int *somme, char *ucRecvBuffer);

 /*Fonction saveOperation() enregistre les dix commandes récemment entrées par l'utilisateur 
  *et stocke les informations de commande dans OperationList [10] selon le format.
  *Cette fonction renvoie la valeur d'index du dernier enregistrement.
  */
 int  saveOperation(char OperationList[10][40], char infos[5][10], int *somme, int *format);
 
 /*Fonction sendMsgn() envoie des données à l'utilisateur, 
  *et les informations envoyées à l'utilisateur sont générées selon la commande de l'utilisateur.
  */
 void sendMsg(char infos[5][10], char OperationList[10][40], int *somme, int *sold, int *num, int *iSocketServer, int *numOperation, struct sockaddr_in *tSocketClientAddr);
 
 /*Fonction isformat_right() détermine si la commande saisie 
  *par l'utilisateur est au format correct
  */
 int isformat_right(char infos[5][10], int format);


 int main(int argc, char const *argv[])
 {
 	int iSocketServer;
 	int iRet;
 	struct sockaddr_in tSocketServerAddr;
 	struct sockaddr_in tSocketClientAddr;

 	int iAddrLen;
 	int iRecvLen;
 	unsigned char ucRecvBuffer[999];
 	unsigned char ucSendBuffer[100];

 	signal(SIGCHLD,SIG_IGN);

 	//Créer un socket serveur
 	iSocketServer = socket(AF_INET , SOCK_DGRAM, 0);
 	if(-1 == iSocketServer)
 	{
 		printf("socket error!\n");
 		return -1;
 	}

 	//Définir les informations du serveur
 	tSocketServerAddr.sin_family      = AF_INET;
 	tSocketServerAddr.sin_port        = htons(SERVER_PORT);
 	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
 	memset(tSocketServerAddr.sin_zero, 0, 8);

 	//Configurer la réutilisation du port pour éviter les erreurs de liaison lors de la sortie par erreur
	int on = 1;
 	setsockopt(iSocketServer, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
 	//Lier les informations de socket et de serveur
 	iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
  	if(-1 == iRet)
 	{
 		printf("bind error!\n");
 		return -1;
 	}

 	int sold = 0;				//Le montant total du compte bancaire, la valeur initiale par défaut est 0
 	int somme;					//Le montant d'une seule transaction
 	char infos[5][10];			/*Enregistrez la commande saisie par le client, 
 								 *tockez la commande, l'identifiant client, le nom du compte, 
 								 *le mot de passe du compte et le montant séparément.
 								 */
 	char OperationList[10][40];	//Enregistrez les dix dernières commandes
 	int marque = 0;				//Enregistrez si le client a saisi des informations pour la première fois

 	char id_client[10];
 	char id_compte[10];
 	char id_password[10];

 	while (1)
 	{
 		int a = 1;
 		iAddrLen = sizeof(struct sockaddr);
 		//Recevoir les données du client
 		iRecvLen = recvfrom(iSocketServer, ucRecvBuffer, 999, 0, (struct sockaddr *)&tSocketClientAddr, &iAddrLen);
 		if(iRecvLen <= 0)
 		{
 			printf("recv error!\n");
 		}else
 		{
 			ucRecvBuffer[iRecvLen] = '\0';
 			printf("Get Msg From client (%s): %s", inet_ntoa(tSocketClientAddr.sin_addr), ucRecvBuffer);
 			int format = check(infos, &somme, ucRecvBuffer);
 			
 			//Si c'est la première fois qu'un client envoie un message, enregistrez-le.
 			if(marque == 0)
 			{
 				strcpy(id_client, infos[1]);
 				strcpy(id_compte, infos[2]);
 				strcpy(id_password, infos[3]);
 				marque++;
 			}else
 			{
 				//Après avoir jugé si les informations client se rencontrent la première fois
 				if( 0 != strcmp(id_client,infos[1]) || 0 != strcmp(id_compte,infos[2]) || 0 != strcmp(id_password,infos[3]) )
 				{
 					printf("infos error!\n");
 					a = 0;
 				}else{
 					a = 1;
 				}
 			}

 			if(a == 1){
 				//Si les informations client sont comparées avec succès
 				int numOperation = saveOperation(OperationList, infos, &somme, &format);
 				sendMsg(infos, OperationList, &somme, &sold, &format, &iSocketServer, &numOperation, &tSocketClientAddr);
 			}else
 			{
 				//Si la comparaison des informations client échoue
 				strcpy(ucSendBuffer,"infos error!\n");
 				int iSendLen = sendto(iSocketServer, ucSendBuffer, strlen(ucSendBuffer), 0, (const struct sockaddr *)&tSocketClientAddr, sizeof(struct sockaddr));
 				if(iSendLen <= 0)
 				{
 					printf("send error!\n");
 					exit(0);
 				}
 			}
 		}
 	}
 	close(iSocketServer);
 	return 0;
 }

  int isformat_right(char infos[5][10], int format)
 {
 	int isright = 1;
 	if(format < 3)
 	{
 		isright = 0;
 	}else if((infos[0][0] == 'A' || infos[0][0] == 'a') && format != 4)
 	{
 		isright = 0;
 	}
 	else if((infos[0][0] == 'R' || infos[0][0] == 'r') && format != 4)
 	{
 		isright = 0;
 	}else if((infos[0][0] == 'S' || infos[0][0] == 's') && format != 3)
 	{
 		isright = 0;
 	}else if((infos[0][0] == 'O' || infos[0][0] == 'o') && format != 3)
 	{
 		isright = 0;
 	}
 	return isright;
 }


 int check(char infos[5][10], int *somme, char *ucRecvBuffer)
 {
 	int p = 0;
 	int j = 0;
 	char num[10];
 	for (int i = 0; ucRecvBuffer[i] != '\0'; i++)
 	{
 		/*Utilisez des caractères d'espace comme nœuds 
 		 *pour diviser les informations de commande client
 		 */
 		if(ucRecvBuffer[i] != ' ')
 		{
 			num[j] = ucRecvBuffer[i];
 			j++;
 		}else
 		{
 			num[j] = '\0';
 			strcpy(infos[p],num);
 			j = 0;
 			p++;
 			memset(num,0,sizeof(num));
 		}
 	}
 	num[j-1] = '\0';
 	strcpy(infos[p],num);

 	/*p = 4 indique qu'il y a des informations de montant 
 	 *dans la commande saisie par le client
 	 */
 	if(p == 4)
 	{
 		int a = atoi(num);
 		*somme = a;
 	}else if(p == 3)
 	{
 		infos[4][0] = ' ';
 		infos[4][1] = '\0';
 	}
 	return p;
 }

 int saveOperation(char OperationList[10][40], char infos[5][10], int *somme, int *format)
 {
 	static int num;

 	time_t rawtime;
 	struct tm* timeinfo;
 	time(&rawtime);
 	timeinfo = localtime(&rawtime);
 	char *time = asctime(timeinfo);
 	time[strlen(time)-1] = 0;

 	if(!isformat_right(infos, *format))
 	{
 		return -1;
 	}
 	else if(num < 10)	//Si l'utilisateur saisit moins de 10 informations
 	{
 		strcpy(OperationList[num],infos[0]);
 		strcat(OperationList[num]," (");
 		strcat(OperationList[num],time);
 		strcat(OperationList[num],") ");
 		strcat(OperationList[num],infos[4]);
 		strcat(OperationList[num],"\n");
 		num++;
 	}else
 	{
 		/*Sinon, une fois les informations du premier enregistrement effacées, 
 		 *chaque information est déplacée vers le haut et
 		 *les dernières informations sont insérées dans OperationList [10]
 		 */
 		for (int i = 0; i < 9; i++)
 		{
 			memset(OperationList[i],0,sizeof(OperationList[i]));
 			strcpy(OperationList[i],OperationList[i + 1]);
 		}
 		memset(OperationList[9],0,sizeof(OperationList[9]));
 		strcpy(OperationList[9],infos[0]);
 		strcat(OperationList[9]," (");
 		strcat(OperationList[9],time);
 		strcat(OperationList[9],") ");
 		strcat(OperationList[9],infos[4]);
 		strcat(OperationList[9],"\n");
 	}
 	return num;
 }

 void sendMsg(char infos[5][10], char OperationList[10][40], int *somme, int *sold, int *format, int *iSocketServer, int *numOperation, struct sockaddr_in *tSocketClientAddr)
 {
 	unsigned char ucSendBuffer[999];
 	int iSendLen;

 	if(!isformat_right(infos, *format))		//Jugez d'abord si le format est correct
 	{
 		strcpy(ucSendBuffer,"Le format de la commande est incorrect!\n");
 	}
 	/*Déterminez de quel type de commande il s'agit
 	 *et stockez les informations de réponse correspondantes dans ucSendBuffer
 	 */
 	else if(infos[0][0] == 'A' || infos[0][0] == 'a')
 	{
 		if(*sold += *somme)
 		{
 			strcpy(ucSendBuffer,"OK");
 		}else
 		{
 			strcpy(ucSendBuffer,"KO");
 		}
 	}else if(infos[0][0] == 'R' || infos[0][0] == 'r')
 	{
 		if(*sold >= *somme)
 		{
 			*sold -= *somme;
 			strcpy(ucSendBuffer,"OK");
 		}else
 		{
 			strcpy(ucSendBuffer,"KO");
 		}
 	}else if(infos[0][0] == 'S' || infos[0][0] == 's')
 	{
 		char s[10];
 		sprintf(s,"%d", *sold);
 		strcpy(ucSendBuffer,s);
 		strcat(ucSendBuffer," ");
 		strcat(ucSendBuffer,OperationList[(*numOperation)-2]);
 	}else if(infos[0][0] == 'O' || infos[0][0] == 'o')
 	{
 		strcpy(ucSendBuffer,"votre list de operations est le suivant:\n");
 		for (int i = 0; i < *numOperation; i++)
 		{
 			strcat(ucSendBuffer,OperationList[i]);
 			//strcat(ucSendBuffer,"\n");
 		}
 	}

 	//Envoyer des informations au client
 	iSendLen = sendto(*iSocketServer, ucSendBuffer, strlen(ucSendBuffer), 0, (const struct sockaddr *)tSocketClientAddr, sizeof(struct sockaddr));
 	if(iSendLen <= 0)
 	{
 		printf("send error!\n");
 		exit(0);
 	}
 }