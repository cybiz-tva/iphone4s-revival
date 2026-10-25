/* SPDX-License-Identifier: MIT */
#include "common.h"
extern int atoi(const char *); extern unsigned int sleep(unsigned int);
extern int fflush(void *);
extern void (*signal(int,void(*)(int)))(int);
extern int CFRunLoopRunInMode(id,double,unsigned char);
static volatile int running=1, acquired=-1;
static void stop(int n){(void)n;running=0;}
static void released(id a){if(a){send0(a,"invalidate");send0(a,"release");}}
static int find_onegram(void){
 /* Never launch an app or accept an arbitrary PID/path from configuration. */
 int (*list)(void*,int)=dlsym(RTLD_DEFAULT,"proc_listallpids");
 int (*path)(int,void*,unsigned int)=dlsym(RTLD_DEFAULT,"proc_pidpath");
 if(!list||!path)return 0;
 int pids[2048],n=list(pids,sizeof(pids)); if(n>2048)n=2048;
 for(int i=0;i<n;i++){char p[4096];int len=path(pids[i],p,sizeof(p));if(len<=0)continue;p[4095]=0;
  if(!strcmp(p,"/Applications/Onegram.app/Telegram.real")||!strcmp(p,"/var/local/relocated-apps/Onegram.app/Telegram.real")||!strcmp(p,"/private/var/local/relocated-apps/Onegram.app/Telegram.real"))return pids[i];
 }return 0;
}
static void heartbeat(int pid){char data[80];int n=snprintf(data,sizeof(data),"%d %lu\n",pid,(unsigned long)time(0));int f=open(ROOT"heartbeat.tmp",1|0x200|0x400,0644);if(f>=0){write(f,data,n);close(f);rename(ROOT"heartbeat.tmp",ROOT"heartbeat");}}
int main(int argc,char **argv){
 int daemon=argc==2&&!strcmp(argv[1],"--daemon");
 int duration=daemon?0:((argc==2)?atoi(argv[1]):600);if(!daemon&&(duration<30||duration>900)){printf("duration must be 30..900 seconds, or --daemon\n");return 2;}
 if(!objc_ready())return 3;
 dlopen("/System/Library/PrivateFrameworks/AssertionServices.framework/AssertionServices",2);
 dlopen("/System/Library/PrivateFrameworks/BackBoardServices.framework/BackBoardServices",2);
 id cls=og_class("BKSProcessAssertion");if(!cls)return 4;
 id *mode=dlsym(RTLD_DEFAULT,"kCFRunLoopDefaultMode");if(!mode)return 5;
 signal(15,stop);signal(2,stop);long deadline=time(0)+duration;id assertion=0;int tracked=0,lastValid=-1;
 printf("OGKA duration=%d daemon=%d target=Onegram-only flags=3 reason=10004\n",duration,daemon);fflush(0);
 while(running&&(daemon||time(0)<deadline)){
  id pool=send0(send0(og_class("NSAutoreleasePool"),"alloc"),"init");
  int enabled=open(ROOT"enabled",0);if(enabled<0){send0(pool,"drain");break;}close(enabled);
  int pid=find_onegram();
  if(pid!=tracked){released(assertion);assertion=0;tracked=pid;acquired=-1;lastValid=-1;unlink(ROOT"heartbeat");
   if(pid){
    void (^handler)(signed char)=^(signed char ok){acquired=ok?1:0;};
    /* 3 = PreventTaskSuspend | PreventTaskThrottleDown; 10004 = FinishTaskUnbounded. */
    assertion=((id(*)(id,SEL,int,unsigned int,unsigned int,id,void* ))og_msg)(send0(cls,"alloc"),og_sel("initWithPID:flags:reason:name:withHandler:"),pid,3,10004,string("Onegram notification helper"),(void*)handler);
    printf("target_pid=%d assertion_created=%d\n",pid,assertion!=0);
   }else printf("Onegram not running; no launch requested\n");
  }
  for(int i=0;i<10&&running&&(daemon||time(0)<deadline);i++)CFRunLoopRunInMode(*mode,1.0,0);
  int valid=assertion&&has(assertion,"valid")?boolean(assertion,"valid"):0;
  if(valid!=lastValid){printf("assertion_valid=%d callback=%d pid=%d\n",valid,acquired,tracked);lastValid=valid;}
  if(valid&&tracked)heartbeat(tracked);else unlink(ROOT"heartbeat");
  if(assertion&&!valid){released(assertion);assertion=0;tracked=0;}
  fflush(0);
  send0(pool,"drain");
 }
 unlink(ROOT"heartbeat");released(assertion);printf("OGKA stopped; assertion released\n");return 0;
}
