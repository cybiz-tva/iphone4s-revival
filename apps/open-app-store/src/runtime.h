typedef void *id; typedef void *SEL;
extern void *dlopen(const char *,int); extern void *dlsym(void *,const char *);
extern int printf(const char *,...); extern int snprintf(char *,unsigned long,const char *,...);
extern int strcmp(const char *,const char *); extern int open(const char *,int,...); extern int close(int);
extern long read(int,void *,unsigned long); extern long write(int,const void *,unsigned long);
extern int sscanf(const char *,const char *,...); extern int getpid(void); extern long time(long *);
extern int unlink(const char *); extern int rename(const char *,const char *);
#define RTLD_DEFAULT ((void *)-2)
static id (*og_class)(const char *); static SEL (*og_sel)(const char *); static void *og_msg;
static int objc_ready(void) {
 dlopen("/System/Library/Frameworks/Foundation.framework/Foundation",2);
 og_class=dlsym(RTLD_DEFAULT,"objc_getClass");og_sel=dlsym(RTLD_DEFAULT,"sel_registerName");og_msg=dlsym(RTLD_DEFAULT,"objc_msgSend");return og_class&&og_sel&&og_msg;
}
static id send0(id o,const char *s) { return ((id(*)(id,SEL))og_msg)(o,og_sel(s)); }
static int has(id o,const char *s) { return ((signed char(*)(id,SEL,SEL))og_msg)(o,og_sel("respondsToSelector:"),og_sel(s)); }
static int boolean(id o,const char *s) { return ((signed char(*)(id,SEL))og_msg)(o,og_sel(s)); }
static id string(const char *s) { return ((id(*)(id,SEL,const char *))og_msg)(og_class("NSString"),og_sel("stringWithUTF8String:"),s); }
