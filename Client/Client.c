#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <netdb.h> 
#include <stdlib.h>
#include <math.h>

#define PORT 11150
#define MAX_CLI 16
#define BUF_SIZE 128

#define DEMANDE_TRAVAIL	3
#define MESSAGE_RECU	2
#define OK		1
#define DORMIRE		0

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;


typedef struct
{
  int pos_x;
  int pos_y;
  int **contenu;
}block;


char *block_to_char(block );
//char *block_to_char(int** );
int count_voisin(int ** ,int , int );
block au_travail(block );
int init_connect(char *);
block char_to_block(char *);
void communication(SOCKET );



int main(int argc, char* argv[]){
	
	char message[16];
	if (argc < 2)
	    puts("Pas assez d'argument, adresse ip attendu !");
	else {
	
	SOCKET soc=init_connect(argv[1]);
	strcpy(message,"Je suis le client");
	communication(soc );
	
	printf("Fermeture de socket \n ");
	close(soc);
	
	  
	
	}
return 0;
}


void communication(SOCKET soc)
{
  char message[20];
  int valide;
  char * rep;
  
  while(1){
  /* envoie demande */
    memset (&message, 0, sizeof (message));
    strcpy(message,"demande");
    valide =(int) write(soc,&message,strlen(message));
  
    if(valide < 1){
      printf("Echec envoi serveur !");
      exit(1);
    }
  
    memset (&message, 0, sizeof (message));
    printf("1 attente block\n");
    valide=(int )read (soc,&message,26);
    
    if(valide < 1){
    printf("Echec reception serveur !\n");
    exit(1);
    }
    
     if(message[0]=='1' || message[0]=='0' || message[0]=='2' || message[0]=='3') 
     {
	printf("Block reçu : \n");
	puts(message);
 
	block inst = char_to_block(message);
	 /*TRaitement du Block*/
        inst = au_travail(inst);
	rep  = block_to_char(inst); 
	memset (&message, 0, sizeof (message));
	strcpy(message,rep);
	printf("Block Traite : \n");
	puts(message);
      valide= (int) write(soc,&message,12);
      if(valide < 1){
	perror("Echec envoi serveur !");
	sleep(1);
	exit(1);
      }
     }
     if(strcmp(message,"quitte")==0)
     {
       break;
     }
     if(strcmp(message,"dors")==0)
     {
       printf("Je dors une demi seconde \n");
       usleep(500000);
     }
  }
}



char *block_to_char(block input)
  { 
    char *result= (char *)malloc(11*sizeof(char));
    char *tmp2  =(char *)malloc(sizeof(char));
    
    for(int i=0;i<2;i++){
      for(int j=0;j<2;j++){
	sprintf(tmp2,"%d",input.contenu[i][j]);
	strcat(result,tmp2);
      }
    }
    sprintf(tmp2,"*%d*%d*",input.pos_x,input.pos_y);
    strcat(result,tmp2);
    free(tmp2);
    return result;
  }
  
 
int init_connect(char *adresse){
	
	SOCKET sock;
	SOCKADDR_IN adr_ser ;
	struct hostent *host;
	struct in_addr ip;
      
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if(0 > sock ){
	    perror("Erreur création socket !\n");
	    }
	/*Initialise l'adresse du serveur*/
	inet_pton(AF_INET,adresse,&ip);
	host=gethostbyaddr(&ip, sizeof ip, AF_INET);
	if(host == NULL) {
	  perror("Aucun Host !");
	  
	}
	
	bzero((char *) &adr_ser, sizeof(adr_ser));
	adr_ser.sin_family = AF_INET;
	adr_ser.sin_port = htons(11550);
	/*adr_ser.sin_addr = *(IN_ADDR *) host->h_addr;*/
	bcopy((char *)host->h_addr,(char *)&adr_ser.sin_addr.s_addr,host->h_length);
	
	int res = connect(sock,(struct sockaddr *)&adr_ser,sizeof(adr_ser));
	if(res < 0)
		perror("Error connect!");
	else 
		printf("Connection....ok\n");
	return sock;
}


    
  
  
  int count_voisin(int ** block,int i, int j)
  {
    int voisin=0;
    for(int x=-1;x<2;x++){
	for(int y=-1;y<2;y++){
	  if( block[i+x][j+y]==1 )
	    voisin++;          
	}
    }
    if(block[i][j])
      voisin--;
    return voisin;
  }
  
    
block au_travail(block toto)
{
  int voisin;
  
  int **blocks= (int **) malloc(4*sizeof(int));
  
  for(int k=0;k<4;k++){
    blocks[k] = (int *)calloc( 4, sizeof(int));}
  
  
  for(int i=1;i<3;i++){
    for(int j=1;j<3;j++){
      
      if(toto.contenu[i][j]!=2){
	voisin=count_voisin(toto.contenu,i,j);
	if(voisin == 3 && toto.contenu[i][j] == 0)
	{
	  blocks[i-1][j-1]=1;
	}
	else if((voisin==3 || voisin == 2)&& toto.contenu[i][j] == 1)
	{
	  blocks[i-1][j-1]=1;
	}
	else 
	  blocks[i-1][j-1]=0;
      }
      else
      {
	blocks[i-1][j-1]=0;
      }
	
    }
  }

  toto.contenu=blocks;
  return toto;
}


block char_to_block(char *message)
  {
    block res;  
    char *buf=(char*)malloc(sizeof(char));
    int l=0;
    char *token;
    /*Contenu du block*/
    token = strtok(message, "*");
    int t= sqrt((int)strlen(token));
    
    int **result=(int**) malloc(t*sizeof(int*));
    for(int k=0;k<t;k++){
	    result[k] =(int *) calloc (t, sizeof(int));}
	    
    for(int i=0;i<t;i++)
    {
      for(int j=0;j<t;j++)
      {
	buf[0]=token[l];
	result[i][j]=atoi(buf);
	l++;
      }
     }
    res.contenu=result; 
    /*pos_x*/
    token = strtok(NULL, "*");
    //res.pos_x=(int) (token[0]);
    res.pos_x=atoi (token);
    /*pos_y*/
    token = strtok(NULL, "*");
    //res.pos_y=(int)(token[0]);
    res.pos_y=atoi(token);
    return res;
  }
