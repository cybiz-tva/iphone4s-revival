/* SPDX-License-Identifier: MIT */
#include "common.h"
extern long long lseek(int,long long,int);
static void (*original_pause)(id,SEL);
static int previously_live;
static long last_pause_log;
static void event(const char *text){
 int f=open(ROOT"logs/app.log",1|0x200|0x8,0644);if(f<0)return;
 if(lseek(f,0,2)>65536){close(f);unlink(ROOT"logs/app.log.previous");rename(ROOT"logs/app.log",ROOT"logs/app.log.previous");f=open(ROOT"logs/app.log",1|0x200|0x8,0644);if(f<0)return;}
 char b[180];int n=snprintf(b,sizeof(b),"%lu pid=%d %s\n",(unsigned long)time(0),getpid(),text);write(f,b,n);close(f);
}
static int live_helper(void){
 /* No helper, stale heartbeat or another PID: retain the app's normal behavior. */
 int f=open(ROOT"enabled",0);if(f<0)return 0;close(f);
 f=open(ROOT"heartbeat",0);if(f<0)return 0;
 char b[80];long n=read(f,b,79);close(f);if(n<=0)return 0;b[n]=0;
 int pid=0;unsigned long stamp=0;if(sscanf(b,"%d %lu",&pid,&stamp)!=2||pid!=getpid())return 0;
 long now=time(0);return now>=(long)stamp&&now-(long)stamp<=45;
}
static int background(void){id app=send0(og_class("UIApplication"),"sharedApplication");return app&&((int(*)(id,SEL))og_msg)(app,og_sel("applicationState"))==2;}
static void pause_hook(id self,SEL cmd){
 if(live_helper()&&background()){
  long now=time(0);if(now-last_pause_log>=30){event("preserved background connection");last_pause_log=now;}return;
 }
 if(original_pause)original_pause(self,cmd);
}
static void sample(id timer,void *info){
 (void)timer;(void)info;int live=live_helper()&&background();
 if(live&&!previously_live){id network=send0(og_class("TGTelegramNetworking"),"instance");if(network&&has(network,"resume")){send0(network,"resume");event("resumed background connection");}}
 previously_live=live;
}
__attribute__((constructor)) static void setup(void){
 if(!objc_ready())return;
 id bid=send0(send0(og_class("NSBundle"),"mainBundle"),"bundleIdentifier");
 const char *name=bid?((const char*(*)(id,SEL))og_msg)(bid,og_sel("UTF8String")):0;
 if(!name||strcmp(name,"dev.IllyaGIF.Onegram"))return;
 id cls=og_class("TGTelegramNetworking");void*(*method)(id,SEL)=dlsym(RTLD_DEFAULT,"class_getInstanceMethod");
 if(!cls||!method||!method(cls,og_sel("pause"))){event("unsupported networking class; no hook installed");return;}
 void *lib=dlopen("/Library/Frameworks/CydiaSubstrate.framework/CydiaSubstrate",2);
 void (*hook)(id,SEL,void*,void**)=lib?dlsym(lib,"MSHookMessageEx"):0;
 if(!hook){event("Substrate unavailable; no hook installed");return;}
 hook(cls,og_sel("pause"),(void*)pause_hook,(void**)&original_pause);event("Onegram notification helper v1 loaded");
 id (*create)(id,double,double,unsigned long,long,void*,void*)=dlsym(RTLD_DEFAULT,"CFRunLoopTimerCreate");
 id (*mainloop)(void)=dlsym(RTLD_DEFAULT,"CFRunLoopGetMain");
 void (*add)(id,id,id)=dlsym(RTLD_DEFAULT,"CFRunLoopAddTimer");
 double (*now)(void)=dlsym(RTLD_DEFAULT,"CFAbsoluteTimeGetCurrent");id *mode=dlsym(RTLD_DEFAULT,"kCFRunLoopCommonModes");
 if(create&&mainloop&&add&&now&&mode){id t=create(0,now()+15,15,0,0,(void*)sample,0);if(t)add(mainloop(),t,*mode);}
}
