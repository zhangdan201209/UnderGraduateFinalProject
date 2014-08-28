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
char dstip[20];
pthread_t thread[10];
struct sockaddr_in server_addr;
int serversd;
int linknumber;
struct validlink
{
char src[40];
char dst[40];
int  linknumber;
int  totallink;
int amount[10];
int interval;
int speed;
char filename[20];
int  startposition;
int  endposition;
int unit;
};
struct validlink alllink[10];
int linknumber;


int init()
{
    struct ifaddrs * ifAddrStruct=NULL;   
    void * tmpAddrPtr=NULL;
    getifaddrs(&ifAddrStruct);
    FILE * fp;
            char addressBuffer[INET_ADDRSTRLEN];
if ((fp = fopen("localInfor.data", "w")) == NULL)
{
fprintf(stderr, "Cannot open output file.\n");
return 1;
}

printf("Local Network resourse :\n"); 

    while (ifAddrStruct!=NULL) {
        if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;

            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
         if(strcmp( ifAddrStruct->ifa_name,"lo")!=0)
          {  printf("%s IP Address %s\n", ifAddrStruct->ifa_name, addressBuffer); 
         
           fprintf( fp, "%s %s\n", ifAddrStruct->ifa_name,addressBuffer);
          }  
        } 

        ifAddrStruct=ifAddrStruct->ifa_next;
    }
 fprintf(fp,"%s","END");
fclose(fp);
return 0;

}

int getdstip()
{
char temp[20];
printf("Type the server IP : \n");
scanf("%s",temp);
strcpy(dstip,temp);
return 0;
}

