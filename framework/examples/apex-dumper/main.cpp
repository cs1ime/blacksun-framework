#include <iostream>
#include <dma.h>
#include <rawmem2dma.h>
#include <downloader.h>
#include <dma_symbol_remote_pdb.h>
#include "apex/offsets.h"
#include "apex/Vars.h"

#define xs

#define p1x(v1) printf(xs("" #v1 "=%08llX\n"), (uint64_t)v1)
#define p1d(v1) printf(xs("" #v1 "=%08lld\n"), (uint64_t)v1)

namespace Apex
{
  std::shared_ptr<process> g_apex=nullptr;
}

int main()
{
  auto accessor=std::make_shared<rawmem2dma>("/home/zxc/Desktop/memory.pmem");
  if(!accessor->valid())
  {
    std::cout<<"accessor create failed!"<<std::endl;
    return 1;
  }
  auto ms_downloader = std::make_unique<downloader>(
      "save", "https://msdl.microsoft.com/download/symbols/");
  if (!ms_downloader->valid())
  {
    std::cout<<"downloader create error!"<<std::endl;
    return 1;
  }
  auto factory =
      std::make_shared<dma_symbol_factory_remote_pdb>(std::move(ms_downloader));
  auto creator = new ntfunc_creator(factory,accessor);
  auto sys = creator->try_create();
  if(sys == nullptr)
  {
    std::cout<<"cannot detect system!"<<std::endl;
    return 1;
  }

  auto apexpid = sys->findpid("r5apex.exe");

  p1d(apexpid);
  auto apex=sys->p(apexpid);
  u64 base = apex->sectionbase();
	p1x(base);

	// void *data = malloc(0x20000000);
	// apex->read_virt(base, data, 0x20000000);

  // FILE *fp = fopen("apex.mem","w+");
  // fwrite(data,0x20000000,1,fp);
  // fclose(fp);

  Apex::g_apex=apex;

  Vars::pGameImage = base;
	offsets::InstallOffsets();

  return 0;
}
