    #include        <sys/time.h>
  #include        <sys/types.h>
   #include        <sys/socket.h>
   #include        <netinet/in.h>
   #include        <errno.h>
   #include        <semaphore.h>
   #include        <pthread.h>
   #include        <signal.h>
   #include        <stdio.h>
  #include        <stdlib.h>
  #include        <string.h>
 //#include        <synch.h>
  #include        <unistd.h>

/*All of my method types are of type int with the following error scheme, return 0 upon success or -1 upon error */


struct Account
{
	char name[100];
	float balance;
	int service;
	pthread_mutex_t accLock;
	
};
typedef struct Account* account;

struct Bank
{	
     account Accs[20];
     int numberofaccs;						
     pthread_mutex_t banklock;	

};
typedef struct Bank* bank;


account mallocAccount();
bank  mallocBank();

int createAccount(char* name,int socket,bank x);
int PrintoutBankInfo(bank x);
int PromptSelector(char* prompt,char* promptarg,int socket,bank x);
int serveAccount(char* name,int socket, float amount,bank x);
int Quit();
account findAccounttoServe(char* name, int socket, bank x); 
