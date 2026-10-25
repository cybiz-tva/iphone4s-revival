#define main store_main
#include "../src/store.c"
#undef main
static int done=0,valid=0;
static void received(id self,SEL sel,id envelope){(void)self;(void)sel;id source=normalize_source(get(envelope,"data"),get(envelope,"url"));valid=source!=0;printf("HTTPS source fetch valid=%d apps=%d\n",valid,integer(get(source,"apps"),"count"));done=1;}
int main(int argc,char**argv){if(argc!=2)return 1;objc_ready();id pool=send0(send0(og_class("NSAutoreleasePool"),"alloc"),"init");(void)pool;
 id(*allocate)(id,const char*,unsigned long)=dlsym(RTLD_DEFAULT,"objc_allocateClassPair");void(*reg)(id)=dlsym(RTLD_DEFAULT,"objc_registerClassPair");signed char(*add)(id,SEL,void*,const char*)=dlsym(RTLD_DEFAULT,"class_addMethod");id cls=allocate(og_class("NSObject"),"SourceFetchTest",0);add(cls,og_sel("sourceReceived:"),(void*)received,"v@:@");reg(cls);gDelegate=send0(send0(cls,"alloc"),"init");fetch_source(string(argv[1]));id*mode=dlsym(RTLD_DEFAULT,"kCFRunLoopDefaultMode");while(!done)CFRunLoopRunInMode(*mode,1,0);return valid?0:1;}
