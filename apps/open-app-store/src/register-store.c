/* SPDX-License-Identifier: MIT */
#include "runtime.h"
int main(int argc,char **argv){
 if(!objc_ready())return 1;
 dlopen("/System/Library/Frameworks/MobileCoreServices.framework/MobileCoreServices",2);
 id w=send0(og_class("LSApplicationWorkspace"),"defaultWorkspace");
 if(argc==2&&!strcmp(argv[1],"--open")){int ok=((signed char(*)(id,SEL,id))og_msg)(w,og_sel("openApplicationWithBundleID:"),string("dev.codex.OpenAppStore"));printf("Open App Store launch=%d\n",ok);return ok?0:2;}
 const char *paths[]={"/var/local/open-app-store/OpenAppStore.app"};
 int errors=0;
 for(int i=0;i<1;i++){
  char plist[256];snprintf(plist,sizeof(plist),"%s/Info.plist",paths[i]);
  id d=((id(*)(id,SEL,id))og_msg)(og_class("NSMutableDictionary"),og_sel("dictionaryWithContentsOfFile:"),string(plist));
  if(!d){errors++;continue;}
  id bid=((id(*)(id,SEL,id))og_msg)(d,og_sel("objectForKey:"),string("CFBundleIdentifier"));
  const char *name=bid?((const char*(*)(id,SEL))og_msg)(bid,og_sel("UTF8String")):0;
  if(!name||strcmp(name,"dev.codex.OpenAppStore")){printf("Refusing unexpected app bundle\n");return 3;}
  ((void(*)(id,SEL,id,id))og_msg)(d,og_sel("setObject:forKey:"),string(paths[i]),string("Path"));
  ((void(*)(id,SEL,id,id))og_msg)(d,og_sel("setObject:forKey:"),string("System"),string("ApplicationType"));
  int ok=((signed char(*)(id,SEL,id))og_msg)(w,og_sel("registerApplicationDictionary:"),d);
  printf("%s registered=%d\n",paths[i],ok);if(!ok)errors++;
 }
 return errors?2:0;
}
