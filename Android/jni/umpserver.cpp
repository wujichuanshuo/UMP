#include "umpserver.h"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <fstream>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include <unistd.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/time.h>
#include "umputils.h"

void umpServerLoop();

int umpServerStart() {
    umpServerLoop();
    return 0;
}

void umpServerLoop() {
  const char* MY_SOCK_PATH = "ioTptServer";

  //宏定义
  //最大连接数
  const int BACKLOG = 5;
  //buffer大小
  //const int BUF_SIZE = 64;

  int fd_A[5] = {0, 0, 0, 0, 0};  //链接块的当前状态tag
  int sock_fd, new_fd;            //前监听，后表示新监听
  struct sockaddr_un server_addr; //服务器信息
  struct sockaddr_un client_addr; // 链接方地址信息

  socklen_t client_addr_length;
  //char buf[BUF_SIZE];
  int ret;
  int i;

  //查询sock是否存在，如果存在则弹出异常
  if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    UMPLOGI("tpmm::server::Can not create sock_fd");
    //return -1;
  }

  //初始化地址信息
  memset(&server_addr, 0, sizeof(server_addr)); //信息归0
  //基础信息设定
  server_addr.sun_family = AF_UNIX;
  strncpy(&server_addr.sun_path[1], MY_SOCK_PATH, strlen(MY_SOCK_PATH));

  //unlink(MY_SOCK_PATH);
  //建立链接
  ret = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(sa_family_t) + 1 + strlen(MY_SOCK_PATH));

  //建立链接状态检查
  if (ret < 0)
  {
    //char error_str[1024];
    UMPLOGI("tpmm::server::Bind error (%s)", strerror(errno));
    //return -1;
  }

  //开始监听及监听状态检
  if (listen(sock_fd, 5) < 0)
  {
    UMPLOGI("Listen error");
    //return -1;
  }
  UMPLOGI("tpmm::server::listen, socket path=%s", MY_SOCK_PATH);
  //fd_set fdsr;         //轮询文件
  int maxsock;         //最大数量
  //struct timeval tv;   //时间函数，检测超时
  //int conn_amount = 0; //当前链接量
  client_addr_length = sizeof(client_addr);
  maxsock = sock_fd;

  //开始轮询
  while (1)
  {
    UMPLOGI("tpmm::server::waiting for client");
    new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &client_addr_length);
    if (new_fd < 0)
    {
      UMPLOGI("tpmm::server::bad client fd");
      continue;
    }
    UMPLOGI("tpmm::server::accept new connection");
    int num=0;
    while(read(new_fd, &num, sizeof(num)) >0){
      //read(new_fd, &num, sizeof(num)) >0)
      UMPLOGI("tpmm::server::get len from client: %d\n", num);
      char ch[num+1];
      ch[num]='\0';
      read(new_fd,&ch,sizeof(ch)-1);
      UMPLOGI("tpmm::server::get char from client: %s\n", ch);
      if(strcmp(ch,"hello")==0){
        char page[] = "hello";
        int len=sizeof(page);
        write(new_fd, &len, sizeof(len));
        write(new_fd, &page, sizeof(page));
      }
      else if(strcmp(ch,"begin")==0) {
        int q = getpid();
        UMPLOGE("pid=%d",q);
        std::string pid = std::to_string(q);
        int read = 0;
        int write = 0;
        int readSpeed = 0;
        int writeSpeed = 0;
        std::string cmd;
        size_t i = 0;
        cmd+="cat /proc/";
        cmd+=pid.c_str();
        cmd+="/io";
        std::ofstream f("/sdcard/iodata.raw",std::ios::out);
        while(1){
            if(read == 0 && write ==0){
              char buf[1024];
                FILE * p_file = NULL;
                UMPLOGI("%s", cmd.c_str()); 
                p_file = popen(cmd.c_str(), "r");
                if (!p_file) {  
                UMPLOGI( "Erro to popen");  
              }  
              std::string readstring="";
              std::string writestring="";
              int flag=0;
              while (fgets(buf, 1024, p_file) != NULL) {  
                UMPLOGI("++%s", buf);
                std::string buff;
                buff+=buf;
                UMPLOGE("BUFF:%s",buff.c_str());
                if(buff.find("read_bytes:")!=buff.npos&&flag==0)
                {
                  readstring += buff.c_str();
                  flag=1;
                }
                else if(buff.find("write_bytes:")!=buff.npos){
                  writestring += buff.c_str();
                }
                buff = "";
              }  
              UMPLOGE("er %s",readstring.c_str());
              UMPLOGE("ew %s",writestring.c_str());
              pclose(p_file);
              int readIndex = readstring.find_first_of("read_bytes:")+12;
              int writeIndex = writestring.find_first_of("write_bytes:")+13;
              std::string tmp;
              std::stringstream ssTemp;
              for(i=readIndex;i<readstring.length();i++){
                if(readstring[i]!='\0'){
                  tmp+=readstring[i];
                }else{
                  break;
                }
              }
              ssTemp<<tmp;  
              ssTemp>>read;
              ssTemp.clear();
              tmp = "";
              for(i=writeIndex;i<writestring.length();i++){
                if(writestring[i]!='\n'){
                  tmp+=writestring[i];
                }else{
                  break;
                }
              }
              ssTemp<<tmp;  
              ssTemp>>write;
              tmp = "";
              ssTemp.clear();
              UMPLOGE("r%d-------w%d--------rs%d--------ws%d",read,write,readSpeed,writeSpeed);
              flag=0;
            }
            else{
                char buf[1024];
                FILE * p_file = NULL;
                p_file = popen(cmd.c_str(), "r");
                UMPLOGE("%s",cmd.c_str());  
                if (!p_file) {  
                UMPLOGI( "Erro to popen");  
              }  
  
              std::string readstring="";
              std::string writestring="";
              int flag=0;
              while (fgets(buf, 1024, p_file) != NULL) {  
                UMPLOGI("++%s", buf);
                std::string buff;
                buff+=buf;
                if(buff.find("read_bytes:")!=buff.npos&&flag==0)
                {
                  readstring += buff.c_str();
                  flag=1;
                }
                else if(buff.find("write_bytes:")!=buff.npos){
                  writestring += buff.c_str();
                }
                buff = "";
              }  
              pclose(p_file);
              UMPLOGE("tr %s",readstring.c_str());
              UMPLOGE("tw %s",writestring.c_str());
              int readIndex = readstring.find_first_of("read_bytes:")+12;
              int writeIndex = writestring.find_first_of("write_bytes:")+13;
              std::string tmp;
              std::stringstream ssTemp;
              for(i=readIndex;i < readstring.length();i++){
                if(readstring[i]!='\0'){
                  tmp+=readstring[i];
                }else{
                  break;
                }
              }
              ssTemp<<tmp;
              int tmp2;
              ssTemp>>tmp2;
              readSpeed = tmp2 - read;
              read=tmp2;
              tmp2=0;
              ssTemp.clear();
              tmp = "";
              for(i=writeIndex;i<writestring.length();i++){
                if(writestring[i]!='\0'){
                  tmp+=writestring[i];
                }else{
                  break;
                }
              }
              ssTemp<<tmp;
              ssTemp>>tmp2;
              writeSpeed = tmp2 - write;
              write=tmp2;
              tmp2=0;  
              tmp = "";
              ssTemp.clear();
              UMPLOGE("r%d-------w%d--------rs%d--------ws%d",read,write,readSpeed,writeSpeed);
              flag =0;
              struct timeval tv;
              gettimeofday(&tv, NULL);
              long time;
              time = (long)((long)tv.tv_sec * 1000 * 1000 + tv.tv_usec);
              f<<"time:"<<time<<" readSpeed:"<<readSpeed<<" writeSpeed:"<<writeSpeed<<std::endl;
            }
            sleep(1);
        }
      }
      

    }
  }
  //关闭socket
  for (i = 0; i < BACKLOG; i++)
  {
    if (fd_A[i] != 0)
    {
      close(fd_A[i]);
    }
  }
  close(sock_fd);
  exit(1);



}

#ifdef __cplusplus
}
#endif // __cplusplus