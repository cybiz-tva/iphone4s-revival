/* Original Open App Store prototype. 32-bit iOS 9, no root daemon or shell bridge. */
#include "runtime.h"
extern int UIApplicationMain(int,char**,id,id);
extern unsigned long strlen(const char*);
extern char *strchr(const char*,int); extern char *strstr(const char*,const char*);
extern int strncmp(const char*,const char*,unsigned long);
extern void *malloc(unsigned long); extern void free(void*);
extern int fflush(void*); extern int CFRunLoopRunInMode(id,double,unsigned char);
extern int usleep(unsigned int);
extern long long lseek(int,long long,int);
typedef struct {float x,y,w,h;} Rect;
#define DATA_DIR "/var/mobile/Library/OpenAppStore/Downloads"
enum {IDLE,DOWNLOADING,VERIFYING,INSTALLING};
static id gDelegate,gWindow,gController,gWeb,gCatalog,gApps,gSources,gTask,gEntry;
static int gBusy,gPhase,gDone,gResult,gVerifyOnly;
static id gBuiltCatalog,gCustom;
static int gSourceBusy;
#define SOURCE_FILE "/var/mobile/Library/OpenAppStore/sources.json"
static void send_catalog(void);
static int can_install(void){
 void*security=dlopen("/System/Library/Frameworks/Security.framework/Security",2);if(!security)return 0;
 id(*create)(id)=dlsym(security,"SecTaskCreateFromSelf");id(*copy)(id,id,id*)=dlsym(security,"SecTaskCopyValueForEntitlement");void(*release)(id)=dlsym(RTLD_DEFAULT,"CFRelease");if(!create||!copy||!release)return 0;
 id task=create(0);if(!task)return 0;id value=copy(task,string("com.apple.private.mobileinstall.allowedSPI"),0);
 int enabled=value&&has(value,"containsObject:")&&((signed char(*)(id,SEL,id))og_msg)(value,og_sel("containsObject:"),string("InstallForLaunchServices"));if(value)release(value);release(task);return enabled;
}
static void note(const char*text){int fd=open("/var/mobile/Library/OpenAppStore/diagnostic.log",1|0x200|0x8,0600);if(fd>=0){char b[220];int n=snprintf(b,sizeof(b),"pid=%d %s\n",getpid(),text);write(fd,b,n);close(fd);}}
static id m1(id o,const char*s,id a){return ((id(*)(id,SEL,id))og_msg)(o,og_sel(s),a);}
static id m2(id o,const char*s,id a,id b){return ((id(*)(id,SEL,id,id))og_msg)(o,og_sel(s),a,b);}
static id get(id o,const char*k){return m1(o,"objectForKey:",string(k));}
static void put(id d,const char*k,id v){if(v)((void(*)(id,SEL,id,id))og_msg)(d,og_sel("setObject:forKey:"),v,string(k));}
static id dict(void){return send0(og_class("NSMutableDictionary"),"dictionary");}
static id num(long long n){return ((id(*)(id,SEL,long long))og_msg)(og_class("NSNumber"),og_sel("numberWithLongLong:"),n);}
static const char *utf(id o){return o&&has(o,"UTF8String")?((const char*(*)(id,SEL))og_msg)(o,og_sel("UTF8String")):"";}
static long long ll(id o,const char*s){return o?((long long(*)(id,SEL))og_msg)(o,og_sel(s)):0;}
static int integer(id o,const char*s){return o?((int(*)(id,SEL))og_msg)(o,og_sel(s)):0;}
static id at(id arr,unsigned int i){return ((id(*)(id,SEL,unsigned int))og_msg)(arr,og_sel("objectAtIndex:"),i);}
static id find(id arr,const char*identifier){for(unsigned int i=0;i<(unsigned int)integer(arr,"count");i++){id e=at(arr,i);if(!strcmp(utf(get(e,"id")),identifier))return e;}return 0;}
static id workspace(void){return send0(og_class("LSApplicationWorkspace"),"defaultWorkspace");}
static id bundle_path(id entry){if(!get(entry,"bundle"))return 0;id proxy=m1(og_class("LSApplicationProxy"),"applicationProxyForIdentifier:",get(entry,"bundle"));return send0(send0(proxy,"bundleURL"),"path");}
static id installed(void){id all=dict();for(unsigned int i=0;i<(unsigned int)integer(gApps,"count");i++){id e=at(gApps,i),v=dict();put(v,"installed",num(strlen(utf(bundle_path(e)))>0));put(all,utf(get(e,"id")),v);}return all;}
static void alert(const char*title,const char*message){
 id a=((id(*)(id,SEL,id,id,id,id,id))og_msg)(send0(og_class("UIAlertView"),"alloc"),og_sel("initWithTitle:message:delegate:cancelButtonTitle:otherButtonTitles:"),string(title),string(message),0,string("OK"),0);
 send0(a,"show");send0(a,"release");
}
static void emit(id self,SEL cmd,id event){
 (void)self;(void)cmd;
 id data=((id(*)(id,SEL,id,unsigned int,id*))og_msg)(og_class("NSJSONSerialization"),og_sel("dataWithJSONObject:options:error:"),event,0,0);
 id json=((id(*)(id,SEL,id,unsigned int))og_msg)(send0(og_class("NSString"),"alloc"),og_sel("initWithData:encoding:"),data,4);
 id js=m1(string("try{window.OAS&&OAS.receive("),"stringByAppendingString:",json);js=m1(js,"stringByAppendingString:",string(");}catch(e){window.OASLastError=String(e).substring(0,100);}"));m1(gWeb,"stringByEvaluatingJavaScriptFromString:",js);send0(json,"release");
 if(!strcmp(utf(get(event,"type")),"job")&&!integer(get(event,"busy"),"intValue")){
  __atomic_store_n(&gBusy,0,__ATOMIC_RELEASE);send0(gTask,"release");gTask=0;
  id refresh=dict();put(refresh,"type",string("installed"));put(refresh,"installed",installed());emit(0,0,refresh);
 }
}
static void post(id event){
 if(gVerifyOnly||!gDelegate){printf("%s\n",utf(get(event,"message")));fflush(0);return;}
 ((void(*)(id,SEL,SEL,id,signed char))og_msg)(gDelegate,og_sel("performSelectorOnMainThread:withObject:waitUntilDone:"),og_sel("emit:"),event,0);
}
static void job(const char*message,int busy,const char*stage,int percent){id e=dict();put(e,"type",string("job"));put(e,"name",get(gEntry,"name"));put(e,"message",string(message));put(e,"busy",num(busy));put(e,"stage",string(stage));put(e,"progress",num(percent));post(e);}
static void finish(int result,const char*message){gResult=result;__atomic_store_n(&gPhase,IDLE,__ATOMIC_RELEASE);job(message,0,result?"error":"done",result?0:100);if(gVerifyOnly)__atomic_store_n(&gDone,1,__ATOMIC_RELEASE);}
static id cache_path(id e){id name=m1(get(e,"id"),"stringByAppendingString:",string(gVerifyOnly?".verification.ipa":".ipa"));return m1(string(DATA_DIR),"stringByAppendingPathComponent:",name);}
static int file_hash(const char*path,char result[65]){
 void *lib=dlopen("/usr/lib/system/libcommonCrypto.dylib",2);
 int(*init)(void*)=dlsym(lib?lib:RTLD_DEFAULT,"CC_SHA256_Init");int(*update)(void*,const void*,unsigned int)=dlsym(lib?lib:RTLD_DEFAULT,"CC_SHA256_Update");int(*final)(unsigned char*,void*)=dlsym(lib?lib:RTLD_DEFAULT,"CC_SHA256_Final");
 if(!init||!update||!final)return 0;
 union {unsigned long long alignment;unsigned char bytes[512];} ctx;
 unsigned char buffer[65536],digest[32];int fd=open(path,0x100);if(fd<0)return 0;init(&ctx);long n;
 while((n=read(fd,buffer,sizeof(buffer)))>0)update(&ctx,buffer,(unsigned int)n);close(fd);if(n<0)return 0;final(digest,&ctx);
 for(int i=0;i<32;i++)snprintf(result+i*2,3,"%02x",digest[i]);result[64]=0;return 1;
}
/* libarchive reads only; OS installation performs the actual extraction. */
static void *arLib;
static void *(*arNew)(void);static int(*arZip)(void*),(*arFilters)(void*),(*arOpen)(void*,const char*,unsigned long),(*arNext)(void*,void**),(*arFree)(void*);
static const char*(*arPath)(void*);static const char*(*arLink)(void*);static long long(*arSize)(void*);static long(*arRead)(void*,void*,unsigned long);
static int archive_ready(void){
 if(arLib)return arNew&&arZip&&arFilters&&arOpen&&arNext&&arFree&&arPath&&arSize&&arRead&&arLink;arLib=dlopen("/usr/lib/libarchive.dylib",2);if(!arLib)return 0;
 arNew=dlsym(arLib,"archive_read_new");arZip=dlsym(arLib,"archive_read_support_format_zip");arFilters=dlsym(arLib,"archive_read_support_filter_all");arOpen=dlsym(arLib,"archive_read_open_filename");arNext=dlsym(arLib,"archive_read_next_header");arFree=dlsym(arLib,"archive_read_free");arPath=dlsym(arLib,"archive_entry_pathname");arSize=dlsym(arLib,"archive_entry_size");arRead=dlsym(arLib,"archive_read_data");arLink=dlsym(arLib,"archive_entry_symlink");
 if(!arFree)arFree=dlsym(arLib,"archive_read_finish");if(!arFilters)arFilters=dlsym(arLib,"archive_read_support_compression_all");
 return arNew&&arZip&&arFilters&&arOpen&&arNext&&arFree&&arPath&&arSize&&arRead&&arLink;
}
static void *archive_open(const char*path){void*a=arNew();if(!a)return 0;arZip(a);arFilters(a);if(arOpen(a,path,65536)!=0){arFree(a);return 0;}return a;}
static unsigned int le(const unsigned char*p){return p[0]|((unsigned int)p[1]<<8)|((unsigned int)p[2]<<16)|((unsigned int)p[3]<<24);}
static unsigned int be(const unsigned char*p){return p[3]|((unsigned int)p[2]<<8)|((unsigned int)p[1]<<16)|((unsigned int)p[0]<<24);}
static unsigned int le16(const unsigned char*p){return p[0]|((unsigned int)p[1]<<8);}
static int safe_path(const char*p){if(!p||!*p||p[0]=='/'||strchr(p,'\\'))return 0;const char*s=p;for(;;){const char*e=strchr(s,'/');unsigned long n=e?(unsigned long)(e-s):strlen(s);if(n==2&&s[0]=='.'&&s[1]=='.')return 0;if(!e)break;s=e+1;}return 1;}
static int full_read(int fd,void*data,unsigned int size){unsigned int n=0;while(n<size){long r=read(fd,(char*)data+n,size-n);if(r<=0)return 0;n+=(unsigned int)r;}return 1;}
/* Bound expanded sizes from the central directory, including streaming ZIPs
 * whose local headers legitimately omit sizes until a data descriptor. */
