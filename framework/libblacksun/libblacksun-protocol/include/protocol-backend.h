//
// Created by RSP on 2023/3/26.
//

#ifndef LIBPROTOCOL_PROTOCOL_BACKEND_H
#define LIBPROTOCOL_PROTOCOL_BACKEND_H

#include <protocol-universal-shm.h>
#include <protocol-rend.h>
#include <protocol-universal-input.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>


class backend{
private:
    ReleasedTaskSenderCache m_cache;
    rendsender * m_rendsender=nullptr;
    universal_shm *m_shm=nullptr;
    universal_hotkey* m_hkey=nullptr;
    sem_t *m_sem=nullptr;
public:
    backend(universal_shm * shm){
        m_sem=sem_open(UNIVERSAL_SEMAPHORE_NAME,O_CREAT,S_IRUSR|S_IWUSR,0);
        m_rendsender=new rendsender(&m_cache,&shm->rendtask.Task);
        m_shm=shm;
        m_hkey=new universal_hotkey(&shm->keystats);
    }
    ~backend(){
        sem_close(m_sem);
        delete m_rendsender;
        delete m_hkey;
    }
    uint32_t screen_width(){
        return m_shm->rendtask.Task.Share.PlaneWidth;
    }
    uint32_t screen_height(){
        return m_shm->rendtask.Task.Share.PlaneHeight;
    }

    void input_mouse_move(long dx,long dy){
        m_shm->mouse_move(dx,dy);
        sem_post(m_sem);
    }
    void input_mouse_leftkey_down(){
        m_shm->mouse_leftkey_down();
        sem_post(m_sem);
    }
    void input_mouse_leftkey_up(){
        m_shm->mouse_leftkey_up();
        sem_post(m_sem);
    }
    void input_mouse_rightkey_down(){
        m_shm->mouse_rightkey_down();
        sem_post(m_sem);
    }
    void input_mouse_rightkey_up(){
        m_shm->mouse_rightkey_up();
        sem_post(m_sem);
    }
    void input_keyboard_keydown(int key){
        m_shm->keyboard_keydown(key);
        sem_post(m_sem);
    }
    void input_keyboard_keyup(int key){
        m_shm->keyboard_keyup(key);
        sem_post(m_sem);
    }
    void add_block_key(int vkey){
        if(vkey>=SDL_NUM_SCANCODES)
            return;
        m_shm->block_key_map.keymap[vkey]=1;
    }
    void remove_block_key(int vkey){
        if(vkey>=SDL_NUM_SCANCODES)
            return;
        m_shm->block_key_map.keymap[vkey]=0;
    }
    void add_block_btn(int vkey){
        if(vkey>=256)
            return;
        m_shm->block_key_map.btnmap[vkey]=1;
    }
    void remove_block_btn(int vkey){
        if(vkey>=256)
            return;
        m_shm->block_key_map.btnmap[vkey]=0;
    }

    rendsender *rend(){
        return m_rendsender;
    }
    universal_hotkey* hkey(){
        return m_hkey;
    }

};

#endif //LIBPROTOCOL_PROTOCOL_BACKEND_H
