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
#include 	"bank.h"

#define CLIENT_PORT	5235

static pthread_attr_t	user_attr;
static pthread_attr_t	kernel_attr;
static sem_t		actionCycleSemaphore;
static pthread_mutex_t	mutex;
static int		connection_count = 0;
int 			y;

set_iaddr( struct sockaddr_in * sockaddr, long x, unsigned int port )
{
	memset( sockaddr, 0, sizeof(*sockaddr) );
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons( port );
	sockaddr->sin_addr.s_addr = htonl( x );
}

static char *
ps( unsigned int x, char * s, char * p )
{
	return x == 1 ? s : p;
}

void
periodic_action_handler( int signo, siginfo_t * ignore, void * ignore2 )
{
	if ( signo == SIGALRM )
	{
		sem_post( &actionCycleSemaphore );		// Perfectly safe to do ...
	}
}

void *
periodic_action_cycle_thread( void * arg)
{
	struct sigaction	action;
	struct itimerval	interval;
	bank x = arg;
	pthread_detach( pthread_self() );			// Don't wait for me, Argentina ...
	action.sa_flags = SA_SIGINFO | SA_RESTART;
	action.sa_sigaction = periodic_action_handler;
	sigemptyset( &action.sa_mask );
	sigaction( SIGALRM, &action, 0 );			// invoke periodic_action_handler() when timer expires
	interval.it_interval.tv_sec = 20;
	interval.it_interval.tv_usec = 0;
	interval.it_value.tv_sec = 20;
	interval.it_value.tv_usec = 0;
	setitimer( ITIMER_REAL, &interval, 0 );			// every 3 seconds
	while (1)
	{
		sem_wait( &actionCycleSemaphore );		// Block until posted
	pthread_mutex_lock(&(x->banklock));
		
  
          if(x->numberofaccs == 0)
          {
                  printf("\nNo account information to display");
          }
         else
	{
	int i = 0;
          for(; i < x->numberofaccs; i++)
          {
                  printf("\nThe account name is %s",x->Accs[i]->name);
                  printf("\nThe account balance is %.2f",x->Accs[i]->balance);
                  if(x->Accs[i]->service == 1)
                          printf("\nThe account is in service");
                 else
                         printf("\nThe account is not in service");
                 printf("\n");        
          }
	}
 
         pthread_mutex_unlock(&(x->banklock));
	 sched_yield();
	//pthread_mutex_unlock(&mutex);
	 	
	}
	return 0;
}

void *
client_session_thread( void * bank ) 
{
	int			sd;
	char* request = malloc(512);
	char* argument = malloc(512);
	char* newrequest = malloc(512);
	char* newarg = malloc(512);
	char			temp;
	int			i;
	int 			sdcopy;
	int			limit, size;
	float			ignore;
	long			senderIPaddr;
	char *			func = "client_session_thread";

	printf("\nServer has accepted this client");
	int flag = 1;
	sd = y;	
	sdcopy = sd;				// keeping to memory management covenant
	pthread_detach( pthread_self() );		// Don't join on this thread
	pthread_mutex_lock( &mutex );
	++connection_count;				// multiple clients protected access
	pthread_mutex_unlock( &mutex );
	while ( read( sd, request,256) > 0 )       /*Read the prompt then the following argument(if any) */
	{
		if(isValid(request) == 0)     /*See if the prompt is create */
		{
			read(sd,argument,256);  /*Get account name */
			createAccount(argument,sdcopy,bank);	/*Create the account!*/	
				
		}

		else if(isValid(request) == 1)  /*See if the prompt is serve */
		{
			read(sd,argument,256);
			account temp = findAccounttoServe(argument,sdcopy,bank);
			if(temp == NULL)
			{
				char prompt[] = "\nPress end to leave this prompt and return to the main prompt";
				write(sd,prompt,256); 
				continue;
			} 
			while(1)              /*Extra credit, i just block the thread every 2 seconds */
			{
				if(pthread_mutex_trylock(&(temp->accLock)) == EBUSY)
				{
		char prompt[] = "\nanother client is performing a customer session for this account, you are currently waiting in the queue, do not enter any commands till a confirmation messages till you do so, problems may occur from entering commands prematurely";	
					write(sd,prompt,256);
				}      /*The trylock for loop, every 2 seconds we try to lock*/
				else
				{
					char prompt[] = "\nCongrats, you are now in a customer session, you may enter commands";
					write(sd,prompt,256);
					break;

				}
				sleep(2);

			}

		while(flag)
	{
			read(sd,newrequest,256);
			read(sd,newarg,256);
			float amount = atof(newarg);
			 if(isValid(newrequest) == 5)   /*Query prompt*/                                 
				{
					 
                                         char prompt[256];
                                        sprintf(prompt,"\nThe current balance is % .2f",temp->balance);
                                         write(sd,prompt,256);
					continue;

                                 }


			else if(isValid(newrequest) == 6)   /*End of the customer session, only break to be performed */
                                 {
					  break;
                                                              
			         } 
		
			else
			{	
		
			
				if(isValid(newrequest) == 3)  /*Deposit */
				{
					char prompt[256];
					if(amount < 0 || amount == 0)
					{
						char error[] = "\nInvalid amount, either less than 0 or letters";
						write(sd,error,256);
						continue;

					}
					temp->balance = temp->balance + amount;
					sprintf(prompt,"\nThe deposit of %.2f was successful",amount);
                                         write(sd,prompt,256);

				}
				
				if(isValid(newrequest) == 4)  /* Withdraw */	
				{
					if(amount > temp->balance)
					{
					char prompt[] = "\nThe amount you entered was greater than the current balance";
					write(sd,prompt,256);
					continue;
					}
					char prompt[256];
					temp->balance = temp->balance - amount;
					  sprintf(prompt,"\nThe withdrawl of %.2f is good ",amount);
                                          write(sd,prompt,256);
				}

			}
	}
					
				//printf("This line is reached");
				pthread_mutex_unlock(&(temp->accLock));
		}	
	
		
		//pthread_mutex_unlock(&(temp->accLock));

	}
	

	close(sd);
	pthread_mutex_lock( &mutex );
	--connection_count;				// multiple clients protected access
	pthread_mutex_unlock( &mutex );
	//free(request);
	//free(argument);               /*Free up the memory */
	//free(newrequest);
	//free(argument);
	return 0;
}


