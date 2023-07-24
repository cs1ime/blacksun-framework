#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mygui.h"
#include <iostream>


namespace mygui{
FN_MYGUI_init g_PFN_MYGUI_init=0;
FN_MYGUI_update g_PFN_MYGUI_update=0;
FN_MYGUI_destory g_PFN_MYGUI_destory=0;
FN_MYGUI_event g_PFN_MYGUI_event=0;
FN_MYGUI_loop g_PFN_MYGUI_loop=0;

    void setfunc(uint64_t _init,uint64_t _update,uint64_t _destory,uint64_t _event,uint64_t _loop){
        g_PFN_MYGUI_init=(FN_MYGUI_init)((uint64_t)_init);
        g_PFN_MYGUI_update=(FN_MYGUI_update)((uint64_t)_update);
        g_PFN_MYGUI_destory=(FN_MYGUI_destory)((uint64_t)_destory);

        g_PFN_MYGUI_event=(FN_MYGUI_event)((uint64_t)_event);
        g_PFN_MYGUI_loop=(FN_MYGUI_loop)((uint64_t)_loop);
    }

    void init(){
        void *mydrawer=dlopen(("libmoonlight-mydrawer.so"),RTLD_NOW);
        if(mydrawer!=0){
            void *pfnGetFuncs=dlsym(mydrawer,("GetFuncs"));
            if(pfnGetFuncs){
                auto fnGetFuncs= (uint64_t *(*)())pfnGetFuncs;
                uint64_t *data=fnGetFuncs();
                uint64_t _init=data[0];
                uint64_t _update=data[1];
                uint64_t _destory=data[2];
                uint64_t _event=data[3];
                uint64_t _loop=data[4];

                setfunc(_init,_update,_destory,_event,_loop);
                return;
            }
        }
        std::cout<<"no mydrawer module"<<std::endl;
        exit(1);
    }


}




