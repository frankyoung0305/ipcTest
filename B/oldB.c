#include <stdio.h>
#include <sys/msg.h> //key_t,ftok,msgget,msgrcv,msgctl,IPC_RMID 相关声明在这个头文件中有所包含
#include <string.h> //memset,strcmp 相关函数声明在这个头文件中有所包含
#define BUFSZ 1024


typedef struct message  //此结构体用于存放消息，从中可以看到消息的两个字段
{
  long msg_type; //消息类型，以整型值进行标示
  char msg_text[BUFSZ]; //消息内容体
}MSG; //取了一个别名

int main()
{
  int res=-1,qid=0;
  key_t key=IPC_PRIVATE;  //IPC_PRIVATE 就是0
  MSG msg;
  
  if(-1 == (key=ftok("/",18))) //调用ftok使用相同的参数生成key，用于获取一样的队列ID
  {
    perror("ftok");
    return res;
  }
  if(-1==(qid=msgget(key,IPC_CREAT|0600))) //创建一个消息队列，将id存到qid中(如果已经存在，则获取它的ID)
  {
    perror("msgget");
    return res;
  }
  printf("open queue %d\n",qid);
  
  do
  {
    memset(msg.msg_text,0,BUFSZ); //将msg.msg_text的内容清零
    if(0 > msgrcv(qid,&msg,BUFSZ,0,0)) //从消息队列中获取信息并且存到msg中
    {
      perror("msgrcv");
      return res;
    } 
    printf("the message from process %ld is %s",msg.msg_type,msg.msg_text); //将信息内容在终端进行打印
  }while(strcmp(msg.msg_text,"quit\n")); //如果内容为quit就进行跳出
  

   if( 0 > msgctl(qid,IPC_RMID,NULL) ) //将队列删除，如果不删除，在进程退出后，消息将依旧保留在内核中，直到重启系统，消息的持久性，界于进程与磁盘之间
   {
     perror("msgctl");
     return res;
   }
  res=0;
  return res;
}
