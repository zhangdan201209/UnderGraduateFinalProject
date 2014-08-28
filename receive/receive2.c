# include <stdio.h>
#include <stdlib.h>
# include <string.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <net/if.h>
#include <sys/stat.h>
#include <pthread.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>

struct validlink
{
char src[40];
char dst[40];
char filename[20];
int  startposition;
int  endposition;
};
int alllink=0;
struct validlink link[100];
pthread_mutex_t mut;
void  exchange(int arg1)

{
int arg = arg1; 
FILE* fp1;
FILE* fp2;
fp1 = fopen("remoteinformation", "w");
fp2 = fopen("localInfor.data", "r");
char data[100];

printf("Start receiving remote information:\n");
   while (1)
        { 
            memset(data,0,100);
       int  numbytes=  recv ( arg, data, 100, 0);
            data[numbytes] = '\0' ; 
            fprintf(fp1, "%s",data);         
            printf("Received %d bytes \n",numbytes);
            printf("%s",data) ;
            fflush(stdout); 
      if((strncmp(&data[numbytes-3],"END",3))==0)
        break;   
        } 

 fclose(fp1);
 printf ( "\nFinished receiving remote information\n\n\n\n" ) ;
send(arg,"END1",4, 0) ;     
 printf ( "Start sending Local information:\n\n" ) ;
 fflush(stdout);
while(fgets(data,100,fp2)!=NULL)
 { 
  int i=send(arg,data,100, 0) ;
  printf("Send %d bytes: %s \n",i,data); 
  fflush(stdout);
  memset(data,0,strlen(data));
  }
send( arg, "END2", 4, 0);
fclose(fp2);
printf ( "Finished sending local information\n" );
printf ( "Information exchange stage is over\n" );

close (arg) ; 


}


void  testlink(int arg1)
{
int arg = arg1;
struct timeval tv1;
struct timeval tv2;
char protocol[5];
char local[30];
char data[65535];
int eachrecvbytes=0;
int allrecvbytes=0;
int interval=0;
int speed=0;
struct sockaddr_in name;
int addrlen=sizeof(struct sockaddr_in);

recv( arg, protocol, 5,0);
recv( arg, local, 30,0);
//printf("Protocol is %s \n",protocol);
if(strncmp(protocol,"START",5)==0)
 {
gettimeofday(&tv1,NULL);
}
 while (1)
        { 
          memset(data,0,65535);
          eachrecvbytes=recv(arg,data,65535,0);
          allrecvbytes=allrecvbytes+eachrecvbytes;
          //printf(" each %d all %d \n", eachrecvbytes, allrecvbytes);
         // fflush(stdout);
         if (strncmp(&data[eachrecvbytes-4],"TEND",4)==0)
            break;
        } 
gettimeofday(&tv2,NULL);
//getchar();
send(arg,"TEND",4, 0) ;
interval=((long long)tv2.tv_sec-(long long)tv1.tv_sec)*1000+((long long)tv2.tv_usec-(long long)tv1.tv_usec)/1000;

speed=(allrecvbytes-4)/interval;

   printf ( "receive %dbytes time %dms speed %dKbps \n" , allrecvbytes-4,interval,speed);


close(arg);

}



void  multireceive( int new_fd)


{
int temp;
int thislink;
char buffer[100];
int fp;
int eachwritebytes=0;
int allwritebytes=0;
int eachrecvbytes=0;
char data[655350];
bzero(buffer,100);
 pthread_mutex_lock(&mut);
thislink=alllink;
alllink++;
pthread_mutex_unlock(&mut);

char filename[20];
 memset (filename,'\0',20);
send(new_fd,"REQUEST",7,0);  
recv(new_fd,filename,20,0);
//printf("filename %s \n",filename);
send(new_fd,"START1",6,0);
temp=recv(new_fd,buffer,100,0);
buffer[temp]='\0';
//printf("%s %d\n",buffer,thislink);
sscanf(buffer,"%s %s %s %d %d",link[thislink].src,link[thislink].dst,link[thislink].filename,&link[thislink].startposition,&link[thislink].endposition);



printf("%s %s %s %d %d \n",link[thislink].src,link[thislink].dst,link[thislink].filename,link[thislink].startposition,link[thislink].endposition);

fp=open(link[thislink].filename,O_WRONLY|O_CREAT);

lseek(fp,link[thislink].startposition,SEEK_SET);
send(new_fd,"START2",6,0);


while(allwritebytes<(link[thislink].endposition-link[thislink].startposition))
{  bzero(data,655350);
   eachrecvbytes= recv(new_fd,data,655350,0) ;
   
  eachwritebytes= write(fp,data,eachrecvbytes) ;
  allwritebytes=allwritebytes+eachwritebytes;
 // printf(" each %d all %d \n", eachsendbytes, allsendbytes);
}

printf(" Write %d should wirte %d \n",allwritebytes,link[thislink].endposition-link[thislink].startposition);

send(new_fd,"END",3,0);


close(new_fd);


}

void  * operation( void * arg)
{
int new_fd = (int )arg;
char protocol[5];


memset(protocol,'\0',5);
read( new_fd, protocol, 4);
printf("protocol is %s\n",protocol);
switch(protocol[0])

{
case 'E' : 
          exchange(new_fd);
          break;
          
case 'T' : 
         
          testlink(new_fd);
          break;


case 'M' :  
         multireceive( new_fd);

          break;
          
}





}







int main()

{

int sockfd, new_fd, allbytes,numbytes; 
struct sockaddr_in server_addr; 
struct sockaddr_in client_addr; 
int sin_size;




pthread_t th;
    
pthread_mutex_init(&mut,NULL);


if ((sockfd = socket (AF_INET,SOCK_STREAM ,0) ) == - 1) { 
    perror ("socket error") ; 
    return 1; 
    } 



memset(&client_addr,0,sizeof(struct sockaddr)); 
server_addr.sin_family=AF_INET; 
server_addr.sin_port=htons (2012);
server_addr.sin_addr.s_addr=INADDR_ANY; 
if (bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == - 1)
    { 
     perror ( "bind error" ) ; 
     return 1; 
    } 
int sock_buf_size=655350;
setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF,
(char *)&sock_buf_size, sizeof(sock_buf_size) );
while(1){
if (listen(sockfd,5) == - 1){ 
    perror ("listen error") ; 
     return 1; 
    } 



    
     sin_size = sizeof ( struct sockaddr_in ) ; 
     if ( ( new_fd = accept ( sockfd, ( struct sockaddr * ) & client_addr, & sin_size) )== - 1) 
        { 
         perror ( "accept error" ) ; 
         
        } 
//int cflags =fcntl(new_fd,F_GETFL,0);
//fcntl(sockfd,F_SETFL, cflags|O_NONBLOCK);
 printf ( "server: got connection from %s\n" , (char *)inet_ntoa(client_addr. sin_addr));
pthread_create(&th,NULL,operation, (void *) new_fd);
pthread_detach(th);

}
    return 0; 
}











