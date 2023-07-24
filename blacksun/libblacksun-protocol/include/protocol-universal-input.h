//
// Created by RSP on 2023/3/25.
//

#ifndef LIBPROTOCOL_PROTOCOL_UNIVERSAL_INPUT_H
#define LIBPROTOCOL_PROTOCOL_UNIVERSAL_INPUT_H

#include <SDL_scancode.h>
#include <SDL.h>

template<typename T,int length>
struct universal_queue {
    int from,rear;
    T data[length];
    void initialize(){
        from=0;
        rear=0;
    }
    bool inqueue(T& e){
        int nextrear = (rear + 1) % length;
        if(nextrear == from){
            return false;
        }
        data[rear]=e;
        rear=nextrear;
        return true;
    }
    bool dequeue(T& e){
        if(from == rear)
            return false;
        e=data[from];
        from=(from+1)%length;
        return true;
    }
};

enum UNIVERSAL_MOUSEDATA_EVENTTYPE : int {
    UNIVERSAL_MOUSEMOVE = 0,
    UNIVERSAL_MOUSELEFTDOWN,
    UNIVERSAL_MOUSELEFTUP,
    UNIVERSAL_MOUSERIGHTDOWN,
    UNIVERSAL_MOUSERIGHTUP,
};

struct universal_mousedata {
    long dx,dy;
    int type;
};
struct universal_keydata {
    int key;
    int state;
};
struct universal_keystats{
    int m_keystatus[SDL_NUM_SCANCODES];
    int m_btnstatus[256];
};
struct universal_block_key_map{
    int keymap[SDL_NUM_SCANCODES];
    int btnmap[256];
};

class universal_keystats_updater {
protected:
    universal_keystats *m_stats=0;
public:
    universal_keystats_updater(universal_keystats * stats){
        m_stats=stats;
    }
    ~universal_keystats_updater(){

    }
    void update(void *msg){
        SDL_Event *event=(SDL_Event *)msg;
        if (event->type == SDL_KEYDOWN) {
            if (event->key.keysym.scancode >= SDL_NUM_SCANCODES)
                return;
            m_stats->m_keystatus[event->key.keysym.scancode] = 1;
        }
        else if (event->type == SDL_KEYUP) {
            if (event->key.keysym.scancode >= SDL_NUM_SCANCODES)
                return;
            m_stats->m_keystatus[event->key.keysym.scancode] = 0;
        }
        else if (event->type == SDL_MOUSEBUTTONDOWN) {
            if (event->button.button >= 256)
                return;
            m_stats->m_btnstatus[event->button.button] = 1;
        }
        else if (event->type == SDL_MOUSEBUTTONUP) {
            if (event->button.button >= 256)
                return;
            m_stats->m_btnstatus[event->button.button] = 0;
        }
    }
    void reset(){
        memset(m_stats,0,sizeof(universal_keystats));
    }
};

class universal_hotkey{
private:
    universal_keystats *m_keystats;
    bool m_key[SDL_NUM_SCANCODES] = { false };
    bool m_ToggleMap[SDL_NUM_SCANCODES] = { false };
public:
    universal_hotkey(universal_keystats *stats){
        m_keystats=stats;
        for(int i=0;i<SDL_NUM_SCANCODES;i++){
            m_ToggleMap[i]=false;
            m_key[i]=false;
        }
    }
    ~universal_hotkey(){

    }
    bool IsKeyDown(int scancode){
        if (scancode >= sizeof(m_keystats->m_keystatus)/sizeof(int))
            return false;
        return m_keystats->m_keystatus[scancode] == 1;
    }
    bool IsBtnDown(int btn) {
        if (btn >= sizeof(m_keystats->m_btnstatus)/sizeof(int))
            return false;
        return m_keystats->m_btnstatus[btn] == 1;
    }
    bool IsKeyUp(int scancode){
        return !IsKeyDown(scancode);
    }
    bool IsBtnUp(int btn) {
        return !IsBtnDown(btn);
    }

    void update(){
        for(int i=0;i<SDL_NUM_SCANCODES;i++){
            m_ToggleMap[i]=false;
        }
        for(int i=0;i<SDL_NUM_SCANCODES;i++){
            if(IsKeyDown(i))
            {
                if(!m_key[i]){
                    m_ToggleMap[i]=true;
                    m_key[i]=true;
                }
            }
            else
            {
                m_key[i]=false;
            }
        }
    }
    bool IsToggle(SDL_Scancode key) {
        if (key >= SDL_NUM_SCANCODES)
            return false;
        if (m_ToggleMap[key])
            return true;
        return false;
    }

};

#endif //LIBPROTOCOL_PROTOCOL_UNIVERSAL_INPUT_H
