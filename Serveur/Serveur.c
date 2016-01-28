#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <netdb.h> 
#include <sys/shm.h>
#include <time.h>
#include <math.h>
#include <signal.h>

#define PORT 11150
#define MAX_CLIENT 16
#define BUF_SIZE 128


typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;


typedef struct
  {
    SOCKET sock;
  }Client;

typedef struct
  {
    int pos_x;
    int pos_y;
    int **contenu;
  }block;


  void gestion_clients(Client*,int*,int**, int**);
  void send_all_clients_dors( Client *, int *);
  block char_to_block(char *);
  char *block_to_char(int**,int,int);
  int** lire_grille_file();
  void create_grille_file(int);
  void send_all_clients_quitte( Client *, int *);
  void verif_clients( Client *, int *);
  void connecteur_client ( Client *, int *);
  int init_connect();
  void update_block(block ,int ** );
  void eleve_client(Client *, int *, int);
  void virus(int **,int ,int );
  void insert_vie(int **,int,int);
  void affiche_maj_grille(int ** ,int**);
  
  
  
int main(int argc, char* argv[]) {
	pid_t pid;
	char message[20];
	SOCKET test;
	int *nbr_client;
	Client *client_tab;
	int id1,id2,id3;
	
	id1=shmget(IPC_PRIVATE,MAX_CLIENT, IPC_CREAT|IPC_EXCL|SHM_R|SHM_W);
	id2=shmget(IPC_PRIVATE,1, IPC_CREAT|IPC_EXCL|SHM_R|SHM_W);
	
	client_tab=(Client*)shmat(id1,0,0);
	nbr_client=(int*)shmat(id2,0,0);
	
	id3=shmget(IPC_PRIVATE,18, IPC_CREAT|IPC_EXCL|SHM_R|SHM_W);
	
	int ** grille = (int **)shmat(id3,0,0);
	for(int i=0; i <18;i++)
	  grille[i]=(int*)shmat(id3,0,0);
	
	int ** grille_n = (int **)shmat(id3,0,0);
	for(int i=0; i <18;i++)
	  grille_n[i]=(int*)shmat(id3,0,0);
	
	
	create_grille_file(16);
	
	grille = lire_grille_file();
	grille_n = lire_grille_file();
	
	connecteur_client(client_tab,nbr_client);
	
	printf("Ouverture de Comm \n");
		
	gestion_clients(client_tab,nbr_client ,grille, grille_n);
	
	send_all_clients_quitte( client_tab, nbr_client);
	
	printf("Fermeture de socket \n ");
	
	  close(client_tab[0].sock);
	
	return 0;
}


void affiche_maj_grille(int ** grille,int ** grille_n){
  
  for(int i=0;i<18;i++){
	    for(int j=0;j<18;j++){
	      if(grille_n[i][j]==1)
		printf("\x1B[48;5;2m%d ",grille_n[i][j]);
	      else if (grille_n[i][j]==0)
		printf("\x1B[48;5;5m%d ",grille_n[i][j]);
	      else if (grille_n[i][j]==2)
		printf("\x1B[48;5;7m%d ",grille_n[i][j]);
	      grille[i][j]=grille_n[i][j];
	      
	    }
	    printf("\n");
	  }
}
  /*raquelette ???? */
  

 void gestion_clients(Client *client_tab,int *nbr_client ,int **grille,int ** grille_n)
 {
	char message[25];
	int valide;
	block tmp;
	char *bloc;
	int l=0;
	int gen=0;
	int fin = 0;
	FILE * file;
	file = fopen("Envoi_serveur.txt","w+");
	
	
    float temps;
    clock_t t1, t2;
    t1=0;
    
    while(!fin && gen < 100 ){
      fprintf(file,"\n***********Generation %d***************** \n",gen);
	for(int i=1; i<16;i=i+2)
	{
	  for(int j=1;j<16;j=j+2)
	    {
      fprintf(file,"Envoi du bloc (%d,%d) de la generation %d au client %d \n",i,j,gen,l);
       /*Ecoute d'un client*/
       memset (message, 0, sizeof (message));
       //printf("**1 attente demande (%d,%d)\n",i,j);
       valide = (int) read(client_tab[l].sock,&message,sizeof(message));
       valide = 1;
       
       
       if(valide < 1)
       {
	 printf("Echec récéption du client %d \n",l);
	 eleve_client(client_tab,nbr_client,l);
	 break;
       }
       //printf("2\n");
       //strcpy(message,"demande");
       
       if (strcmp(message,"demande")==0)
       { /*Envoi d'un block */
	 
	
	memset (&message, 0, sizeof (message));
	bloc = block_to_char(grille,i,j);
	
	
	strcpy(message,bloc);
	
	valide=(int) write(client_tab[l].sock,*&bloc,strlen(*&bloc));
	if(valide < 1)
	{
	 printf("Echec Envoi du bloc (%d,%d) au client %d \n",i,j,l);
	 eleve_client(client_tab,nbr_client,l);
	 break;
	}
	/*Attente du block */
	
	memset (&message, 0, sizeof (message));
	valide = (int) read(client_tab[l].sock,&message,sizeof(message));
	fprintf(file,"Réception du bloc (%d,%d) de la generation %d  du client %d\n\n",i,j,gen,l);
	if(valide < 1)
	{
	 printf("Echec réception du bloc (%d,%d) du client %d\n",i,j,l);
	 eleve_client(client_tab,nbr_client,l);
	 break;
	}
	
	
	tmp=char_to_block(message);
	update_block(tmp,grille_n);
	
	bloc=NULL;
	
	l=(l+1) % nbr_client[0];
	
       }
       
    }
   } 
   //send_all_clients_dors(client_tab, nbr_client);
   fin = 1;
	for(int i=0;i<18;i++)
	  for(int j=0;j<18;j++)
	    if(grille[i][j]!=grille_n[i][j]){
	      fin=0;
	      break;
	    }
	    
	    if( gen%10 == 0)
	      virus(grille_n,gen,gen*gen);
   printf("\e[1;1H\e[2J");
   
   printf("\n***********Generation %d***************** \n",gen);
   
   affiche_maj_grille( grille , grille_n);
   
   gen++;
   
	if(fin){
	  printf("Insert vie\n");
	insert_vie(grille,gen,gen*gen);
	fin=0;
	}
	//usleep(500000);
	
    }
	
	printf("\n\n Tous les envois sont dans le fichier Envoi_serveur.txt\n\n");
	
	fclose(file);
   
}
 
 /* Fonction qui met à jour un block dans la grille */