int isValid(char* prompt)
 {
         if(memcmp(prompt,"create",6) == 0)
                 return 0;
         else if(memcmp(prompt,"serve",5) == 0)
                 return 1;
         else if(memcmp(prompt,"quit",4) == 0)
                 return 2;
         else if(memcmp(prompt,"deposit",6) == 0)
                 return 3;
         else if(memcmp(prompt,"withdraw",8) == 0)
                 return 4;
         else if(memcmp(prompt,"query",5) == 0)
                 return 5;
         else if(memcmp(prompt,"end",3) == 0)
                 return 6;
         else                
		 return -1;
}



void* tester_thread(void * ignore)
{
	printf("The server says hi\n");
}

void *
session_acceptor_thread( void * ignore )
{
	int			sd;
	int			fd;
	int *			fdptr;
	struct sockaddr_in	addr;
	struct sockaddr_in	senderAddr;
	socklen_t		ic;
	pthread_t		tid;
	int			on = 1;
	char *			func = "session_acceptor_thread";

	pthread_detach( pthread_self() );
	if ( (sd = socket( AF_INET, SOCK_STREAM, 0 )) == -1 )
	{
		printf( "socket() failed in %s()\n", func );
		return 0;
	}
	else if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ) == -1 )
	{
		printf( "setsockopt() failed in %s()\n", func );
		return 0;
	}
	else if ( set_iaddr( &addr, INADDR_ANY, CLIENT_PORT ), errno = 0,
			bind( sd, (const struct sockaddr *)&addr, sizeof(addr) ) == -1 )
	{
		printf( "bind() failed in %s() line %d errno %d\n", func, __LINE__, errno );
		close( sd );
		return 0;
	}
	else if ( listen( sd, 100 ) == -1 )
	{
		printf( "listen() failed in %s() line %d errno %d\n", func, __LINE__, errno );
		close( sd );
		return 0;
	}
	else
	{
		int abc = 0;
		int*  abcptr;
		ic = sizeof(senderAddr);
		while ( (fd = accept( sd, (struct sockaddr *)&senderAddr, &ic )) != -1 )
		{
			y = fd;
			fdptr = (int *)malloc( sizeof(int) );
			*fdptr = fd;					// pointers are not the same size as ints any more.
			if ( pthread_create( &tid, &kernel_attr, client_session_thread, ignore ) != 0 )
			{
				printf( "pthread_create() failed in %s()\n", func );
				return 0;
			}			

			else
				continue;
		}


		
		close( sd );
		return 0;
	}
}

int
main( int argc, char ** argv )
{
	pthread_t		tid;
	char *			func = "server main";
	bank			Chase;             			 //Chase bank since i dont know what to name the variable
	Chase = mallocBank();
	
	

	if ( pthread_attr_init( &user_attr ) != 0 )
	{
		printf( "pthread_attr_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_attr_setscope( &user_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
	{
		printf( "pthread_attr_setscope() failed in %s() line %d\n", func, __LINE__ );
		return 0;
	}
	else if ( pthread_attr_init( &kernel_attr ) != 0 )
	{
		printf( "pthread_attr_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_attr_setscope( &kernel_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
	{
		printf( "pthread_attr_setscope() failed in %s() line %d\n", func, __LINE__ );
		return 0;
	}
	else if ( sem_init( &actionCycleSemaphore, 0, 0 ) != 0 )
	{
		printf( "sem_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_mutex_init( &mutex, NULL ) != 0 )
	{
		printf( "pthread_mutex_init() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_create( &tid, &kernel_attr, session_acceptor_thread, Chase ) != 0 )
	{
		printf( "pthread_create() failed in %s()\n", func );
		return 0;
	}
	else if ( pthread_create( &tid, &kernel_attr, periodic_action_cycle_thread, Chase ) != 0 )
	{
		printf( "pthread_create() failed in %s()\n", func );
		return 0;
	}
	
	else
	{
		printf( "server is ready to receive client connections ...\n" );
		pthread_exit( 0 );
	}
}
