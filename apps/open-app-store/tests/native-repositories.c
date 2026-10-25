#define main store_main
#include "../src/store.c"
#undef main
int main(void){
 objc_ready();id pool=send0(send0(og_class("NSAutoreleasePool"),"alloc"),"init");(void)pool;
 const char*fixture="Package: example.old\nName: Legacy App\nArchitecture: iphoneos-arm\nVersion: 1.0\nDepends: firmware (>= 5.0), firmware (<< 10.0)\n\nPackage: example.new\nName: Modern App\nArchitecture: iphoneos-arm\nVersion: 2\nDepends: firmware (>= 11.0)\n\nPackage: example.rootless\nArchitecture: iphoneos-arm64\n\nPackage: example.tooold\nArchitecture: iphoneos-arm\nDepends: firmware (<= 7.1.2)\n\nPackage: bad/name\nArchitecture: iphoneos-arm\n\nPackage: example.old\nName: Legacy App\nArchitecture: iphoneos-arm\nVersion: 1.10\nDepends: firmware (>= 8.0)\n\nPackage: example.all\nName: Data\nArchitecture: all\nVersion: 1\n\n";
 id parsed=parse_packages(string(fixture),string("https://example.org/"));if(integer(get(parsed,"apps"),"count")!=2)return 1;id app=at(get(parsed,"apps"),0);if(strcmp(utf(get(app,"version")),"1.10"))return 2;
 id norm=normalize_source(parsed,string("https://example.org/"));if(strcmp(utf(get(at(get(norm,"apps"),0),"package")),"example.old"))return 3;
 gBuiltCatalog=dict();put(gBuiltCatalog,"apps",send0(og_class("NSArray"),"array"));put(gBuiltCatalog,"sources",send0(og_class("NSArray"),"array"));gCustom=send0(og_class("NSMutableArray"),"array");m1(gCustom,"addObject:",norm);merge_sources();app=at(gApps,0);if(strcmp(utf(get(app,"kind")),"cydia")||strcmp(utf(get(app,"handoff")),"cydia://package/example.old")||get(app,"sha256")||get(app,"hosts"))return 4;
 printf("Repository tests passed: ARM64 / firmware / invalid IDs filtered; duplicates merged; Cydia-only handoff\n");return 0;
}