void update_block(block input,int ** grille)
{
  for(int i=input.pos_x; i< input.pos_x+2; i++)
  {
    for(int j=input.pos_y;j<input.pos_y+2;j++)
    {
      grille[i][j]=input.contenu[i-input.pos_x][j-input.pos_y];
    }
  }
}
 
 /*Initialise un socket */
 
int init_connect(){

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   	SOCKADDR_IN sin = { 0 };
	SOCKADDR_IN csin;
	socklen_t client_len=sizeof(csin);
	if(sock < 0)
   	{
		perror("Erreur de socket");
	}
   	int optval=200;
   	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,&optval, sizeof (optval));
	
	sin.sin_addr.s_addr =htonl(INADDR_ANY);/*inet_addr("192.168.1.67");*/
   	sin.sin_port = htons(11550);
   	sin.sin_family = AF_INET;
	if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == -1)
   	{
      		perror("Erreur de bind");
      	}
	if(listen(sock,18) == -1)
	{
	perror("Erreur de réception");
	
	}
	//int res=accept(sock,(struct sockaddr *)&csin,&client_len); 
	//if(res < 0 )
	//	perror("Error accept");
	//else 
	//	printf("Connection....ok\n");
 	//return res;
 	return sock;
}



void connecteur_client ( Client *client_tab, int *nbr_client){
	SOCKADDR_IN sin = { 0 };
	SOCKADDR_IN csin;
	socklen_t client_len=sizeof(csin);
	SOCKET sock=init_connect();
        
     do {
 	client_tab[nbr_client[0]].sock=accept(sock,(struct sockaddr *)&csin,&client_len);
 	printf("Un client vient se connecte avec la socket %d\n",nbr_client[0]);
	nbr_client[0]++;} while(nbr_client[0] < MAX_CLIENT);
     }

     
     
void eleve_client(Client *client_tab, int *nbr_client, int indice)
{
  Client* tmp=(Client *)malloc(MAX_CLIENT);
  int nbr_tmp[0];
  nbr_tmp[0]=0;
  
  client_tab[indice].sock= -99 ;
  
  for (int i=0;i<nbr_client[0];i++)
  {
    if(client_tab[i].sock > 1)
    {
      tmp[nbr_tmp[0]].sock=client_tab[i].sock; 
      nbr_tmp[0]++;
    }
  }
  
  memcpy(client_tab,tmp,sizeof(tmp));
  *nbr_client=nbr_tmp[0];
}

void verif_clients( Client *client_tab, int *nbr_client )
{
    Client* tmp=(Client *)malloc(MAX_CLIENT);
    int nbr_tmp[0];
    nbr_tmp[0]=0;
    char message[20];
    int rd=0,wr=0;
    strcpy(message,"ok");
    
    for (int i=0;i<nbr_client[0];i++)
    {
	 bzero((char *) &message, sizeof(message));
	 strcpy(message,"ok");
	 wr=write(client_tab[i].sock,&message,2);
	 rd=read (client_tab[i].sock,&message,2);
	if( wr < 1 || rd < 1)
	{
		*&client_tab[i].sock=-1;
		printf("le client connecte avec la socket %d ne répond pas \n",i);
	}
	else
	{
		tmp[nbr_tmp[0]].sock=client_tab[i].sock; 
		nbr_tmp[0]++;	    
	}
    }
	memcpy(client_tab,tmp,sizeof(tmp));
	*nbr_client=nbr_tmp[0];    
}


