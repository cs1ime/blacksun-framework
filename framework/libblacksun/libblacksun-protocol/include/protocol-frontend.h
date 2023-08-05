#ifndef _PROTOCOL_FRONTEND_H_
#define _PROTOCOL_FRONTEND_H_


#include <protocol-universal-shm.h>
#include <protocol-rend.h>
#include <protocol-universal-input.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

class frontend {
private:
    sem_t *m_sem=nullptr;    
    rendrecver *m_rendrecv=nullptr;
    universal_shm *m_shm=nullptr;
    universal_keystats_updater *m_keyupdater=nullptr;
    universal_hotkey* m_hkey=nullptr;
public:
    frontend(universal_shm *shm){
        m_sem = sem_open(UNIVERSAL_SEMAPHORE_NAME, O_CREAT, S_IRUSR | S_IWUSR, 0);
        m_rendrecv = new rendrecver(&shm->rendtask);
        m_shm=shm;
        m_keyupdater=new universal_keystats_updater(&shm->keystats);
        m_hkey=new universal_hotkey(&shm->keystats);
    }
    ~frontend(){
        sem_close(m_sem);
        delete m_rendrecv;
        delete m_keyupdater;
        delete m_hkey;
    }
    rendrecver *rend(){
        return m_rendrecv;
    }
    sem_t* sem()
    {
        return m_sem;
    }
    bool mouse_dequeue(universal_mousedata& o){
        return m_shm->mouse_dequeue(o);
    }
    bool keyboard_dequeue(universal_keydata& o){
        return m_shm->keyboard_dequeue(o);
    }
    bool is_key_blocked(int vkey) const {
        if(vkey>=SDL_NUM_SCANCODES)
            return false;
        return m_shm->block_key_map.keymap[vkey]!=0;
    }
    bool is_btn_blocked(int vbtn) const {
        if(vbtn>=256)
            return false;
        return m_shm->block_key_map.btnmap[vbtn]!=0;
    }
    void update_hotkey(void* msg){
        m_keyupdater->update(msg);
        m_hkey->update();
    }
    universal_hotkey* hkey(){
        return m_hkey;
    }


};



#endif