static int zip_bounds(const char*path){
 int fd=open(path,0x100),ok=0;if(fd<0)return 0;long long length=lseek(fd,0,2);unsigned char tail[65557],h[46];char name[1025];
 if(length<22||length>838860800)goto done;
 unsigned int tailSize=length>(long long)sizeof(tail)?sizeof(tail):(unsigned int)length;
 if(lseek(fd,length-tailSize,0)<0||!full_read(fd,tail,tailSize))goto done;
 int pos=(int)tailSize-22;for(;pos>=0;pos--)if(le(tail+pos)==0x06054b50&&pos+22+(int)le16(tail+pos+20)==(int)tailSize)break;if(pos<0)goto done;
 unsigned char*end=tail+pos;unsigned int count=le16(end+10),cdSize=le(end+12),cdOffset=le(end+16);
 if(le16(end+4)||le16(end+6)||le16(end+8)!=count||!count||count>50000||count==65535||cdSize==0xffffffff||cdOffset==0xffffffff||(long long)cdOffset+cdSize>length-tailSize+pos)goto done;
 if(lseek(fd,cdOffset,0)<0)goto done;long long expanded=0;id seen=send0(og_class("NSMutableSet"),"set");
 for(unsigned int i=0;i<count;i++){
  if(!full_read(fd,h,sizeof(h))||le(h)!=0x02014b50||(le16(h+8)&1)||le16(h+34))goto done;
  unsigned int size=le(h+24),compressed=le(h+20),n=le16(h+28),extra=le16(h+30),comment=le16(h+32),offset=le(h+42);
  if(size>1073741824||compressed==0xffffffff||!n||n>1024||(long long)offset+30+n+compressed>cdOffset||!full_read(fd,name,n))goto done;
  name[n]=0;if(strlen(name)!=n||!safe_path(name)||((le(h+38)>>16)&0170000)==0120000)goto done;
  id canonical=send0(send0(string(name),"stringByStandardizingPath"),"lowercaseString");if(!canonical||((signed char(*)(id,SEL,id))og_msg)(seen,og_sel("containsObject:"),canonical))goto done;m1(seen,"addObject:",canonical);
  expanded+=size;if(expanded>2147483648LL)goto done;
  if(lseek(fd,(long long)extra+comment,1)<0||lseek(fd,0,1)>(long long)cdOffset+cdSize)goto done;
 }
 {id fm=send0(og_class("NSFileManager"),"defaultManager");id space=((id(*)(id,SEL,id,id*))og_msg)(fm,og_sel("attributesOfFileSystemForPath:error:"),string("/var/mobile"),0);if(ll(get(space,"NSFileSystemFreeSize"),"longLongValue")<expanded*2+length+52428800)goto done;}
 ok=1;
 done:close(fd);return ok;
}
static int compatible_cpu(unsigned int cpu,unsigned int subtype){subtype&=0xffffff;return cpu==12&&(subtype==9||subtype==6||subtype==0);}
static long region(const char*ipa,const char*exe,unsigned int offset,unsigned char*out,unsigned int capacity){
 void*a=archive_open(ipa),*entry=0;if(!a)return -1;long total=-1;unsigned char scratch[16384];
 while(arNext(a,&entry)==0){if(strcmp(arPath(entry),exe))continue;unsigned int skipped=0;while(skipped<offset){unsigned int want=offset-skipped;if(want>sizeof(scratch))want=sizeof(scratch);long n=arRead(a,scratch,want);if(n<=0)goto done;skipped+=(unsigned int)n;}
  total=0;while((unsigned long)total<capacity){long n=arRead(a,out+total,capacity-(unsigned int)total);if(n<0){total=-1;break;}if(!n)break;total+=n;}break;
 }done:arFree(a);return total;
}
static unsigned int os_version(void){unsigned int a=0,b=0,c=0;sscanf(utf(send0(send0(og_class("UIDevice"),"currentDevice"),"systemVersion")),"%u.%u.%u",&a,&b,&c);return (a<<16)|(b<<8)|c;}
static int inspect_executable(const char*ipa,const char*exe){
 unsigned char b[65536];long n=region(ipa,exe,0,b,4096);if(n<28)return 0;unsigned int offset=0;
 if(be(b)==0xcafebabe){unsigned int count=be(b+4);if(!count||count>32||8+count*20>(unsigned long)n)return 0;int found=0;for(unsigned int i=0;i<count;i++){unsigned char*p=b+8+i*20;if(compatible_cpu(be(p),be(p+4))){offset=be(p+8);found=1;break;}}if(!found)return 0;}
 else if(le(b)!=0xfeedface)return 0;
 n=region(ipa,exe,offset,b,sizeof(b));if(n<28||le(b)!=0xfeedface||!compatible_cpu(le(b+4),le(b+8)))return 0;
 unsigned int commands=le(b+16),bytes=le(b+20),p=28;if(commands>1024||bytes>(unsigned long)n-28)return 0;
 for(unsigned int i=0;i<commands;i++){if(p+8>(unsigned long)n)return 0;unsigned int type=le(b+p),size=le(b+p+4);if(size<8||size>(unsigned long)n-p)return 0;
  if(type==0x21&&(size<20||le(b+p+16)))return 0; /* Encrypted App Store executables are not supported. */
  if(type==0x25&&(size<16||le(b+p+8)>os_version()))return 0;
  if(type==0x32&&(size<24||le(b+p+8)!=2||le(b+p+12)>os_version()))return 0;
  p+=size;
 }return 1;
}
static int validate_ipa(const char*ipa,id expected){
 if(!archive_ready()||!zip_bounds(ipa))return 0;void*a=archive_open(ipa),*entry=0;if(!a)return 0;
 int infos=0,valid=1,rc=0,reason=0;unsigned int entries=0;long long expanded=0;char exe[1024]={0};
 while((rc=arNext(a,&entry))==0){const char*p=arPath(entry);long long size=arSize(entry);if(!safe_path(p)||arLink(entry)||size<0||size>1073741824LL||++entries>50000){valid=0;reason=1;break;}expanded+=size;if(expanded>2147483648LL){valid=0;reason=2;break;}
  if(strncmp(p,"Payload/",8))continue;const char*slash=strchr(p+8,'/');if(!slash||strcmp(slash,"/Info.plist"))continue;
  if(++infos!=1||size>1048576){valid=0;reason=3;break;}unsigned char*raw=malloc(1048577);if(!raw){valid=0;reason=4;break;}long readCount=0;
  while(readCount<1048577){long count=arRead(a,raw+readCount,(unsigned long)(1048577-readCount));if(count<0){readCount=-1;break;}if(!count)break;readCount+=count;}
  if(readCount<=0||readCount>1048576||(size>0&&readCount!=size)){free(raw);valid=0;reason=5;break;}
  id data=((id(*)(id,SEL,const void*,unsigned long))og_msg)(og_class("NSData"),og_sel("dataWithBytes:length:"),raw,(unsigned long)readCount);free(raw);
  id info=((id(*)(id,SEL,id,unsigned int,void*,id*))og_msg)(og_class("NSPropertyListSerialization"),og_sel("propertyListWithData:options:format:error:"),data,0,0,0);
  if(!info||strcmp(utf(get(info,"CFBundleIdentifier")),utf(get(expected,"bundle")))){valid=0;reason=5;break;}
  id minimum=get(info,"MinimumOSVersion"),system=send0(send0(og_class("UIDevice"),"currentDevice"),"systemVersion");
  if(!minimum||((int(*)(id,SEL,id,unsigned int))og_msg)(minimum,og_sel("compare:options:"),system,64)>0){valid=0;reason=6;break;}
  id families=get(info,"UIDeviceFamily");if(families&&!((signed char(*)(id,SEL,id))og_msg)(families,og_sel("containsObject:"),num(1))){valid=0;reason=7;break;}
  const char*binary=utf(get(info,"CFBundleExecutable"));if(!*binary||strchr(binary,'/')||strchr(binary,'\\')||!strcmp(binary,"..")){valid=0;reason=8;break;}
  int length=snprintf(exe,sizeof(exe),"%.*s/%s",(int)(slash-p),p,binary);if(length<0||length>=(int)sizeof(exe)){valid=0;reason=9;break;}
 }
 char report[160];snprintf(report,sizeof(report),"archive check: valid=%d reason=%d rc=%d infos=%d executable=%d",valid,reason,rc,infos,*exe!=0);note(report);arFree(a);if(!(valid&&rc==1&&infos==1&&*exe))return 0;int machine=inspect_executable(ipa,exe);snprintf(report,sizeof(report),"executable check=%d",machine);note(report);return machine;
}
static void process_download(id entry,id path){
 __atomic_store_n(&gPhase,VERIFYING,__ATOMIC_RELEASE);job("Checking download integrity and iOS compatibility…",1,"verifying",100);
 id fm=send0(og_class("NSFileManager"),"defaultManager");id attrs=((id(*)(id,SEL,id,id*))og_msg)(fm,og_sel("attributesOfItemAtPath:error:"),path,0);
 long long size=ll(get(attrs,"NSFileSize"),"longLongValue");char digest[65];
 if(size!=ll(get(entry,"bytes"),"longLongValue")||!file_hash(utf(path),digest)||strcmp(digest,utf(get(entry,"sha256")))){unlink(utf(path));finish(1,"The download failed its pinned integrity check. Nothing was installed.");return;}
 if(!validate_ipa(utf(path),entry)){unlink(utf(path));finish(1,"This IPA failed bundle, ARMv7, iOS-version or archive validation. Nothing was installed.");return;}
 if(gVerifyOnly){finish(0,"Verified official download, pinned SHA-256, bundle ID, ARMv7 and iOS compatibility. No installation performed.");return;}
 if(strlen(utf(bundle_path(entry)))){finish(1,"This app is already installed. Existing apps are protected from replacement.");return;}
 __atomic_store_n(&gPhase,INSTALLING,__ATOMIC_RELEASE);job("Installing on the data partition. Keep this app open…",1,"installing",100);
 id error=0,options=dict();put(options,"CFBundleIdentifier",get(entry,"bundle"));
 signed char ok=((signed char(*)(id,SEL,id,id,id*))og_msg)(workspace(),og_sel("installApplication:withOptions:error:"),m1(og_class("NSURL"),"fileURLWithPath:",path),options,&error);
 if(!ok){char message[180];snprintf(message,sizeof(message),"iOS could not install this app (error %d). Check signing support and device compatibility.",integer(error,"code"));finish(1,message);return;}
 const char*installedPath="";for(int tries=0;tries<25;tries++){installedPath=utf(bundle_path(entry));if(!strncmp(installedPath,"/var/",5)||!strncmp(installedPath,"/private/var/",13))break;usleep(200000);}
 if(strncmp(installedPath,"/var/",5)&&strncmp(installedPath,"/private/var/",13)){finish(1,"iOS reported success, but a data-partition installation could not be verified.");return;}
 finish(0,"Installed on the data partition. You can open the app now.");
}
static void start_download(id entry){
 if(gSourceBusy||__atomic_load_n(&gBusy,__ATOMIC_ACQUIRE))return;gEntry=entry;
 if(!gVerifyOnly&&!can_install()){alert("Preview mode","App installation is not activated. Browsing and download verification are available.");return;}
 if(strcmp(utf(get(entry,"kind")),"ipa")){finish(1,"This catalog entry is not enabled for direct installation.");return;}
 if(!gVerifyOnly&&strlen(utf(bundle_path(entry)))){alert("Already installed","Existing apps are protected from replacement. Use Open instead.");return;}
 id fm=send0(og_class("NSFileManager"),"defaultManager");
 id storage=((id(*)(id,SEL,id,id*))og_msg)(fm,og_sel("attributesOfFileSystemForPath:error:"),string("/var/mobile"),0);
 if(ll(get(storage,"NSFileSystemFreeSize"),"longLongValue")<ll(get(entry,"bytes"),"longLongValue")*4+104857600){if(gVerifyOnly){finish(1,"Not enough free data-partition space.");return;}alert("Storage","More free space is needed on the data partition.");return;}
 id attributes=dict();put(attributes,"NSFilePosixPermissions",num(0700));
 ((signed char(*)(id,SEL,id,signed char,id,id*))og_msg)(fm,og_sel("createDirectoryAtPath:withIntermediateDirectories:attributes:error:"),string(DATA_DIR),1,attributes,0);
 __atomic_store_n(&gBusy,1,__ATOMIC_RELEASE);__atomic_store_n(&gDone,0,__ATOMIC_RELEASE);__atomic_store_n(&gPhase,DOWNLOADING,__ATOMIC_RELEASE);job("Downloading from the developer over HTTPS…",1,"downloading",0);
 id url=m1(og_class("NSURL"),"URLWithString:",get(entry,"url"));
 if(strcmp(utf(send0(url,"scheme")),"https")){finish(1,"Only HTTPS developer downloads are enabled.");return;}
 id configuration=send0(og_class("NSURLSessionConfiguration"),"defaultSessionConfiguration");
 m1(configuration,"setHTTPCookieStorage:",0);m1(configuration,"setURLCredentialStorage:",0);((void(*)(id,SEL,signed char))og_msg)(configuration,og_sel("setHTTPShouldSetCookies:"),0);
 ((void(*)(id,SEL,double))og_msg)(configuration,og_sel("setTimeoutIntervalForRequest:"),60.0);((void(*)(id,SEL,double))og_msg)(configuration,og_sel("setTimeoutIntervalForResource:"),600.0);
 id session=m1(og_class("NSURLSession"),"sessionWithConfiguration:",configuration);
 void (^completion)(id,id,id)=^(id location,id response,id error){
  id pool=send0(send0(og_class("NSAutoreleasePool"),"alloc"),"init");
  if(error||!location){char detail[90];snprintf(detail,sizeof(detail),"download error code=%d",integer(error,"code"));note(detail);finish(1,"Secure download failed or was cancelled. Check the network and device date.");send0(pool,"drain");return;}
  id finalURL=send0(response,"URL"),host=send0(send0(finalURL,"host"),"lowercaseString");
  if(strcmp(utf(send0(finalURL,"scheme")),"https")||!((signed char(*)(id,SEL,id))og_msg)(get(entry,"hosts"),og_sel("containsObject:"),host)||integer(response,"statusCode")!=200){finish(1,"The download redirected outside its approved developer hosts or returned an error.");send0(pool,"drain");return;}
  id destination=cache_path(entry);unlink(utf(destination));id moveError=0;
  signed char moved=((signed char(*)(id,SEL,id,id,id*))og_msg)(fm,og_sel("moveItemAtURL:toURL:error:"),location,m1(og_class("NSURL"),"fileURLWithPath:",destination),&moveError);
  if(!moved){finish(1,"The downloaded file could not be saved on the data partition.");send0(pool,"drain");return;}
  process_download(entry,destination);send0(pool,"drain");
 };
 gTask=send0(((id(*)(id,SEL,id,void*))og_msg)(session,og_sel("downloadTaskWithURL:completionHandler:"),url,(void*)completion),"retain");send0(gTask,"resume");send0(session,"finishTasksAndInvalidate");
}
static void progress(id timer,void*info){(void)timer;(void)info;if(__atomic_load_n(&gBusy,__ATOMIC_ACQUIRE)&&__atomic_load_n(&gPhase,__ATOMIC_ACQUIRE)==DOWNLOADING){long long received=ll(gTask,"countOfBytesReceived"),expected=ll(get(gEntry,"bytes"),"longLongValue");if(received>expected+1024){send0(gTask,"cancel");return;}char b[120];snprintf(b,sizeof(b),"Downloading %.1f of %.1f MB…",(double)received/1048576.0,(double)expected/1048576.0);job(b,1,"downloading",expected?(int)(received*100/expected):0);}}
static void send_catalog(void){id event=dict();put(event,"type",string("catalog"));put(event,"catalog",gCatalog);put(event,"installed",installed());put(event,"canInstall",num(can_install()));emit(0,0,event);}
static void browse(id source){id url=m1(og_class("NSURL"),"URLWithString:",get(source,"url"));if(strcmp(utf(send0(url,"scheme")),"https"))return;
 dlopen("/System/Library/Frameworks/SafariServices.framework/SafariServices",2);id cls=og_class("SFSafariViewController");if(!cls){alert("Browser","The in-app browser is unavailable.");return;}
 id vc=m1(send0(cls,"alloc"),"initWithURL:",url);m1(vc,"setDelegate:",gDelegate);((void(*)(id,SEL,id,signed char,void*))og_msg)(gController,og_sel("presentViewController:animated:completion:"),vc,1,0);send0(vc,"release");
}
static void safari_done(id self,SEL cmd,id vc){(void)self;(void)cmd;(void)vc;((void(*)(id,SEL,signed char,void*))og_msg)(gController,og_sel("dismissViewControllerAnimated:completion:"),1,0);}