void send_all_clients_dors( Client *client_tab, int *nbr_client){
    char message[10];
    for (int i=0;i<nbr_client[0];i++){
	strcpy(message,"dors");
	write(client_tab[i].sock,&message,sizeof(message));
	}
}

void send_all_clients_quitte( Client *client_tab, int *nbr_client){
    char message[10];
    for (int i=0;i<nbr_client[0];i++){
	strcpy(message,"quitte");
	write(client_tab[i].sock,&message,sizeof(message));
	}
}

  void create_grille_file(int taille)
	{
		FILE *file;
		time_t t;
		srand((unsigned) time(&t));
		int val;
		file = fopen("grille.init","w+");
		if(file == NULL){
			perror("Erreur ouverture fichier");
			exit(1);
		}
		else{
			
			fprintf(file,"%d\n",taille);
			fprintf(file,"%d\n",4);
			fprintf(file,"%d\n",16);
			for(int i =0;i<taille;i++){
				for(int j=0;j<taille;j++){
				val=rand()%2;
				fprintf(file,"%d %d %d\n",i,j,val);
				}
			}
		fclose(file);
		}
	}

int** lire_grille_file(){
	int i,j;
	int taille;
	FILE *file;
	int val;
	file = fopen("grille.init","r");
	if(file==NULL){
	  perror("Erreur Lecture fichier");
	  printf("le fichier va etre cree !\n");
	  create_grille_file(16);
	  lire_grille_file();
	}
	else 
	{
	  /*recupere la taille de la grille*/
	  fseek(file,0,0);
	  fscanf(file,"%d",&taille);
	  printf("le taille de la grille%d\n",taille);
	  /*recupere le nombre se cellules par block*/
	  fscanf(file,"%d",&j);
	  printf("cellules par block %d\n",j);
	  fscanf(file,"%d",&j);
	   printf("Nombre de Clients %d\n",j);
	  taille=taille+2;
	  /*creation de la grille*/	  
	  int **grille=(int **)malloc( (taille) * sizeof(int*));
	  for(int k=0;k<taille;k++){
	    grille[k] = (int *) calloc ( (taille), sizeof(int));}
	    /*initialisation des bord*/
	  for(i=0;i<taille;i++)
	    for(j=0;j<taille;j++)
	      grille[i][j]=0;
	   
	  while(fgetc(file) !=  EOF){
	  fscanf(file,"%d %d %d",&i,&j,&val);
	  grille[i+1][j+1]=val;
	  }
	  
	  return grille;
	    
	}	  
	  return NULL;
}	  

  char *block_to_char(int** grille, int n, int m)
  { 
    char *result=(char *)malloc(10*sizeof(char));
    char *tmp2  =(char *) malloc(sizeof(char));
    
    for(int i=n-1;i<n+3;i++){
      for(int j=m-1;j<m+3;j++){
	sprintf(tmp2,"%d",grille[i][j]);
	strcat(result,tmp2);
      }
    }
    
    sprintf(tmp2,"*%d*%d*",n,m);
    strcat(result,tmp2);
    free(tmp2);
    return result;
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
	/*result[i]= message[i] -48;*/
	result[i][j]=atoi(buf);
	l++;
      }
     }
    res.contenu=result; 
    /*pos_x*/
    token = strtok(NULL, "*");
    //res.pos_x=(int)(token[0]);
    res.pos_x=atoi(token);
    /*pos_y*/
    token = strtok(NULL, "*");
    //res.pos_y=(int)(token[0]);
    res.pos_y=atoi(token);
    return res;
  }

 void virus(int ** grille,int p,int q)
  {
    srand(q+p);
    int i,j,n,m;
    
    n=i=(rand())%16;
    m=j=(rand())%16;
    
    if (n<0)
      n=i=0;
    
    if(m<0)
      m=j=0;
    int quit=0;
    while(!quit){
    if ( (i >= 1) && (j >= 1 ) && (i <= 16) && ( j <= 16)){
	grille[i][j]=2;
	i++;
	j--;
    }
    else if ( (n >= 1) && (m >= 1 ) && (n <= 16) && ( m <= 16)){
	grille[n][m]=2;
	m++;
	n--;
    }
    
    else 
      quit=1;
    }
    
  }
  
  
  void insert_vie(int ** grille,int p, int q)
  {
    int x,y;
    for(int i=0;i<30;i++){
      srand(p+q*p);
      p++;
      x=rand()%16;
      srand(p+q*p);
      y=rand()%16;
      //printf("iV %d %d \n",x,y);
      if(grille[x][y] != 2)
	grille[x][y]=!grille[x][y];
      
    }
  }