int connectserver()
{

if ( ( serversd = socket ( AF_INET , SOCK_STREAM , 0) ) == - 1) {
perror ( "socket error" ) ;
return 1;
}
memset ( & server_addr, 0, sizeof ( struct sockaddr ) ) ;
server_addr.sin_family=AF_INET;
server_addr.sin_port=htons (2012);
server_addr.sin_addr.s_addr=inet_addr(dstip);

if(connect(serversd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == - 1){
perror("connect error");
return 1;
}
 printf("Success connect \n");
}
int exchange()
{
char data[100];
FILE *fp1;
FILE *fp2;
fp1 = fopen("localInfor.data", "r");
fp2 = fopen("remoteInfor.data","w");
int  numbytes;
printf("Start receing Local information:\n");
send(serversd,"EX-1",4, 0) ;
while(fgets(data, 40, fp1)!=NULL)
 { 
  numbytes=send(serversd,data,strlen(data), 0) ;
  printf("Send %d byte: %s",numbytes,data); 
  fflush(stdout);
  memset(data,0,strlen(data));
  }
recv(serversd,data,4,0);
if(strncmp(data,"END1",4)==0)
 {
  printf("\nFinished sending local inforamtion\n\n\n");
 }  
printf("Start receing Remote Server information:\n\n");
while (1) 
  { 
     memset(data,0,strlen(data));
     numbytes=  recv (serversd, data, 100, 0);
     data[numbytes] = '\0' ; 
      
     printf("Received %d bytes : %s \n", numbytes,data) ;      
     fflush(stdout);
      if((strncmp(&data[numbytes-4],"END2",4))==0)
     {

        printf("\nFinished receiving remote inforamtion\n\n\n");
        break;
    }
fprintf(fp2, "%s",data); 

  }
fclose(fp1);
fclose(fp2);
close(serversd);
return 0;
}


int findspeed(struct validlink * arg)
{

struct timeval tv1;
struct timeval tv2;

char data[655350];
char protocol[5];

int eachsendbytes=0;
int allsendbytes=0;

serversd=createconnection(arg->src,arg->dst);

send(serversd,"TEST",4, 0) ;
printf(" Test from %s to %s start\n",arg->src,arg->dst);

gettimeofday(&tv1,NULL);
send(serversd,"START",5, 0) ;
memset(data,'1',655350);
send(serversd,arg->dst,30, 0) ;

while(allsendbytes<6553500)
{
 
  eachsendbytes=send(serversd,data,65535,0) ;
  allsendbytes=allsendbytes+eachsendbytes;
 // printf(" each %d all %d \n", eachsendbytes, allsendbytes);
}
send(serversd,"TEND",4,0);
recv(serversd,protocol,5,0);
if(strncmp(protocol,"TEND",4)==0)
  {gettimeofday(&tv2,NULL);}




arg->interval=((long long)tv2.tv_sec-(long long)tv1.tv_sec)*1000+((long long)tv2.tv_usec-(long long)tv1.tv_usec)/1000;
arg->speed=allsendbytes/arg->interval;
//printf ( "From %s to %s receive %dbytes time %dms speed %dKbps \n" , arg->src,arg->dst,allsendbytes,arg->interval,arg->speed);
//printf ( "---------------------\n\n");


}



int linktest()
{
FILE *fp1;
FILE *fp2;
char srcinterface[30];
char srcip[30];
char dstinterface[30];
char dstip[30];
int amount[10];
int n=0;
int temp=0;
int sum=0;
fp1 = fopen("localInfor.data", "r");
fp2 = fopen("remoteInfor.data","r");
while(fscanf(fp1,"%s %s",srcinterface,srcip)!=EOF)
 {

if(strcmp(srcinterface,"END")!=0)
    { 
      while(fscanf(fp2,"%s %s",dstinterface,dstip)!=EOF)
        {  
         if(strcmp(dstinterface,"END")!=0)
           {  
            strcpy(alllink[n].src,srcinterface);
            strcpy(alllink[n].dst,dstip);
            findspeed(&alllink[n]);
           fflush(stdout);
            n++;
           }
       }
        fseek(fp2,0,0);
    }
 }

linknumber=n;
//Collect link information store in the struct;


for ( n=1;n<linknumber;n++)
{
if(alllink[temp].interval>alllink[n].interval)
     temp=n; 
}
//Calculate the best link rtt;

for ( n=0;n<linknumber;n++)
{ amount[n]=alllink[n].interval/alllink[temp].interval;
 sum=sum+amount[n];
}

//Use the best link rtt to calculate the sum amount and store each amount in amount[4];
for ( n=0;n<linknumber;n++)
{ 
  alllink[n].totallink=linknumber;
  alllink[n].linknumber=n+1;
  for(temp=0;temp<linknumber;temp++)
  {alllink[n].amount[temp]=amount[temp];}
}

//Store total link number ,sequence and all amount in struct;


/*
for ( n=0;n<linknumber;n++)
{ 
  printf("totallinkis %d\n", alllink[n].totallink);
   printf("linknumber %d\n",alllink[n].linknumber);
  for(temp=0;temp<linknumber;temp++)
  printf("amount is %d",alllink[n].amount[temp]);
}

*///printf struct information;

fclose(fp1);
fclose(fp2);
close(serversd);
}


int createconnection(char *src,char *dst)


{
int sd;
struct ifreq interface;
struct sockaddr_in dstip;

if ( ( sd= socket ( AF_INET , SOCK_STREAM , 0) ) == - 1) {
perror ( "socket error" ) ;

}


strncpy(interface.ifr_name, src, IFNAMSIZ);
if (setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE,(char *)&interface, sizeof(interface)) < 0)
{
    close(sd);
    return -1;
}


memset ( & dstip, 0, sizeof ( struct sockaddr ) ) ;
dstip.sin_family=AF_INET;
dstip.sin_port=htons (2012);
dstip.sin_addr.s_addr=inet_addr(dst);
int sock_buf_size=655350;

setsockopt( sd, SOL_SOCKET, SO_SNDBUF, (char *)&sock_buf_size, sizeof(sock_buf_size) );
if(connect(sd,(struct sockaddr *)&dstip,sizeof(struct sockaddr)) == - 1){
perror("connect error");
return -1;
}


return sd;


}




int CalTaskAmount(struct validlink *arg)

{
int temp;
double temp2;
int unit;
int fp;
int startamount=0;
int startposition=0;
int endposition=0;
struct stat buf;


lstat(arg->filename, &buf);
//Find the size of the file;
printf(" %s  size %ld\n",arg->filename,buf.st_size);
for(temp=0;temp<arg->linknumber;temp++)
{
startposition=arg->amount[temp]+startposition;
}
//Caluation the percentage start position;
startposition=startposition-1;
temp2= ((double)buf.st_size)/((double)(arg->totallink));
unit= (int)ceil(temp2);

startposition=startposition*unit;

//Caluation the actual start position;

 
endposition=startposition+arg->amount[arg->linknumber-1]*unit;






//Caluation the actual end position;

arg->startposition=startposition;
if(arg->linknumber==arg->totallink)
  arg->endposition=buf.st_size;
else
arg->endposition=endposition;
arg->unit=unit;
//Save in the struct;
//printf( "Start %d  End %d Se %d\n",arg->startposition,arg->endposition,arg->linknumber);

return 0;


}
