/* Imported catalogs supply display data only; they cannot grant install rights. */
static int isclass(id value,const char*name){return value&&((signed char(*)(id,SEL,id))og_msg)(value,og_sel("isKindOfClass:"),og_class(name));}
static id text(id value,unsigned int limit){if(!isclass(value,"NSString"))return string("");return integer(value,"length")>(int)limit?m1(value,"substringToIndex:",(id)(unsigned long)limit):value;}
static int https_url(id value){if(!isclass(value,"NSString")||integer(value,"length")>2048)return 0;id u=m1(og_class("NSURL"),"URLWithString:",value);return !strcmp(utf(send0(u,"scheme")),"https")&&integer(send0(u,"host"),"length")>0&&!send0(u,"user")&&!send0(u,"password");}

static int package_id(id value){const char*p=utf(value);unsigned long n=strlen(p);if(n<2||n>180)return 0;for(unsigned long i=0;i<n;i++)if(!((p[i]>='a'&&p[i]<='z')||(p[i]>='0'&&p[i]<='9')||p[i]=='+'||p[i]=='-'||p[i]=='.'))return 0;return 1;}
static int firmware_allowed(id depends){
 const char*p=utf(depends);if(strchr(p,'|'))return 1; /* Alternatives need Cydia's full dependency resolver. */
 while((p=strstr(p,"firmware"))){p+=8;while(*p==' ')p++;if(*p!='(')continue;p++;while(*p==' ')p++;char op[3]={0},version[40]={0};int k=0;while((*p=='<'||*p=='>'||*p=='=')&&k<2)op[k++]=*p++;while(*p==' ')p++;k=0;while(((*p>='0'&&*p<='9')||*p=='.')&&k<39)version[k++]=*p++;
  if(!k)continue;int cmp=((int(*)(id,SEL,id,unsigned int))og_msg)(send0(send0(og_class("UIDevice"),"currentDevice"),"systemVersion"),og_sel("compare:options:"),string(version),64);
  if((!strcmp(op,">=")&&cmp<0)||(!strcmp(op,">")&&cmp<=0)||(!strcmp(op,">>")&&cmp<=0)||(!strcmp(op,"<=")&&cmp>0)||(!strcmp(op,"<")&&cmp>=0)||(!strcmp(op,"<<")&&cmp>=0)||(!strcmp(op,"=")&&cmp))return 0;
 }return 1;
}
static id parse_packages(id content,id url){
 if(!isclass(content,"NSString")||integer(content,"length")>16777216)return 0;
 id lines=m1(content,"componentsSeparatedByString:",string("\n")),apps=send0(og_class("NSMutableArray"),"array"),fields=dict(),seen=dict();unsigned int length=integer(lines,"count");
 for(unsigned int i=0;i<=length;i++){
  const char*line=i<length?utf(at(lines,i)):"";
  if(!*line||!strcmp(line,"\r")){
   id pkg=get(fields,"Package");const char*arch=utf(get(fields,"Architecture"));
   if(package_id(pkg)&&integer(get(fields,"Version"),"length")>0&&(!strcmp(arch,"iphoneos-arm")||!strcmp(arch,"all"))&&firmware_allowed(get(fields,"Depends"))&&!strstr(utf(get(fields,"Name")),"64-bit")){
    id a=dict();put(a,"name",get(fields,"Name")?get(fields,"Name"):pkg);put(a,"version",get(fields,"Version"));put(a,"summary",get(fields,"Description"));put(a,"package",pkg);put(a,"section",get(fields,"Section"));put(a,"depends",get(fields,"Depends"));put(a,"url",url);id previous=m1(seen,"objectForKey:",pkg);if(previous){unsigned int index=integer(previous,"unsignedIntValue");id old=at(apps,index);if(((int(*)(id,SEL,id,unsigned int))og_msg)(get(a,"version"),og_sel("compare:options:"),get(old,"version"),64)>0)((void(*)(id,SEL,unsigned int,id))og_msg)(apps,og_sel("replaceObjectAtIndex:withObject:"),index,a);}else{put(seen,utf(pkg),num(integer(apps,"count")));m1(apps,"addObject:",a);}if(integer(apps,"count")>=1000)break;
   }fields=dict();continue;
  }
  if(*line==' '||*line=='\t'||strlen(line)>8192)continue;const char*colon=strchr(line,':');if(!colon||colon-line>50)continue;char key[52];snprintf(key,sizeof(key),"%.*s",(int)(colon-line),line);const char*v=colon+1;while(*v==' ')v++;id value=string(v);if(has(value,"stringByTrimmingCharactersInSet:"))value=m1(value,"stringByTrimmingCharactersInSet:",send0(og_class("NSCharacterSet"),"whitespaceAndNewlineCharacterSet"));put(fields,key,value);
 }
 if(!integer(apps,"count"))return 0;id result=dict();put(result,"name",send0(m1(og_class("NSURL"),"URLWithString:",url),"host"));put(result,"format",string("apt"));put(result,"apps",apps);return result;
}
static id repository_data(id location){
 id bytes=m1(og_class("NSData"),"dataWithContentsOfURL:",location);if(integer(bytes,"length")>8388608)return 0;
 const unsigned char*raw=bytes?((const unsigned char*(*)(id,SEL))og_msg)(bytes,og_sel("bytes")):0;unsigned int n=integer(bytes,"length");
 if(n>2&&((raw[0]=='B'&&raw[1]=='Z')||(raw[0]==0x1f&&raw[1]==0x8b))){
  if(!archive_ready())return 0;int(*supportRaw)(void*)=dlsym(arLib,"archive_read_support_format_raw");if(!supportRaw)return 0;void*a=arNew();if(!a)return 0;arFilters(a);supportRaw(a);void*entry=0;
  if(arOpen(a,utf(send0(location,"path")),65536)||arNext(a,&entry)){arFree(a);return 0;}id output=send0(og_class("NSMutableData"),"data");unsigned char buffer[65536];long got;int valid=1;
  while((got=arRead(a,buffer,sizeof(buffer)))>0){if(integer(output,"length")+got>16777216){valid=0;break;}((void(*)(id,SEL,const void*,unsigned long))og_msg)(output,og_sel("appendBytes:length:"),buffer,(unsigned long)got);}arFree(a);if(!valid||got<0)return 0;bytes=output;
 }
 return ((id(*)(id,SEL,id,unsigned int))og_msg)(send0(og_class("NSString"),"alloc"),og_sel("initWithData:encoding:"),bytes,4);
}
static id normalize_source(id raw,id url){
 if(!isclass(raw,"NSDictionary")||!https_url(url))return 0;id apps=get(raw,"apps");if(!isclass(apps,"NSArray")||integer(apps,"count")>1000)return 0;
 id out=dict(),clean=send0(og_class("NSMutableArray"),"array");put(out,"url",url);int apt=!strcmp(utf(get(raw,"format")),"apt");if(apt)put(out,"format",string("apt"));id name=text(get(raw,"name"),100);put(out,"name",integer(name,"length")?name:send0(m1(og_class("NSURL"),"URLWithString:",url),"host"));
 for(unsigned int i=0;i<(unsigned int)integer(apps,"count");i++){
  id a=at(apps,i);if(!isclass(a,"NSDictionary"))continue;id title=text(get(a,"name"),100);if(!integer(title,"length"))continue;
  id e=dict();put(e,"name",title);put(e,"summary",text(get(a,"localizedDescription")?get(a,"localizedDescription"):get(a,"summary"),600));
  id version=get(a,"version"),minimum=get(a,"minOSVersion")?get(a,"minOSVersion"):get(a,"minimum");id versions=get(a,"versions");if(isclass(versions,"NSArray")&&integer(versions,"count")&&isclass(at(versions,0),"NSDictionary")){version=get(at(versions,0),"version");minimum=get(at(versions,0),"minOSVersion");}
  put(e,"version",text(version,40));put(e,"minimum",text(minimum,30));id page=get(a,"website")?get(a,"website"):get(a,"url");if(!https_url(page))page=url;put(e,"url",page);if(apt&&package_id(get(a,"package"))){put(e,"package",get(a,"package"));put(e,"section",text(get(a,"section"),80));put(e,"depends",text(get(a,"depends"),400));}m1(clean,"addObject:",e);
 }
 if(!integer(clean,"count"))return 0;put(out,"apps",clean);return out;
}
static void merge_sources(void){
 id catalog=send0(gBuiltCatalog,"mutableCopy"),apps=send0(get(gBuiltCatalog,"apps"),"mutableCopy"),sources=send0(get(gBuiltCatalog,"sources"),"mutableCopy");
 for(unsigned int i=0;i<(unsigned int)integer(gCustom,"count");i++){
  id saved=at(gCustom,i);id source=normalize_source(saved,get(saved,"url"));if(!source)continue;char sid[40];snprintf(sid,sizeof(sid),"custom%u",i);id row=dict();put(row,"id",string(sid));put(row,"name",get(source,"name"));put(row,"url",get(source,"url"));put(row,"type",string(get(source,"format")?"Cydia repository · indexed packages":"Added catalog · unreviewed"));put(row,"note",string(get(source,"format")?"Package index imported. ARM64-only and explicitly incompatible firmware entries are omitted. Cydia checks dependencies and installs packages; system storage may be used. Metadata is not a compatibility guarantee.":"Source-provided listings. Versions and compatibility are unverified; source links only. Refresh to retrieve changes."));if(get(source,"format"))put(row,"repoURL",get(source,"url"));put(row,"custom",num(1));m1(sources,"addObject:",row);
  id list=get(source,"apps");for(unsigned int j=0;j<(unsigned int)integer(list,"count");j++){
   id original=at(list,j),e=dict();char aid[64];snprintf(aid,sizeof(aid),"custom%uapp%u",i,j);put(e,"id",string(aid));put(e,"source",string(sid));put(e,"name",get(original,"name"));put(e,"summary",get(original,"summary"));put(e,"version",get(original,"version"));put(e,"kind",string("reference"));put(e,"category",string("Source apps"));put(e,"color",string("#687a87"));put(e,"url",get(original,"url"));
   if(package_id(get(original,"package"))){put(e,"kind",string("cydia"));put(e,"handoff",m1(string("cydia://package/"),"stringByAppendingString:",get(original,"package")));put(e,"package",get(original,"package"));put(e,"section",get(original,"section"));
    id built=get(gBuiltCatalog,"apps");for(unsigned int bi=0;bi<(unsigned int)integer(built,"count");bi++){id b=at(built,bi);if(get(b,"bundle")&&!strcmp(utf(send0(get(b,"bundle"),"lowercaseString")),utf(get(original,"package")))){put(e,"bundle",get(b,"bundle"));break;}}
   }
   id min=get(original,"minimum");char label[180];snprintf(label,sizeof(label),"Unreviewed source · %s%s",integer(min,"length")?"requires iOS ":"compatibility unknown",utf(min));put(e,"compat",get(original,"package")?string("32-bit repo package · review requirements in Cydia"):string(label));m1(apps,"addObject:",e);
  }
 }
 put(catalog,"apps",apps);put(catalog,"sources",sources);gCatalog=catalog;gApps=apps;gSources=sources;
}
static void save_sources(void){id d=((id(*)(id,SEL,id,unsigned int,id*))og_msg)(og_class("NSJSONSerialization"),og_sel("dataWithJSONObject:options:error:"),gCustom,0,0);((signed char(*)(id,SEL,id,signed char))og_msg)(d,og_sel("writeToFile:atomically:"),string(SOURCE_FILE),1);}
static void source_received(id self,SEL cmd,id envelope){
 (void)self;(void)cmd;gSourceBusy=0;id normalized=normalize_source(get(envelope,"data"),get(envelope,"url"));if(!normalized){alert("Source not added","No usable app catalog or legacy package index was found. Enter an HTTPS JSON catalog, a flat Cydia repo URL, or its full Packages.bz2 / Packages.gz / Packages URL. Modern-only repositories may have no compatible entries.");return;}
 id url=get(normalized,"url");int found=-1;for(unsigned int i=0;i<(unsigned int)integer(gCustom,"count");i++)if(!strcmp(utf(get(at(gCustom,i),"url")),utf(url)))found=(int)i;
 if(found>=0)((void(*)(id,SEL,unsigned int,id))og_msg)(gCustom,og_sel("replaceObjectAtIndex:withObject:"),(unsigned int)found,normalized);else if(integer(gCustom,"count")<20)m1(gCustom,"addObject:",normalized);else{alert("Source limit","Remove a source before adding another. Maximum: 20.");return;}
 save_sources();merge_sources();send_catalog();alert("Source updated","Listings are now under Apps → Source apps. Repository packages have Cydia buttons; JSON-only listings have source links. Cydia must know the repository to install its packages.");
}

