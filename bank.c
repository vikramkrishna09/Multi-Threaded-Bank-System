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
   #include      "bank.h"

/*Malloc space for an account and bank, not really necessary but old habits die hard */
account mallocAccount()
{
	account x = malloc(sizeof(struct Account));
	pthread_mutex_init(&(x->accLock),NULL);
	return x;		
}

bank mallocBank()
{
	bank x = malloc(sizeof(struct Bank));
	x->numberofaccs = 0;
	pthread_mutex_init(&(x->banklock),NULL);	
	return x;
}

int createAccount(char* accountname,int socket, bank x)
{
	//printf("Seg fault at line 34");
	if(pthread_mutex_trylock(&(x->banklock)) == EBUSY)  /*If the bank is in the process of printing out information or in the process of creating a new account, then whichever thread creating the account needs to wait */
	{
		char prompt[] = "\nCurrently the bank is printing out information or another account is being made, so no new account can be created currently";
		write(socket,prompt,256);
		pthread_mutex_lock(&(x->banklock));
	}

	//printf("Seg fault befoe line 41");
	
	//sleep(10);
	
	   /*If the bank isn't printing out information nor is it in the process of creating another account, now i have to check whether the account exists in the bank or not */
	if(x->numberofaccs == 20)
	{
		char prompt[] = "\nSorry, the max number of accounts has been reached";
		write(socket,prompt,256);
		pthread_mutex_unlock(&(x->banklock));
		return -1;
	}

	//printf("Seg fault before line 51");
	for(int i = 0; i < 20; i++)   /*Lastly i need to check whether this account already exists in the bank */
	{
		if((x->Accs[i] != NULL) && (strcmp(accountname,x->Accs[i]->name) == 0))
		{
			char prompt[] = "\nError, another account with the same name already exists in the bank account,try to serve it instead";
			write(socket,prompt,256);
			pthread_mutex_unlock(&(x->banklock));
			return -1;

		}
	}

	account new = mallocAccount();
	memcpy(new->name,accountname,256);   //If these lines are code are reached, an account can be created   
	new->balance = 0.000000000;
	new->service = 0;
	x->Accs[x->numberofaccs] = new;
	x->numberofaccs++;
	pthread_mutex_unlock(&(x->banklock));
	char prompt[] = "\nThe account was created successfully!";
	write(socket,prompt,256);
	return 1;
}	

int PrintOutBankInfo( bank x)
{
	if(pthread_mutex_trylock(&(x->banklock)) == EBUSY)
	{
		char prompt[] = "\nSorry, an account is being created so the account information will print after the account(s) is done being made";
		//write(socket,prompt,sizeof(prompt));
	
	}

	if(x->numberofaccs == 0)
	{
		printf("No account information to display");
	}
	pthread_mutex_lock(&(x->banklock));  //No errors can occur as this only prints out the bank information
	for(int i = 0; i < x->numberofaccs; i++)		
	{
		printf("\nThe account name is %s",x->Accs[i]->name);
		printf("\nThe account balance is %f",x->Accs[i]->balance);
		if(x->Accs[i]->service == 1)
			printf("\nThe account is in service");
		else
			printf("\nThe account is not in service");
		printf("\n\n");
	}

	pthread_mutex_unlock(&(x->banklock));
	return 1;
}
		
	
account findAccounttoServe(char* name,int socket, bank x)   /*Finds the account for serve */
{
	//pthread_mutex_lock(&(x->banklock)); 
	///*Need to lock the bank in order to search through it, EDIT i am going to assume that it is not nneeded really, i.e if someone creates a bank as someone serve its, it can go either way i don't really care */
	for(int i = 0; i <( x->numberofaccs);i++)
	{
		if((x->Accs[i] != NULL) && (strcmp(name,x->Accs[i]->name) == 0))         /*Search for the account name with the current state of the bank */
		{
			//pthread_mutex_unlock(&(x->banklock));
			return x->Accs[i];
		}

	}
	//pthread_mutex_unlock(&(x->banklock));                                          
	///*If these lines are reached, the account that serve was specified with isn't in the bank */
	char prompt[] = "\nError, wrong bank account name has been entered";
	write(socket,prompt,256);
	return NULL;
}
	


/*I use negative 2 to indicate success in order for query to be able to return a correct amount of the balance, while -2 indicates success with depositing or withdrawing */


/*THIS METHOD IS NOT USED ANYMORE */	
int ServeSelector(char* prompt,int socket,account serveaccount,float amount)       /*This method deals with the various serve commands, it is called directly when serve is used */
{
	
	serveaccount->service = 1;

	if(memcmp("deposit",prompt,7) == 0)    /*The deposit command */
	{
		                                  
	
		  
		//pthread_mutex_lock(&(serveaccount->accLock));
		serveaccount->balance = serveaccount->balance + amount;
		printf("%f",amount);
		//pthread_mutex_unlock(&(serveaccount->accLock));
		char prompt[] = "The deposit was successful!";
		write(socket,prompt,256);
		return -2;
	}

	else if(memcmp("withdraw",prompt,8) == 0)  /*The withdraw command */
	{
	/*	if(amount <= ((float)0))
		{
			char prompt[] = "Invalid amount to withdraw";
	              write(socket,prompt,256);
                         return -1;
		}
	*/
                // pthread_mutex_lock(&(serveaccount->accLock));
		if(amount > (serveaccount->balance))
		{
			 char prompt[] = "Invalid amount to withdraw";
                       write(socket,prompt,256);
                          return -1;

		}
                 serveaccount->balance = serveaccount->balance - amount;
                 //pthread_mutex_unlock(&(serveaccount->accLock));
		 char prompt[] = "The withdrawl was successful!";
		write(socket,prompt,256);
                 return -2;
	}


	else if(memcmp("query",prompt,5) == 0);  /*The query command */
	{
		//pthread_mutex_lock(&(serveaccount->accLock));
		float accbalance = serveaccount->balance;
		//pthread_mutex_unlock(&(serveaccount->accLock));
		char prompt[256];
		sprintf(prompt,"%f",accbalance);
		write(socket,prompt,256);
	}	

	/*Important to mention that quit doesn't need to be processed here, as the client-session thread will be able to close the serve session far easier */


}  
	
 