void * multisend (void *arg1)
{
struct validlink * arg=(struct validlink *)arg1;
struct stat buf;
char protocol[10];
char buffer[100];
char data[655350];
struct timeval tv1;
struct timeval tv2;
int multisd;
int fp;
int eachsendbytes=0;
int eachreadbytes=0;
int allsendbytes=0;
int allreadbytes=0;
char * filename ="haha";
strcpy(arg->filename,filename);
multisd=createconnection(arg->src,arg->dst);
bzero(buffer,100);
CalTaskAmount(arg);
sprintf(buffer,"%s %s %s %d %d",arg->src,arg->dst,arg->filename,arg->startposition,arg->endposition);

//printf("buffer is %s \n",buffer);








send(multisd,"MULT",4,0);

recv(multisd,protocol,7,0);

if(strncmp(protocol,"REQUEST",7)==0)
{
send(multisd,filename,7,0);
}
recv(multisd,protocol,6,0);
if(strncmp(protocol,"START1",6)==0)
{
 send(multisd,buffer,100,0);
 //printf("Sequence is %d \n",arg->linknumber);
}
recv(multisd,protocol,6,0);
if(strncmp(protocol,"START2",6)==0)
{



fp = open(arg->filename,O_RDONLY);
lseek(fp,arg->startposition,0);
printf("Se %d start %d  end %d \n", arg->linknumber,arg->startposition,arg->endposition);



gettimeofday(&tv1,NULL);
while(1)
{
     bzero(data,655350);
     eachreadbytes=read(fp,  data,655350);
      allreadbytes=allreadbytes+eachreadbytes;
if(allreadbytes<(arg->endposition-arg->startposition))
    {
     eachsendbytes=write(multisd,data,655350);
     allsendbytes=allsendbytes+eachsendbytes;
    }
else if(allreadbytes>(arg->endposition-arg->startposition))
    {
      eachsendbytes=write(multisd,data,((arg->endposition-arg->startposition)-allsendbytes));
      allsendbytes=allsendbytes+eachsendbytes;
     // printf("%d OHH\n",arg->linknumber);
      break;
    }
else  if(allreadbytes=(arg->endposition-arg->startposition) )
     { 
       eachsendbytes=write(multisd,data,eachreadbytes);
       allsendbytes=allsendbytes+eachsendbytes;
     //printf("%d =\n",arg->linknumber);
       break;
     }

//printf("Se %d Send %d should send %d\n",arg->linknumber,allsendbytes,arg->endposition-arg->startposition);


}



}









recv(multisd,protocol,3,0);
if(strncmp(protocol,"END",3)==0)
{
gettimeofday(&tv2,NULL);
close(multisd);
}
arg->interval=((long long)tv2.tv_sec-(long long)tv1.tv_sec)*1000+((long long)tv2.tv_usec-(long long)tv1.tv_usec)/1000;
arg->speed=allsendbytes/arg->interval;

printf ( "Se %d From %s to %s send %dbytes time %dms speed %dKbps should %d  \n" ,  arg->linknumber,arg->src,arg->dst,allsendbytes,arg->interval,arg->speed,arg->endposition-arg->startposition);
//printf ( "---------------------\n\n");





pthread_exit(NULL);
}









int usercommand()

{
int n;
void *thread_result;
struct timeval tv1;
struct timeval tv2;
int speed;
int interval;
printf("totallinkis %d\n", alllink[0].totallink);
printf("--------------------------\n");
for ( n=0;n<linknumber;n++)
{ 
   printf("linknumber %d :from %s  to %s  speed is %d Kbps\n",alllink[n].linknumber,alllink[n].src,alllink[n].dst,alllink[n].speed);
   
}
printf("Which one you want to use?\n");
printf("Use 1 3\n");

gettimeofday(&tv1,NULL);
for ( n=0;n<linknumber;n++)
{
pthread_create(&thread[n],NULL,multisend, (void *) &alllink[n]);

}

pthread_join(thread[0],&thread_result);
printf("1 is over\n");
pthread_join(thread[1],&thread_result);
printf("2 is over\n");
pthread_join(thread[2],&thread_result);
printf("3 is over\n");
pthread_join(thread[3],&thread_result);
printf("4 is over\n");
gettimeofday(&tv2,NULL);
interval=((long long)tv2.tv_sec-(long long)tv1.tv_sec)*1000+((long long)tv2.tv_usec-(long long)tv1.tv_usec)/1000;
speed=102400001/interval;

printf (" speed %d Kbps \n" , speed);
//printf ( "---------------------\n\n");


return 0;
}








int main()

{

if(init()==1)
  {
   printf("Init Wrong \n");
  }

if(getdstip()==1)
  {
   printf("Wrong IP type in \n");
  }
if(connectserver()==1)
  {
   printf("Connect failed\n");
  }

if(exchange()==1)
   {
   printf("Exchange failed\n");
  }
if(linktest()==1)
   {
   printf("Test failed\n");
  }
if(usercommand()!=0)
    {
   printf("User command failed\n");
  }
 
}