static void fetch_attempt(id url,int attempt){
 id target=url;if(attempt){const char*names[]={"","Packages.bz2","Packages.gz","Packages"};id base=url;
  if(strcmp(utf(send0(m1(og_class("NSURL"),"URLWithString:",base),"query")),"")){id e=dict();put(e,"url",url);((void(*)(id,SEL,SEL,id,signed char))og_msg)(gDelegate,og_sel("performSelectorOnMainThread:withObject:waitUntilDone:"),og_sel("sourceReceived:"),e,0);return;}
  const char*t=utf(base);if(!strlen(t)||t[strlen(t)-1]!='/')base=m1(base,"stringByAppendingString:",string("/"));target=m1(base,"stringByAppendingString:",string(names[attempt]));
 }
 id configuration=send0(og_class("NSURLSessionConfiguration"),"ephemeralSessionConfiguration");m1(configuration,"setHTTPCookieStorage:",0);m1(configuration,"setURLCredentialStorage:",0);((void(*)(id,SEL,double))og_msg)(configuration,og_sel("setTimeoutIntervalForResource:"),30.0);id session=m1(og_class("NSURLSession"),"sessionWithConfiguration:",configuration);
 void(^completion)(id,id,id)=^(id location,id response,id error){id pool=send0(send0(og_class("NSAutoreleasePool"),"alloc"),"init"),raw=0;id fm=send0(og_class("NSFileManager"),"defaultManager");id attrs=((id(*)(id,SEL,id,id*))og_msg)(fm,og_sel("attributesOfItemAtPath:error:"),send0(location,"path"),0);
  if(!error&&location&&https_url(send0(send0(response,"URL"),"absoluteString"))&&integer(response,"statusCode")==200&&ll(get(attrs,"NSFileSize"),"longLongValue")<=8388608){
   if(ll(get(attrs,"NSFileSize"),"longLongValue")<=2097152){id bytes=m1(og_class("NSData"),"dataWithContentsOfURL:",location);id json=((id(*)(id,SEL,id,unsigned int,id*))og_msg)(og_class("NSJSONSerialization"),og_sel("JSONObjectWithData:options:error:"),bytes,0,0);if(isclass(json,"NSDictionary")&&isclass(get(json,"apps"),"NSArray"))raw=json;}
   if(!raw)raw=parse_packages(repository_data(location),url);
  }
  if(!raw&&attempt<3){fetch_attempt(url,attempt+1);send0(pool,"drain");return;}
  id envelope=dict();put(envelope,"url",url);if(raw)put(envelope,"data",raw);((void(*)(id,SEL,SEL,id,signed char))og_msg)(gDelegate,og_sel("performSelectorOnMainThread:withObject:waitUntilDone:"),og_sel("sourceReceived:"),envelope,0);send0(pool,"drain");};
 id task=((id(*)(id,SEL,id,void*))og_msg)(session,og_sel("downloadTaskWithURL:completionHandler:"),m1(og_class("NSURL"),"URLWithString:",target),(void*)completion);send0(task,"resume");send0(session,"finishTasksAndInvalidate");
}
static void fetch_source(id url){
 if(gSourceBusy){alert("Source download","Wait for the current source update.");return;}if(!https_url(url)){alert("Source URL","Enter a complete HTTPS catalog or repository URL without a username or password.");return;}if(__atomic_load_n(&gBusy,__ATOMIC_ACQUIRE)){alert("Installation active","Finish the installation before updating sources.");return;}
 gSourceBusy=1;fetch_attempt(url,0);
}

