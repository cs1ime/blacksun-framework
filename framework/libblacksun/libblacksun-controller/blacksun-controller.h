//
// Created by RSP on 2023/3/27.
//

#ifndef _BLACKSUN_DMA_H_
#define _BLACKSUN_DMA_H_


#include <dma.h>
#include <pthread.h>
#include <unistd.h>
#include <string>
#include <functional>

using CONTROLLER_FUNCTION_FIND_GAME_PROCESS = std::function<int(void*,std::shared_ptr<ntfuncs>)>;
using CONTROLLER_FUNCTION_NOTICE_GAME_START = std::function<void(void*,std::shared_ptr<ntfuncs>,std::shared_ptr<process>)>;
using CONTROLLER_FUNCTION_NOTICE_GAME_QUIT = std::function<void(void*,std::shared_ptr<ntfuncs>,std::shared_ptr<process>)>;
using CONTROLLER_FUNCTION_NOTICE_UPDATE = std::function<void(void*,std::shared_ptr<ntfuncs>,std::shared_ptr<process>)>;

class blackcontroller {

public:
    blackcontroller(){

    }
    ~blackcontroller(){

    }
    void setFuncs(CONTROLLER_FUNCTION_FIND_GAME_PROCESS findGameProcess,
                  CONTROLLER_FUNCTION_NOTICE_GAME_START noticeGameStart,
                  CONTROLLER_FUNCTION_NOTICE_GAME_QUIT noticeGameQuit,
                  CONTROLLER_FUNCTION_NOTICE_UPDATE noticeUpdate)
    {
        m_findGameProcess=findGameProcess;
        m_noticeGameStart=noticeGameStart;
        m_noticeGameQuit=noticeGameQuit;
        m_noticeUpdate=noticeUpdate;
    }
    void setContext(void* context){
        m_context=context;
    }
    void setInterval(int ms){
        m_invertal=ms;
    }

    bool Launch(std::string vm);

private:
    std::shared_ptr<ntfuncs> m_nt = nullptr;
    pthread_t m_thread_runtime=0;

    bool m_exit=false;

    void* m_context = nullptr;
    int m_invertal=10;

    CONTROLLER_FUNCTION_FIND_GAME_PROCESS m_findGameProcess=nullptr;
    CONTROLLER_FUNCTION_NOTICE_GAME_START m_noticeGameStart=nullptr;
    CONTROLLER_FUNCTION_NOTICE_GAME_QUIT m_noticeGameQuit=nullptr;
    CONTROLLER_FUNCTION_NOTICE_UPDATE m_noticeUpdate=nullptr;

    void RunTime();
};


#endif
