//
// Created by RSP on 2023/3/25.
//

#ifndef LIBPROTOCOL_PROTOCOL_UNIVERSAL_SHM_H
#define LIBPROTOCOL_PROTOCOL_UNIVERSAL_SHM_H

#include <protocol-universal-rend.h>
#include <protocol-universal-input.h>
#include <protocol-rend.h>
#include <stdint.h>
#include <pthread.h>
#include <forceinline.h>
#include <sys/shm.h>
#include <string.h>

#define UNIVERSAL_SHM_MAGIC (static_cast<uint64_t>(0xded5e1170465385d))
#define UNIVERSAL_SEMAPHORE_NAME ("universal_shm_semaphore_name")
#define UNIVERSAL_SHM_KEY (0x9912)

struct universal_shm {
    __forceinline void initialize(){
        if(init_magic!=UNIVERSAL_SHM_MAGIC){
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

            pthread_mutex_init(&queue_mouse_lock,&attr);
            pthread_mutex_init(&queue_key_lock,&attr);

            pthread_mutexattr_destroy(&attr);

            queue_mouse.initialize();
            queue_key.initialize();

            frontendAvaliable=0;
            memset(&keystats,0,sizeof(keystats));
            memset(&block_key_map,0,sizeof(block_key_map));
            memset(&rendtask,0,sizeof(rendtask));
            rendrecver::InitNewTaskBuffer(&rendtask);
        }

        init_magic=UNIVERSAL_SHM_MAGIC;
    }
    __forceinline bool is_initialized(){
        return init_magic==UNIVERSAL_SHM_MAGIC;
    }
    __forceinline void mouse_move(long dx,long dy){
        pthread_mutex_lock(&queue_mouse_lock);
        universal_mousedata dat;
        dat.dx=dx;dat.dy=dy;
        dat.type=UNIVERSAL_MOUSEMOVE;
        queue_mouse.inqueue(dat);
        pthread_mutex_unlock(&queue_mouse_lock);
    }
    __forceinline void mouse_leftkey_down(){
        pthread_mutex_lock(&queue_mouse_lock);
        universal_mousedata dat;
        dat.dx=0;dat.dy=0;
        dat.type=UNIVERSAL_MOUSELEFTDOWN;
        queue_mouse.inqueue(dat);
        pthread_mutex_unlock(&queue_mouse_lock);
    }
    __forceinline void mouse_leftkey_up(){
        pthread_mutex_lock(&queue_mouse_lock);
        universal_mousedata dat;
        dat.dx=0;dat.dy=0;
        dat.type=UNIVERSAL_MOUSELEFTUP;
        queue_mouse.inqueue(dat);
        pthread_mutex_unlock(&queue_mouse_lock);
    }
    __forceinline void mouse_rightkey_down(){
        pthread_mutex_lock(&queue_mouse_lock);
        universal_mousedata dat;
        dat.dx=0;dat.dy=0;
        dat.type=UNIVERSAL_MOUSERIGHTDOWN;
        queue_mouse.inqueue(dat);
        pthread_mutex_unlock(&queue_mouse_lock);
    }
    __forceinline void mouse_rightkey_up(){
        pthread_mutex_lock(&queue_mouse_lock);
        universal_mousedata dat;
        dat.dx=0;dat.dy=0;
        dat.type=UNIVERSAL_MOUSERIGHTUP;
        queue_mouse.inqueue(dat);
        pthread_mutex_unlock(&queue_mouse_lock);
    }
    __forceinline void keyboard_keydown(int key){
        pthread_mutex_lock(&queue_key_lock);
        universal_keydata dat;
        dat.key=key;
        dat.state=0;
        queue_key.inqueue(dat);
        pthread_mutex_unlock(&queue_key_lock);
    }
    __forceinline void keyboard_keyup(int key){
        pthread_mutex_lock(&queue_key_lock);
        universal_keydata dat;
        dat.key=key;
        dat.state=1;
        queue_key.inqueue(dat);
        pthread_mutex_unlock(&queue_key_lock);
    }

    __forceinline bool mouse_dequeue(universal_mousedata& o){
        pthread_mutex_lock(&queue_mouse_lock);
        bool result= queue_mouse.dequeue(o);
        pthread_mutex_unlock(&queue_mouse_lock);
        return result;
    }
    __forceinline bool keyboard_dequeue(universal_keydata& o){
        pthread_mutex_lock(&queue_key_lock);
        bool result = queue_key.dequeue(o);
        pthread_mutex_unlock(&queue_key_lock);
        return result;
    }

    uint64_t init_magic;

    uint32_t frontendAvaliable;

    pthread_mutex_t queue_mouse_lock;
    universal_queue<universal_mousedata,100> queue_mouse;
    pthread_mutex_t queue_key_lock;
    universal_queue<universal_keydata,100> queue_key;
    universal_keystats keystats;
    universal_block_key_map block_key_map;

    ReleasedTaskBuffer rendtask;
};

universal_shm *universal_shm_create_and_attach(key_t shmkey);
universal_shm *universal_shm_attach(key_t shmkey);
void universal_shm_detach(void*shm);
universal_shm *universal_shm_wait_and_attach();

#endif //LIBPROTOCOL_PROTOCOL_UNIVERSAL_SHM_H
