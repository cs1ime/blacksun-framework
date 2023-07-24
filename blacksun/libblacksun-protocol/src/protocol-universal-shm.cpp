//
// Created by RSP on 2023/3/26.
//

#include <protocol-universal-shm.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdint.h>
#include <util.h>

universal_shm *universal_shm_create_and_attach(key_t shmkey){
    int shmid;
    key_t key;
    char* shm, * s;

    key = shmkey;

    if ((shmid = shmget(key, sizeof(universal_shm), IPC_CREAT | 0666)) < 0)
    {
        return nullptr;
    }

    if ((shm = (char*)shmat(shmid, NULL, 0)) == (char*)-1)
    {
        return nullptr;
    }
    auto strushm = (universal_shm*)shm;

    return strushm;

}
universal_shm *universal_shm_attach(key_t shmkey){
    int shmid;
    key_t key;
    char* shm, * s;

    key = shmkey;

    if ((shmid = shmget(key, sizeof(universal_shm), 0666)) < 0)
    {
        return nullptr;
    }

    if ((shm = (char*)shmat(shmid, NULL, 0)) == (char*)-1)
    {
        return nullptr;
    }
    auto strushm = (universal_shm*)shm;

    return strushm;
}

void universal_shm_detach(void*shm){
    shmdt(shm);
}
universal_shm *universal_shm_wait_and_attach(){
    universal_shm *shm=0;
    while(shm==0){
        shm=universal_shm_attach(UNIVERSAL_SHM_KEY);
        if(shm){
            if(!shm->is_initialized()){
                universal_shm_detach(shm);
                shm=0;
            }
            else{
                break;
            }
        }
        util::msleep(100);
    }
    return shm;
}
