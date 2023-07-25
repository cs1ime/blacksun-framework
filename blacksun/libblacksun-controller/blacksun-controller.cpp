//
// Created by RSP on 2023/3/27.
//
#include <blacksun-controller.h>
#include <qemukvm2dma.h>
#include <dma.h>
#include <util.h>
#include <memory>
#include <print-conrtol.h>
#include <downloader.h>
#include <dma_symbol_remote_pdb.h>

void blackcontroller::RunTime() __attribute__((noinline)){
    uint64_t start_time=util::GetTickCount();
    while(1){
        uint64_t pid=0;
        m_nt->getmmu()->invtlb();
        if (m_nt->getmmu()->chkvirt(0xfffff78000000000) == 0)
            break;
        if((pid=m_switchGameProcess(m_context,m_nt))!=0){
            m_nt->getmmu()->invtlb();
            if(m_nt->pidexist(pid)){
                auto p=m_nt->p(pid);
                {
                    m_nt->getmmu()->invtlb();
                    p->invtlb();
                    m_noticeGameStart(m_context,m_nt,p);
                }

                uint64_t prev_time=0;
                while (1){
                    m_noticeUpdate(m_context,m_nt,p);
                    if (util::GetTickCount() - prev_time > 100) {
                        p->invtlb();
                        m_nt->getmmu()->invtlb();
                        if (!m_nt->pidexist(pid))
                            break;

                        prev_time = util::GetTickCount();
                    }
                    if(!m_nt->pidexist(pid) || !p->isactive())
                        break;
                    util::msleep(m_invertal);
                }
                {
                    m_nt->getmmu()->invtlb();
                    p->invtlb();
                    m_noticeGameQuit(m_context,m_nt,p);
                }
                break;
            }
        }
        else
        {
            if(util::GetTickCount()-start_time>30000){
                puts("out of time");
                break;
            }
        }
        util::msleep(5000);
    }

}

bool blackcontroller::Launch(std::string vm) __attribute__((noinline)){
    if(m_switchGameProcess==nullptr || m_noticeGameQuit==nullptr || m_noticeGameStart==nullptr || m_noticeUpdate==nullptr)
        return false;
    auto ms_downloader = std::make_unique<downloader>(
      "save", "https://msdl.microsoft.com/download/symbols/");
    if (!ms_downloader->valid())
    {
        puts("downloader create failed!");
        return false;
    }
        
    auto factory =
        std::make_shared<dma_symbol_factory_remote_pdb>(std::move(ms_downloader));
    
    while (1) {
        auto qemukvm=std::make_shared<qemukvm2dma>(vm);
        if (qemukvm->valid()) {
            puts("detected qemukvm");
            {
                char testbyte = 0;
                if (qemukvm->read_physical_memory(0, (u8*)&testbyte, 1)) {
                    auto creator = std::make_shared<ntfunc_creator>(factory, qemukvm);
                    auto fns = creator->try_create();
                    if (fns != nullptr && fns->is_valid()) {
                        puts("detected guest system");
                        long long loop_start_time = util::GetTickCount();
                        m_nt=fns;
                        RunTime();
                    }
                    puts("guest system closed");
                }
                util::msleep(5000);
            }
            puts("qemukvm closed");
        }
        if(m_exit)
            break;
        util::msleep(5000);
    }
    return true;
}
