#include <stdio.h>
#include <sys/msg.h> //key_t,ftok,msgget,msgsnd,IPC_CREAT 等相关声明都在这里面定义
#include <unistd.h> //getpid 的函数声明在这个头文件里
#include <string.h> //strncmp,strcmp,strlen 的函数声明在这个头文件里
#define BUFSZ 1024


typedef struct message //此结构体用于存放消息，从中可以看到消息的两个字段
{
  long msg_type; //消息类型，以整型值进行标示
  char msg_text[BUFSZ]; //消息内容
}MSG; //取了一个别名

int main() 
{
  int res=-1,qid=0;
  key_t key=IPC_PRIVATE; //IPC_PRIVATE 就是0  
  int len=0; //赋初值是一个好习惯
  MSG msg;
  
  if(-1 == (key=ftok("/",18))) // 通过ftok获取一个key，两个进程可以通过这个key来获取队列信息
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
  
  while(1)
  {
    puts("please enter the message to queue:\n(message start with 'START' will be valid,'quit' to exit)"); 
    if(NULL == (fgets(msg.msg_text,BUFSZ,stdin))) //从标准输入中获取信息放到 msg.msg_text 中
    {
      perror("fgets");
      return res;
    }
    msg.msg_type=getpid(); //将消息的类型设置为本进程的ID
    if( 0== strncmp(msg.msg_text,"START",5) || 0== strcmp(msg.msg_text,"quit\n")) //如果内容是以 START 开头，并且不是 quit，就将这条信息发送
    {
      len=strlen(msg.msg_text);
      if (0 > msgsnd(qid,&msg,len,0)) //发送信息
      {
	perror("msgsnd");
	return res;
      }
    }
    if ( 0 == strcmp(msg.msg_text,"quit\n") ) break; //如果是quit 就进行退出
  }    
  res=0;
  return res;
}
