#include "umpserver.h"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <sstream>

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
#include "umputils.h"

struct umpSendCache {
    std::uint32_t size;
    const char* data;
};

std::mutex sendCacheMutex_;
int flag = 0;
void (*recvCallback_)(unsigned int, const char*, unsigned int);

char* buffer_ = nullptr;
std::atomic<bool> serverRunning_ {true};
std::atomic<bool> hasClient_ {false};
std::thread socketThread_;
bool started_ = false;
std::string name;
bool umpServerStarted() {
    return started_;
}

void umpSend(int i) {
  /*
    UMPLOGI("%d",i);
    UMPLOGI("-------------f l a g");
    flag = 1;
    */
   UMPLOGI("%d",i);
   flag = 1 ;
   std::lock_guard<std::mutex> lock(sendCacheMutex_);
   if(flag ==1 ){
     while (true) {
            {
                std::lock_guard<std::mutex> lock(sendCacheMutex_);
                if (flag == 0)
                    break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
   } 
}

void umpRecv(void (*recvCallback)(unsigned int, const char*, unsigned int)) {
    UMPLOGI("begin start1");
    recvCallback_ = recvCallback;
}

void* umpServerLoop(void* args);

int umpServerStart(int port = 8000) {
    // UMPLOGI("%d",port);
    // if (started_)
    //     return 0;
    // // allocate buffer
    // buffer_ = (char*)malloc(BUFSIZ);
    // memset(buffer_, 0, BUFSIZ);
    // // setup server addr

    // // struct sockaddr_in serverAddr;
    // // memset(&serverAddr, 0, sizeof(serverAddr));
    // // serverAddr.sin_family = AF_INET;
    // // serverAddr.sin_addr.s_addr = INADDR_ANY;
    // // serverAddr.sin_port = htons(port);
    // // create socket
    // const char* MY_SOCK_PATH = "memoryTptServer";
    // struct sockaddr_un serverAddr;
    // memset(&serverAddr, 0, sizeof(serverAddr));
    // serverAddr.sun_family = AF_UNIX;
    // strncpy(&serverAddr.sun_path[1], MY_SOCK_PATH, strlen(MY_SOCK_PATH));
    

    // int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    // if (sock < 0) {
    //     UMPLOGI("start.socket %i", sock);
    //     return -1;
    // }
    // // bind address
    // int ecode = bind(sock, (struct sockaddr *)&serverAddr, sizeof(sa_family_t) + 1 + strlen(MY_SOCK_PATH));
    // if (ecode < 0) {
    //     UMPLOGI("start.bind %i", ecode);
    //     return -1;
    // }
    // // set max send buffer
    // int sendbuff = 327675;
    // ecode = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff));
    // if (ecode < 0) {
    //     UMPLOGI("start.setsockopt %i", ecode);
    //     return -1;
    // }
    // // listen for incomming connections
    // ecode = listen(sock, 2);
    // if (ecode < 0) {
    //     UMPLOGI("start.listen %i", ecode);
    //     return -1;
    // }
    // started_ = true;
    // serverRunning_ = true;
    // hasClient_ = false;
    // socketThread_ = std::thread(umpServerLoop, sock);
    /* 准备这个简单服务器返回的信息 */
    UMPLOGI("%d",port);
    pthread_t tids[1];
    int ret = pthread_create(&tids[0], NULL, umpServerLoop, NULL);
    UMPLOGI("%d",ret);
    UMPLOGI("tpmm::server::heart sever begin!");
    return 0;
}

void umpServerShutdown() {
    if (!started_)
        return;
    serverRunning_ = false;
    hasClient_ = false;
    socketThread_.join();
    delete[] buffer_;
    buffer_ = nullptr;
    started_ = false;
}

void* umpServerLoop(void* args) {
    // struct timeval time;
    // time.tv_usec = 33;
    // fd_set fds;
    // int clientSock = -1;
    // while (serverRunning_) {
    //     if (!serverRunning_)
    //         break;
    //     if (!hasClient_) { // handle new connection
    //         FD_ZERO(&fds);
    //         FD_SET(sock, &fds);
    //         if (select(sock + 1, &fds, NULL, NULL, &time) < 1)
    //             continue;
    //         if (FD_ISSET(sock, &fds)) {
    //             clientSock = accept(sock, NULL, NULL);
    //             if (clientSock >= 0) {
    //                 hasClient_ = true;
    //             }
    //         }
    //     } else {
    //         // check for client connectivity
    //         FD_ZERO(&fds);
    //         FD_SET(clientSock, &fds);
    //         if (select(clientSock + 1, &fds, NULL, NULL, &time) > 0 && FD_ISSET(clientSock, &fds)) {
    //             int length = recv(clientSock, buffer_, BUFSIZ, 0);
    //             if (length <= 0) {
    //                 hasClient_ = false;
    //                 continue;
    //             } else {
    //                 if (length > 0) {
    //                     std::uint32_t type = ntohl(*reinterpret_cast<std::uint32_t*>(buffer_));
    //                     recvCallback_(type, buffer_ + 4, static_cast<std::uint32_t>(length - 4));
    //                 }
    //             }
    //         }
    //         std::lock_guard<std::mutex> lock(sendCacheMutex_);
    //         if (sendCache_.size > 0) {
    //             send(clientSock, &sendCache_.size, 4, 0); // send net buffer size
    //             send(clientSock, sendCache_.data, sendCache_.size, 0); // then send data
    //             // UMPLOGI("sending: %i, %i", sendCache_.compressedSize, sendCache_.size);
    //             sendCache_.size = 0;
    //         }
    //     }
    // }
    // close(sock);
    // if (hasClient_)
    //     close(clientSock);
    buffer_ = (char*)malloc(BUFSIZ);
    args = NULL;
    const char* MY_SOCK_PATH = "memorySnapshotTptServer";

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
      else if(strcmp(ch,"begin")==0){
        int i=1;
        
        for(i=1;i<6555555;i++){
            std::stringstream ssTemp;  
            ssTemp<<i;  
            std::string number=ssTemp.str(); 
            name = "/sdcard/"+number+".rawsnapshot";
            if(access( name.c_str(), F_OK ) == -1){
                break;
            }
        }
        UMPLOGI("qqqqqqqqqqqqqq %d",flag);
        std::uint32_t type = ntohl(*reinterpret_cast<std::uint32_t*>(buffer_));
        recvCallback_(type, buffer_ + 4, static_cast<std::uint32_t>(4));
        UMPLOGI("ppppppppppp %d",flag);
        int p=0;
        std::lock_guard<std::mutex> lock(sendCacheMutex_);
        while(1){
          if(p<10){
            p++;
            sleep(1);
            UMPLOGI("p: %d",p);
            if (flag) {
                UMPLOGI("+++++++++++++flag %d",flag);
                char page[100];
                name.copy(page,name.size(),0);
                *(page+name.size())='\0';
                int len=strlen(page)+1;
                write(new_fd, &len, sizeof(len));
                write(new_fd, &page, len); 
                flag=0;  
                UMPLOGI("------------flag %d",flag);  
                break; 
          } 
          }
          else break;
            
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