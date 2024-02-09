//#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include<string.h>
#define num 4

struct matrix 
{
	int row,col;
	int ** m;
};
struct multiply_arg
{
	struct matrix *a, *b;
};

struct matrices
{
     int n;
     struct matrix m[1000];
     
};
struct threadarg
{
   int sockfd;
   int start,end;
   struct matrix *mat;
   
};
struct matrix_pair
{
    struct matrix m1,m2;
};
 

struct matrix *a,**toutput;
int thread_count=0;

void *single(void*arg)
{
   struct matrix* output=(struct matrix *) arg;
   pthread_exit(output);
}

void* multiply(void*arg) // thread function to be called for multiplying two matrices
{

   int i,j,k;
   struct multiply_arg * ip=(struct multiply_arg*)arg; //  getting thread argument pointer by type casting function input to struct multiply_arg*
   struct matrix* output;
   output=(struct matrix*)malloc(sizeof(struct matrix));  
   output->row=(ip->a)->row;
   output->col=(ip->b)->col;
   
   output->m=(int**)malloc((output->row)*sizeof(int*));//printf("82\n");
 //  printf("\n in output");
   for(i=0;i<output->row;i++)
   	output->m[i]=(int*)malloc((output->col)*sizeof(int));
   for(i=0;i<output->row;i++)  // calculating value of each cell of the resultant matrix obtained by multiplying two matrices
   {
      for(j=0;j<output->col;j++)
      {
        output->m[i][j]=0;
        for(k=0;k<((ip->a)->col);k++)
        {
            output->m[i][j]+=(((ip->a)->m)[i][k])*(((ip->b)->m)[k][j]);
            //printf("%d * %d ",(((ip->a)->m)[i][k]),(((ip->b)->m)[k][j]));
        }
       // printf("%d  ",output->m[i][j]);
      }
   //   printf("\n");
   }
   pthread_exit(output);
   
}
void *handle(void *arg)  // thread function to be called from server thread to assign some matrices to clients and storing the output received from client
{
   struct matrix *output=(struct matrix*)malloc(sizeof(struct matrix));
   struct threadarg *a=(struct threadarg*)arg;
   int i,n,r,c,pass,j,k;
   //printf("form index %d to index %d matrix are passed\n",a->start,a->end);
   n=(a->end)-(a->start); // passing number of matrices to be assigned to client
   send(a->sockfd,&n,sizeof(int),0);
   
   for(i=a->start;i<(a->end);i++) // passing matrices to client
   {
        //passing dimension of matrix to client
        r=((a->mat)[i]).row;
        c=((a->mat)[i]).col;
        
        //passing elements of matrix  element by element
   	send(a->sockfd,&r,sizeof(int),0);
   	send(a->sockfd,&c,sizeof(int),0);
   	for(j=0;j<r;j++)
   	{
   	   for(k=0;k<c;k++)
   	   {
   	      pass=(a->mat)[i].m[j][k];
   	      send(a->sockfd,&pass,sizeof(int),0);
   	     // printf("%d  ",pass);
   	      
   	   }
   	  // printf("\n");
   	}
   }
        //receiving the matrix computed by client
   	recv(a->sockfd,&(output->row),sizeof(int),0);
   	recv(a->sockfd,&(output->col),sizeof(int),0);
   	
   	output->m=(int**)malloc((output->row)*(sizeof(int*)));
   	for(i=0;i<output->row;i++)
   		(output->m)[i]=(int*)malloc((output->col)*sizeof(int));
   	// receiving the matrix elements from client element by element
   	for(j=0;j<(output->row);j++)
   	{
   	    for(k=0;k<(output->col);k++)
   	    {
   	       recv(a->sockfd,&((output->m)[j][k]),sizeof(int),0);
   	       
   	    }
   	    
   	}
   	
   
 
   
   close(a->sockfd);
   pthread_exit(output);
}
void client_process(int argc,char* argv[]) // function for executing client 
{
    
    void *o;  // declaring void pointer to be passed to thread
    int num_thread,rc,matrix_count;
    pthread_t thread , threads[1000];
    
    
   struct matrix *input,*a1,*a2,**op,**temp;
   
   
   int sockfd ;
   struct sockaddr_in serv_addr ;
   
   struct multiply_arg a[1000];
   
   int i,n,r,c,j;
   int buf;
   /* Opening a socket */
   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
   printf("Unable to create socket\n");
   exit(0);
                                                       }
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = inet_addr("127.0.0.9");
   serv_addr.sin_port = 6500;
   
   for(i=1;i<argc;i++)
   {
      
      if(strcmp(argv[i],"-i")==0)
      	 break;
   }
   if(i<argc)
   {
      serv_addr.sin_addr.s_addr=inet_addr(argv[i+1]);
      
   }
   for(i=0;i<argc;i++)
   {
      if(strcmp(argv[i],"-p")==0)
      	 break;
   }
   if(i<argc)
   {
      serv_addr.sin_port=atoi(argv[i+1]);
      
   }
   /* With the information specified in serv_addr, the connect( )
   system call establishes a connection with the server process. */
   if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
   {
      printf("Unable to connect to server\n");
      exit(0);
   }
   /* After connection, the client can send or receive messages. However,
   please note that recv( ) will block when the server is not sending and
   vice versa. Similarly send( ) will block when the server is not receiving
   and vice versa. For non-blocking modes, refer to the online man pages.*/  
   recv(sockfd, &buf, sizeof(int), 0);
   //printf("\n numberof matrix sent by server are %d\n",buf);
   n=buf;
   
   // storing the matrix received from server
   input =(struct matrix*)malloc(n*sizeof(struct matrix));
   for(i=0;i<n;i++)
   {
      // receiving ith matrix
       recv(sockfd,&buf,sizeof(int),0);
       input[i].row=buf;
       recv(sockfd,&buf,sizeof(int),0);
       input[i].col=buf;
       input[i].m=(int**)malloc((input[i].row)*sizeof(int*));
       
       
       for(r=0;r<(input[i].row);r++)
       {
           input[i].m[r]=(int*)malloc((input[i].col)*sizeof(int));
       }
       for(r=0;r<input[i].row;r++)//receiving matrix elements from server one by one
       {
          for(c=0;c<input[i].col;c++)
          {
             recv(sockfd,&buf,sizeof(int),0);
             input[i].m[r][c]=buf;
             
          }
        
       }
       
       
       
   }
  
  
 
   op=(struct matrix **)(malloc(n*sizeof(struct matrix*))); // storing the result of multithreaded multiplication  in a data structure initialised to matrices obtained from server
   for(i=0;i<n;i++)
   	op[i]=&input[i];
   
   
   while(n>1)
   {
      
       for(i=0;i<(n-1);i+=2)  // doing adjacent matrix multiplication by using thread
       {
          a[i/2].a=op[i]; 
          a[i/2].b=op[i+1]; 
          
          rc=pthread_create(&threads[i/2],NULL,multiply,(void*)&a[i/2]); 
          //printf("line 253\n");
          if(rc)
          {
             printf("error\n");
             exit(0);
          }
       
       }
       if(n%2) // if n is odd then last matrix should be called alone
       {
          rc=pthread_create(&threads[i/2],NULL,single,(void*)op[n-1]);
          if(rc)
          {
             printf("error\n");
             exit(0);
          }
       
       }
       temp=(struct matrix **)(malloc(((n+1)/2)*sizeof(struct matrix*)));
    
       for(i=0;i<n-1;i+=2) // joining thread results
       {
          pthread_join(threads[i/2],&o);
          temp[i/2]=(struct matrix*)o;
       }
    
       if(n%2)
       {
          pthread_join(threads[i/2],&o);
          temp[i/2]=(struct matrix*) o;
       
       }
    
       free(op);
       op=temp;
       n=(n+1)/2;
       
       
         
   }
  
 
   buf=op[0]->row; 
   send(sockfd,&buf,sizeof(int),0);
   buf=op[0]->col;
   send(sockfd,&buf,sizeof(int),0);;
   for(i=0;i<op[0]->row;i++)
   {
      for(j=0;j<op[0]->col;j++)
      {
         buf=op[0]->m[i][j];
         send(sockfd,&buf,sizeof(int),0);
      }
   }
  // printf("line 280\n");
   pthread_exit(NULL);
   close(sockfd);
}
void server_process(int argc,char *argv[])
{
 int num_thread,rc,matrix_count;
    pthread_t *threads ,final_threads[1000];
    
    struct matrix *a1,*a2,**op,**out;
    struct threadarg arg;
    pthread_attr_t attr;
    
    struct multiply_arg aman[1000];
    int sockfd,newsockfd,clien ;
    struct sockaddr_in cli_addr, serv_addr;
    int buf[1],count;
    
    void *temp;
    
   
    int n , i,j,r,c,size,k;
    
    
    FILE *fp = fopen("input1.txt","r");
    // code for scanning file name : default filename is input1.txt
    for(i=1;i<argc;i++)
    {
       if(strcmp(argv[i],"-f")==0)
       	break;
    }
    if(i<argc)
    	fp=fopen(argv[i+1],"r");
    fscanf(fp,"%d",&n);
   // printf("enter number of clients\n");
  //  pthread_mutex_init(&mutexprod,NULL);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    
    // scanning number of clients from command line , default value is 1
    count =1;
    for(i=1;i<argc;i++)
    {
       if(strcmp(argv[i],"-cl")==0)
       	break;
    }
    if(i<argc)
    	count=atoi(argv[i+1]);
    if(n%count)
    	matrix_count = (n/count)+1;
    else
    	matrix_count=(n/count);
    a=(struct matrix*)malloc(n*sizeof(struct matrix));
    toutput=(struct matrix**)malloc(count*sizeof(struct matrix*));
    threads=(pthread_t*)malloc(count*sizeof( pthread_t));
    num_thread=count;
    // scanning matrices and storing it in a data structure for further computation
    for(i=0;i<n;i++)
    {
        fscanf(fp,"%d%d",&(a[i].row),&(a[i].col));
        a[i].m=(int**)malloc((a[i].row)*sizeof(int*));
        for(r=0;r<a[i].row;r++)
        	a[i].m[r]=(int*)malloc((a[i].col)*sizeof(int));
        for(r=0;r<a[i].row;r++)
        {
            for(c=0;c<a[i].col;c++)
            	fscanf(fp,"%d",&a[i].m[r][c]);
        }
     }
       
        
    
    // creating socket
    
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
         printf("cannot create socket\n");
         exit(0);
    }
    
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=6001;
    //Getting ip adddress from commandline : default IP address is system IP address
    for(i=1;i<argc;i++)
    {
       if(strcmp(argv[i],"-i")==0)
       	break;
    }
    if(i<argc)
    	serv_addr.sin_addr.s_addr=inet_addr(argv[i+1]);
    	
    // Getting port number , default port number is 6001
    for(i=1;i<argc;i++)
    {
       if(strcmp(argv[i],"-p")==0)
       	break;
    }
    if(i<argc)
    	serv_addr.sin_port=atoi(argv[i+1]);
    	
    // binding socket with IP address and port number specified in  serv_addr
    if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {
        printf("unable to bind local address\n");
        exit(0);
    }
    listen(sockfd,5);
    size=n/count;
    k=n%count;
    arg.end=0;
    
    // Allocating matrices to different threads and storing its output
    while(thread_count<count)
    {
       
    	clien=sizeof(cli_addr);
    	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clien) ;
    	
    	if (newsockfd < 0) {
           printf("Accept error\n");
           exit(0);
                          }
   //      pthread_mutex_lock(&mutexprod);
         arg.sockfd=newsockfd;
         arg.start=arg.end;
         if(thread_count<k)
         	arg.end=arg.start + size+1;
         else
         	arg.end=arg.start+size;
         arg.mat=a;
        
        rc=pthread_create(&threads[thread_count],NULL,handle,(void*)&arg);
        if(rc)
        {
            printf("error\n");
            exit(0);
        }
        thread_count++;
       
    
       
                          
    }
    for(i=0;i<count;i++) // joining threads
    {
    	pthread_join(threads[i],&temp);
    	toutput[i]=(struct matrix*)temp;
    }

    for(i=0;i<count-1;i+=2)
   {
      a1=(toutput[i]); 
      a2=(toutput[i+1]);
      aman[i/2].a=a1;   
      aman[i/2].b=a2;    
   }
   op=(struct matrix **)(malloc(count*sizeof(struct matrix*)));
   for(i=0;i<count;i++)
   	op[i]=toutput[i];
   while(count>1) // doing multi threaded multiplication on output matrices obtained from client
   {
    
       // storing thread function arguments
       for(i=0;i<(count-1);i+=2) 
       {
          aman[i/2].a=op[i]; 
          aman[i/2].b=op[i+1]; 
          
          rc=pthread_create(&final_threads[i/2],NULL,multiply,(void*)&aman[i/2]); 
          
          if(rc)
          {
             printf("error\n");
             exit(0);
          }
       
       }
       
       if(count%2)
       {
          rc=pthread_create(&final_threads[i/2],NULL,single,(void*)op[count-1]);
          if(rc) 
          {
             printf("error\n");
             exit(0);
          }
       
       }
       out=(struct matrix **)malloc(count*sizeof(struct matrix*));
       
       // joining the threads and obtaining the results
       for(i=0;i<count-1;i+=2)
       {
          pthread_join(final_threads[i/2],&temp);
          out[i/2]=(struct matrix*)temp;
       }
       
       if(count%2) 
       {
          pthread_join(final_threads[i/2],&temp); 
          out[i/2]=(struct matrix*) temp;  
       
       }
      
       free(op);
       op=out;
       count=(count+1)/2;
       
       
         
   }
    
    //printing the final output matrix
    for(i=0;i<op[0]->row;i++)
    {
       for(j=0;j<op[0]->col;j++)
       	printf("%d  ",op[0]->m[i][j]);
       printf("\n");
    }
    
     close(sockfd);

}
int main(int argc,char* argv[])
{

    int i;
    // checking whether commandline inputs are for server or client
    for(i=1;i<argc;i++)
    {
       
       if(strcmp(argv[i],"-s")==0)
       {
          server_process(argc,argv);
          return 0;
       }
       
    
    }
    
    
    
    for(i=1;i<argc;i++)
    {
       //printf("inside client check\n");
       if(strcmp(argv[i],"-c")==0)
       {
          client_process(argc,argv);
          return 0;
       }
       
    }
    
    printf("invalid input\n");
    return 0;
    
    
    
}
