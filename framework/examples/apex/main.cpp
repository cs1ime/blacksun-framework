//
// Created by huaji on 6/5/2023.
//

#include <stdio.h>
#include <blacksun-controller.h>
#include <string>
#include <oldnames.h>
#include <xorstr.hpp>
#include <protocol-universal-shm.h>
#include <protocol-backend.h>
#include "apex/Parser.h"
#include "apex/offsets.h"
#include "apex/Vars.h"


#include <print-conrtol.h>

namespace Apex {
    std::shared_ptr<process> g_apex=0;
}

blackcontroller *m_dma=0;
backend *m_backend=0;

uint64_t findGameProcess(void* context,std::shared_ptr<ntfuncs> nt){
    uint64_t pid= nt->findpid(xs("r5apex.exe"));
    puts("1");
    fflush(stdout);
    
    return pid;
}
void noticeGameStart(void* context,std::shared_ptr<ntfuncs> nt,std::shared_ptr<process> p){
    puts("Start");
    Apex::g_apex = p;
    Vars::pGameImage=p->sectionbase();
    SetScreen(m_backend->screen_width(),m_backend->screen_height());
    LaunchAimThread();
    
}
void noticeUpdate(void* context,std::shared_ptr<ntfuncs> nt,std::shared_ptr<process> p){
    UpdateDataDisableSmap();
}
void noticeGameQuit(void* context,std::shared_ptr<ntfuncs> nt,std::shared_ptr<process> p){
    puts("Quit");
}

int main(){
    puts("start");

    InitValues();
    universal_shm*shm = universal_shm_wait_and_attach();

    p1x(shm);

    m_backend=new backend(shm);

    m_backend->add_block_btn(SDL_BUTTON_X1);

    puts("start");
    m_dma= new blackcontroller();

    m_dma->setFuncs(findGameProcess,noticeGameStart,noticeGameQuit,noticeUpdate);
    m_dma->setInterval(5);
    m_dma->Launch("win10");

    return 0;
}