static void clicked(id self,SEL cmd,id view,int index){(void)self;(void)cmd;(void)view;if(index==1&&gEntry)start_download(gEntry);}
static void confirm(id entry){
 if(gSourceBusy||__atomic_load_n(&gBusy,__ATOMIC_ACQUIRE))return;gEntry=entry;char text[400];
 snprintf(text,sizeof(text),"%s %s\nDeveloper-hosted download · %.1f MB\nPinned SHA-256 and compatibility checks.\nInstalls on the data partition. Keep this app open.",utf(get(entry,"name")),utf(get(entry,"version")),(double)ll(get(entry,"bytes"),"longLongValue")/1048576.0);
 /* Varargs initializer requires a final nil after the Install button. */
 id a=((id(*)(id,SEL,id,id,id,id,id,id))og_msg)(send0(og_class("UIAlertView"),"alloc"),og_sel("initWithTitle:message:delegate:cancelButtonTitle:otherButtonTitles:"),string("Install app?"),string(text),gDelegate,string("Cancel"),string("Install"),0);send0(a,"show");send0(a,"release");
}
static void command(id url){
 const char*action=utf(send0(url,"host"));id query=send0(url,"query");const char*q=utf(query);char identifier[100]={0};
 if(!strcmp(action,"ready")||!strcmp(action,"refresh"))note("local UI bridge ready");
 if(!strcmp(action,"addsource")&&!strncmp(q,"id=",3)){fetch_source(send0(string(q+3),"stringByRemovingPercentEncoding"));return;}
 if(!strncmp(q,"id=",3)){id raw=string(q+3);id decoded=send0(raw,"stringByRemovingPercentEncoding");snprintf(identifier,sizeof(identifier),"%s",utf(decoded));}
 id entry=find(gApps,identifier),source=find(gSources,identifier);
 if(!strcmp(action,"ready")||!strcmp(action,"refresh")){send_catalog();return;}
 if(!strcmp(action,"cancel")){if(__atomic_load_n(&gPhase,__ATOMIC_ACQUIRE)==DOWNLOADING)send0(gTask,"cancel");return;}
 if(!strcmp(action,"install")&&entry&&!strcmp(utf(get(entry,"kind")),"ipa")){confirm(entry);return;}
 if(!strcmp(action,"open")&&entry&&strlen(utf(bundle_path(entry)))){if(!((signed char(*)(id,SEL,id))og_msg)(workspace(),og_sel("openApplicationWithBundleID:"),get(entry,"bundle")))alert("Unable to open","iOS could not open this app.");return;}
 if((!strcmp(action,"removesource")||!strcmp(action,"updatesource"))&&source&&get(source,"custom")){
 if(gSourceBusy||__atomic_load_n(&gBusy,__ATOMIC_ACQUIRE)){alert("Busy","Wait for the current operation to finish.");return;}
 if(!strcmp(action,"updatesource")){fetch_source(get(source,"url"));return;}
 for(unsigned int i=0;i<(unsigned int)integer(gCustom,"count");i++)if(!strcmp(utf(get(at(gCustom,i),"url")),utf(get(source,"url")))){((void(*)(id,SEL,unsigned int))og_msg)(gCustom,og_sel("removeObjectAtIndex:"),i);break;}save_sources();merge_sources();send_catalog();return;}
 if(!strcmp(action,"importrepo")&&source&&https_url(get(source,"repoURL"))){fetch_source(get(source,"repoURL"));return;}
 if(!strcmp(action,"addtocydia")&&source&&get(source,"repoURL")){id repo=get(source,"repoURL");if(!https_url(repo)){alert("Legacy HTTP repo","Copy this repository URL and review its insecure transport in Cydia.");return;}id encoded=m1(repo,"stringByAddingPercentEncodingWithAllowedCharacters:",m1(og_class("NSCharacterSet"),"characterSetWithCharactersInString:",string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~")));id address=m1(string("cydia://url/https://cydia.saurik.com/api/share#?source="),"stringByAppendingString:",encoded);((signed char(*)(id,SEL,id))og_msg)(send0(og_class("UIApplication"),"sharedApplication"),og_sel("openURL:"),m1(og_class("NSURL"),"URLWithString:",address));return;}
 if(!strcmp(action,"source")&&source){browse(source);return;}
 if(!strcmp(action,"browseapp")&&entry){if(get(entry,"url"))browse(entry);else browse(find(gSources,utf(get(entry,"source"))));return;}
 if(!strcmp(action,"copy")&&source&&get(source,"repoURL")){m1(send0(og_class("UIPasteboard"),"generalPasteboard"),"setString:",get(source,"repoURL"));alert("Copied","The repository URL is on the clipboard. Review compatibility in Cydia before adding packages.");return;}
 if(!strcmp(action,"handoff")&&entry){const char*kind=utf(get(entry,"kind"));if(strcmp(kind,"apple")&&strcmp(kind,"cydia"))return;id target=m1(og_class("NSURL"),"URLWithString:",get(entry,"handoff"));const char*scheme=utf(send0(target,"scheme"));if(strcmp(scheme,"itms-apps")&&strcmp(scheme,"cydia"))return;((signed char(*)(id,SEL,id))og_msg)(send0(og_class("UIApplication"),"sharedApplication"),og_sel("openURL:"),target);return;}
 if(!strcmp(action,"clearcache")&&!__atomic_load_n(&gBusy,__ATOMIC_ACQUIRE)){for(unsigned int i=0;i<(unsigned int)integer(gApps,"count");i++){id e=at(gApps,i);unlink(utf(cache_path(e)));id name=m1(get(e,"id"),"stringByAppendingString:",string(".verification.ipa"));unlink(utf(m1(string(DATA_DIR),"stringByAppendingPathComponent:",name)));}alert("Downloads cleared","Only this store's downloaded IPAs were removed. Installed apps and account data were preserved.");}
}
static signed char navigation(id self,SEL cmd,id web,id request,int type){
 (void)self;(void)cmd;(void)web;(void)type;id url=send0(request,"URL");const char*scheme=utf(send0(url,"scheme"));
 if(!strcmp(scheme,"oas")){command(url);return 0;}if(!strcmp(scheme,"about"))return !strcmp(utf(send0(url,"absoluteString")),"about:blank");
 if(!strcmp(scheme,"file")){id actual=send0(send0(send0(url,"path"),"stringByStandardizingPath"),"stringByResolvingSymlinksInPath");id root=send0(send0(send0(og_class("NSBundle"),"mainBundle"),"bundlePath"),"stringByResolvingSymlinksInPath");const char*path=utf(actual),*base=utf(root);unsigned long n=strlen(base);return n&&!strncmp(path,base,n)&&(path[n]=='/'||path[n]==0);}
 return 0; /* Remote HTML never enters the privileged UIWebView. */
}
static void loaded(id self,SEL cmd,id web){static int checked=0;(void)self;(void)cmd;send_catalog();id result=m1(web,"stringByEvaluatingJavaScriptFromString:",string("(function(){if(!window.OAS)return 'ui_ready=false';if(window.OASLastError)return window.OASLastError;var a=document.getElementsByClassName('card'),n=a.length,o='not found';for(var i=0;i<a.length;i++){var h=a[i].getElementsByTagName('h3')[0];if(h&&h.textContent==='Onegram')o=a[i].getElementsByTagName('button')[0].textContent;}OAS.tab('sources');var s=document.getElementsByClassName('card').length;OAS.tab('apps');return 'ui_ready=true apps='+n+' sources='+s+' Onegram='+o;})()"));note(utf(result));if(!checked){checked=1;m1(web,"stringByEvaluatingJavaScriptFromString:",string("OAS.command('refresh')"));}}
static signed char launch(id self,SEL cmd,id app,id options){
 note("application launched; local UI loading");
 (void)cmd;(void)app;(void)options;gDelegate=self;
 gWindow=((id(*)(id,SEL,Rect))og_msg)(send0(og_class("UIWindow"),"alloc"),og_sel("initWithFrame:"),(Rect){0,0,320,480});
 gController=send0(send0(og_class("UIViewController"),"alloc"),"init");((void(*)(id,SEL,signed char))og_msg)(gController,og_sel("setAutomaticallyAdjustsScrollViewInsets:"),0);
 id view=send0(gController,"view");m1(view,"setBackgroundColor:",send0(og_class("UIColor"),"whiteColor"));
 gWeb=((id(*)(id,SEL,Rect))og_msg)(send0(og_class("UIWebView"),"alloc"),og_sel("initWithFrame:"),(Rect){0,20,320,460});m1(gWeb,"setDelegate:",self);m1(view,"addSubview:",gWeb);
 m1(gWindow,"setRootViewController:",gController);send0(gWindow,"makeKeyAndVisible");
 id bundle=send0(og_class("NSBundle"),"mainBundle"),base=send0(bundle,"bundlePath"),htmlPath=m1(base,"stringByAppendingPathComponent:",string("index.html"));
 id html=((id(*)(id,SEL,id,unsigned int,id*))og_msg)(og_class("NSString"),og_sel("stringWithContentsOfFile:encoding:error:"),htmlPath,4,0);
 m2(gWeb,"loadHTMLString:baseURL:",html,m1(og_class("NSURL"),"fileURLWithPath:",m1(base,"stringByAppendingString:",string("/"))));
 id (*create)(id,double,double,unsigned long,long,void*,void*)=dlsym(RTLD_DEFAULT,"CFRunLoopTimerCreate");id(*mainloop)(void)=dlsym(RTLD_DEFAULT,"CFRunLoopGetMain");void(*add)(id,id,id)=dlsym(RTLD_DEFAULT,"CFRunLoopAddTimer");double(*now)(void)=dlsym(RTLD_DEFAULT,"CFAbsoluteTimeGetCurrent");id*mode=dlsym(RTLD_DEFAULT,"kCFRunLoopCommonModes");if(create&&mainloop&&add&&now&&mode){id timer=create(0,now()+1,1,0,0,(void*)progress,0);add(mainloop(),timer,*mode);}return 1;
}
static void active(id self,SEL cmd,id app){(void)self;(void)cmd;(void)app;if(gWeb)send_catalog();}
int main(int argc,char**argv){
 if(!objc_ready())return 1;dlopen("/System/Library/Frameworks/MobileCoreServices.framework/MobileCoreServices",2);
 id pool=send0(send0(og_class("NSAutoreleasePool"),"alloc"),"init");(void)pool;
 id path=m1(send0(send0(og_class("NSBundle"),"mainBundle"),"bundlePath"),"stringByAppendingPathComponent:",string("catalog.json"));id data=m1(og_class("NSData"),"dataWithContentsOfFile:",path);
 gCatalog=send0(((id(*)(id,SEL,id,unsigned int,id*))og_msg)(og_class("NSJSONSerialization"),og_sel("JSONObjectWithData:options:error:"),data,0,0),"retain");if(!gCatalog)return 2;gBuiltCatalog=gCatalog;id customData=m1(og_class("NSData"),"dataWithContentsOfFile:",string(SOURCE_FILE));id saved=customData?((id(*)(id,SEL,id,unsigned int,id*))og_msg)(og_class("NSJSONSerialization"),og_sel("JSONObjectWithData:options:error:"),customData,0,0):0;gCustom=isclass(saved,"NSArray")&&integer(saved,"count")<=20?send0(saved,"mutableCopy"):send0(send0(og_class("NSMutableArray"),"alloc"),"init");merge_sources();note("local catalog parsed");
 if(argc==3&&!strcmp(argv[1],"--check-source")){id bytes=m1(og_class("NSData"),"dataWithContentsOfFile:",string(argv[2]));id raw=((id(*)(id,SEL,id,unsigned int,id*))og_msg)(og_class("NSJSONSerialization"),og_sel("JSONObjectWithData:options:error:"),bytes,0,0);id result=normalize_source(raw,string("https://example.org/catalog.json"));printf("source_valid=%d apps=%d\n",result!=0,integer(get(result,"apps"),"count"));return result?0:1;}
 if(argc==2&&!strcmp(argv[1],"--capabilities")){printf("installer_rights=%d archive_support=%d\n",can_install(),archive_ready());return 0;}
 if(argc==3&&!strcmp(argv[1],"--verify-catalog")){gVerifyOnly=1;id e=find(gApps,argv[2]);if(!e||strcmp(utf(get(e,"kind")),"ipa"))return 3;start_download(e);id*mode=dlsym(RTLD_DEFAULT,"kCFRunLoopDefaultMode");while(!__atomic_load_n(&gDone,__ATOMIC_ACQUIRE))CFRunLoopRunInMode(*mode,1,0);return gResult;}
 id(*allocate)(id,const char*,unsigned long)=dlsym(RTLD_DEFAULT,"objc_allocateClassPair");void(*registerClass)(id)=dlsym(RTLD_DEFAULT,"objc_registerClassPair");signed char(*add)(id,SEL,void*,const char*)=dlsym(RTLD_DEFAULT,"class_addMethod");
 id cls=allocate(og_class("NSObject"),"OpenAppStoreDelegate",0);add(cls,og_sel("application:didFinishLaunchingWithOptions:"),(void*)launch,"c@:@@");add(cls,og_sel("applicationDidBecomeActive:"),(void*)active,"v@:@");add(cls,og_sel("webView:shouldStartLoadWithRequest:navigationType:"),(void*)navigation,"c@:@@i");add(cls,og_sel("webViewDidFinishLoad:"),(void*)loaded,"v@:@");add(cls,og_sel("emit:"),(void*)emit,"v@:@");add(cls,og_sel("alertView:clickedButtonAtIndex:"),(void*)clicked,"v@:@i");add(cls,og_sel("safariViewControllerDidFinish:"),(void*)safari_done,"v@:@");add(cls,og_sel("sourceReceived:"),(void*)source_received,"v@:@");registerClass(cls);
 return UIApplicationMain(argc,argv,0,string("OpenAppStoreDelegate"));
}
