#include	<sys/types.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<errno.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include 	<pthread.h>
#include	<signal.h>
#include	<sys/time.h>

// Miniature client to exercise getaddrinfo(2).

//int ServeSession;
int Socket;
int timer;
connect_to_server( const char * server, const char * port )
{
	int			sd;
	struct addrinfo		addrinfo;
	struct addrinfo *	result;
	char			message[256];

	addrinfo.ai_flags = 0;
	addrinfo.ai_family = AF_INET;		// IPv4 only
	addrinfo.ai_socktype = SOCK_STREAM;	// Want TCP/IP
	addrinfo.ai_protocol = 0;		// Any protocol
	addrinfo.ai_addrlen = 0;
	addrinfo.ai_addr = NULL;
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_next = NULL;
	if ( getaddrinfo( server, port, &addrinfo, &result ) != 0 )
	{
		fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed.  File %s line %d.\x1b[0m\n", server, __FILE__, __LINE__ );
		return -1;
	}
	else if ( errno = 0, (sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol )) == -1 )
	{
		freeaddrinfo( result );
		return -1;
	}
	else
	{
		do {
			if ( errno = 0, connect( sd, result->ai_addr, result->ai_addrlen ) == -1 )
			{
				sleep( 1 );
				write( 1, message, sprintf( message, "\x1b[2;33mConnecting to server %s ...\x1b[0m\n", server ) );
			}
			else
			{
				freeaddrinfo( result );
				return sd;		// connect() succeeded
			}
		} while ( errno == ECONNREFUSED );
		freeaddrinfo( result );
		return -1;
	}
}


void ServePrompt()
{
	printf("\nThis is the Serve Prompt");
	printf("\ndeposit amount");
	printf("\nwithdraw amount");
	printf("\nquery");
	printf("\nend");
}


void Prompt()
{

	printf("\nThis is the command prompt ");
	printf("\n create accountname ");
	printf("\n serve accountname ");
	printf("\n quit ");
}


void* ServerResponse(void *c)
{
	pthread_detach(pthread_self());
	char message[512];
	char newline[] = "\n";
	while(read(Socket,message,256) > 0)
	{
		printf("%s\n",message);


	} 
	close(Socket);
	printf("Server has shut down, so will this client");
	exit(0);

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


 /* void timerhandler(int signo,siginfo_t* ignore, void* ignore2)
{
	if(signo == SIGALRM)
		{
		//printf("You must now wait 2 seconds, in order to throttle the input commands");
		timer = 1;
		}
}
*/		
int
main( int argc, char ** argv )
{
	int			sd;
	char			message[256];
	char			string[512];
	char			buffer[512];
	//char			prompt[20];
	char			output[512];
	//char			arg[256];
	int			len;
	int			count;
	pthread_t 		tid;
	pthread_attr_t		user;	
	struct sigaction        action;
	struct itimerval	interval;	
	char* prompt = malloc(256);
	char* arg = malloc(256);
	char* newprompt = malloc(256);
	char* newarg = malloc(256);

	if ( argc < 2 )
	{
		fprintf( stderr, "\x1b[1;31mNo host name specified.  File %s line %d.\x1b[0m\n", __FILE__, __LINE__ );
		exit( 1 );
	}
	else if ( (sd = connect_to_server( argv[1], "5235" )) == -1 )
	{
		write( 1, message, sprintf( message,  "\x1b[1;31mCould not connect to server %s errno %s\x1b[0m\n", argv[1], strerror( errno ) ) );
		return 1;
	}
	else
	{
		printf( "Connected to server %s\n", argv[1] );
		Socket = sd;
		if(pthread_attr_init(&user) != 0)
		{
			printf("Error at line 145");
			return 0;
		}
		else if(pthread_attr_setscope(&user,PTHREAD_SCOPE_SYSTEM) != 0)
		{
			printf("Error at line 150");
			return 0;
		}
		else if(pthread_create(&tid,&user,ServerResponse,NULL) != 0)
		{
			printf("Error at line 156");
			return 0;
		}
          
		while(Prompt(),printf("\n"),fgets(string,256,stdin))
	{
			prompt = strtok(string," ");
			arg = strtok(NULL,"-><><><>");				

			//printf("%s",prompt);
			//printf("%s",arg);	
			if(isValid(prompt) == -1)
			{
			printf("\nWrong or incorrect prompt entered,try again");
			continue;
			}



		     else if(isValid(prompt) == 3 || isValid(prompt) == 4 || isValid(prompt) == 5 || isValid(prompt) == 6)
                               {
                                         printf("\nYou arent in a serve session, these commands are invalid");
                               }

			else if(isValid(prompt) == 2)
			{
				printf("This client is closing ");	
				close(sd);
				exit(0);
			}
		
			//printf("\nSize of prompt %i vs strlen prompt %i",sizeof(prompt),strlen(prompt));
			//printf("\nSize of arg %i vs strlen arg %i",sizeof(arg),strlen(arg)); 

			write(sd,prompt,256);	
			write(sd,arg,256);
			if(isValid(prompt) == 1)  /*A customer session has now been opened, even though it is possible for this customer to need to wait to enter any valid serve commands, i.e this session only is valid when the the account specified has its mutex unlocked */
		{
				while(1)
			{
					printf("\n");
					ServePrompt();      				/*Serve/Customer Sesson */
					printf("\n");
					fgets(string,256,stdin);
					newprompt = strtok(string," ");
					newarg = strtok(NULL,"-<><><>");
					int isvalid = isValid(newprompt);
				if(isvalid != 3 && isvalid != 4 && isvalid != 5 && isvalid != 6) /*Checking if prompt is part of the valid serve prompts*/
				{
					printf("\nInvalid prompt was entered");
					continue;

				}
				write(sd,newprompt,256);        /*Send this prompt to serve*/
				if(isvalid == 6)               /*If end is the command, we close the customer session */
				{
					printf("\nCustomer session has now been closed");
					write(sd," ",256);
					break;
				}
				else if(isvalid == 5)
				{
					char bs[] = "-1";
					write(sd,bs,256);
					continue;
				}
				else
				{
					float val = atof(newarg); 
					/*if(val < 0 || val == 0)
					{
						printf("Bad input");
						char bs[] = "-1";
						write(sd,bs,256);
						continue;
					}
					*/
					write(sd,newarg,256);
				}
					sleep(2);
			}	
		}
			sleep(2);
	  }
	}
		free(prompt);
		free(arg);           /*Free up the memory */
		free(newprompt);
		free(newarg);
		close( sd );
		return 0;
	
}
