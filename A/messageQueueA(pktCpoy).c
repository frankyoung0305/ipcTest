#include <stdio.h>
#include <sys/msg.h> //key_t,ftok,msgget,msgsnd,IPC_CREAT 等相关声明都在这里面定义
#include <unistd.h> //getpid 的函数声明在这个头文件里
#include <string.h> //strncmp,strcmp,strlen 的函数声明在这个头文件里
#define BUFSZ 2048
#define MAXLEN 1500  //最大caplen
////////////////////////
#include "pcap.h" //pcapreader

#include <sys/time.h> //time stamp
//struct timeval timestamp;
//gettimeofday( &timestamp, NULL);

int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y)
{       
	int nsec;
		
	if ( x->tv_sec>y->tv_sec )
		return -1;
		
	if ( (x->tv_sec==y->tv_sec) && (x->tv_usec>y->tv_usec) )
		return -1;
		
	result->tv_sec = ( y->tv_sec-x->tv_sec );
	result->tv_usec = ( y->tv_usec-x->tv_usec );
		
	if (result->tv_usec<0)
	{
		result->tv_sec--;
		result->tv_usec+=1000000;
	}
	return 0;
} 

int StrCopyEx (void* pstFrom ,void* pstTo)
{
	if (pstFrom == NULL || pstTo == NULL)
	return -1 ;
	unsigned char* pstF = (unsigned char*) pstFrom ;
	unsigned char* pstT = (unsigned char*) pstTo ;
	while (*pstT++ = *pstF++) ;
	return 0 ;
}//unsigned char 数组copy
typedef struct message //此结构体用于存放消息，从中可以看到消息的两个字段
{
    long msg_type; //消息类型，以整型值进行标示
    //bpf_u_int32 caplen; //pcap pkt length
    //struct timeval timestamp;
    u_char pktdata[MAXLEN]; //pkt内容
}MSG; //取了一个别名

int main() 
{
    int res=-1,qid=0;
    key_t key=IPC_PRIVATE; //IPC_PRIVATE 就是0  
    int len=0; //赋初值是一个好习惯 (in byte
    int counter=0;
    int maxlen = 0;
    MSG msg;
    
    struct timeval startTime, stopTime,diff;

    char errbuf[100];  //error buf for pcapReader

    pcap_t *pfile = pcap_open_offline("traffic_sample.pcap", errbuf);  //head
    if (NULL == pfile) {
        printf("%s\n", errbuf);
        return -1;
    } 
    //printf("file opened\n");
    struct pcap_pkthdr *pkthdr = 0;
    const u_char *pktdata = 0;
  
    if(-1 == (key=ftok("/",18))) // 通过ftok获取一个key，两个进程可以通过这个key来获取队列信息 fname = "/", keep fname 
    {
        perror("ftok");
        return res;
    }

    if(-1==(qid=msgget(key,IPC_CREAT|0600))) //创建一个消息队列，将id存到qid中
    {
        perror("msgget");
        return res;
    }
  
    printf("open queue %d\n",qid); //将qid进行显示

    gettimeofday( &startTime, NULL);  //start time
    //printf("time stamped\n");
    while(pcap_next_ex(pfile, &pkthdr, &pktdata) == 1)
    {

        //if(NULL == (fgets(msg.msg_text,BUFSZ,stdin))) //从标准输入中获取信息放到 msg.msg_text 中
        //{
        //    perror("fgets");
        //    return res;
        //}
    //pcap_next_ex(pfile, &pkthdr, &pktdata);  //read
        //printf("pkt got, write:\n");
        //for (bpf_u_int32 i = 0; i < pkthdr->caplen; ++i) {
        //    msg.pktdata[i] = pktdata[i];
    	//}
    	//for (bpf_u_int32 i = 0; i < pkthdr->caplen; ++i) {
        //	if (0 < i && 0 == i % 16) printf("\n");
        //	printf("%2x ", pktdata[i]);
    	//}
	    //msg.pktdata = (char *)pktdata;
        //printf("wrote\n");
		StrCopyEx(pktdata, msg.pktdata);  //copy u_char
        msg.msg_type=getpid(); //将消息的类型设置为本进程的ID
   
        len += pkthdr->caplen; //accumulate 
        if (pkthdr->caplen >= maxlen)
	    maxlen = pkthdr->caplen;
        if (0 > msgsnd(qid,&msg,pkthdr->caplen,0)) //发送信息
        {
            perror("msgsnd");
	    return res;
        }
        counter++;
	//printf("send %d pkt. length:%d\n", counter, pkthdr->caplen);
    
    }
    gettimeofday( &stopTime, NULL);  //start time 
       
    res=0;
    pcap_close(pfile);
    printf("Done.\n");
    timeval_subtract(&diff,&startTime,&stopTime);
    double bandWidth = ((double)len / diff.tv_usec) / 1024 / 1024 *1000000;
    double pps = ((double)counter/diff.tv_usec) * 1000000;
    printf("time-used:%d us\npks:%d\npps:%lf\nbandwidth:%lf Mbyte/s maxlen: %d",diff.tv_usec, counter, pps, bandWidth, maxlen);
    return res;
}
