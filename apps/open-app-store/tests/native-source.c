#define main store_main
#include "../src/store.c"
#undef main
int main(void){
 objc_ready();id pool=send0(send0(og_class("NSAutoreleasePool"),"alloc"),"init");(void)pool;
 const char*fixtures[]={"{\"name\":\"Example\",\"apps\":[{\"name\":\"Demo\",\"versions\":[{\"version\":\"1\",\"minOSVersion\":\"15\"}],\"kind\":\"ipa\",\"bundle\":\"dev.IllyaGIF.Onegram\",\"url\":\"javascript:alert(1)\"}]}","[]","{\"apps\":\"wrong\"}","{\"apps\":[null,4,{}, {\"name\":\"Valid\"}]}","{\"apps\":[]}"};int expected[]={1,0,0,1,0};
 for(int i=0;i<5;i++){id bytes=m2(string(fixtures[i]),"dataUsingEncoding:",(id)4,0);id raw=((id(*)(id,SEL,id,unsigned int,id*))og_msg)(og_class("NSJSONSerialization"),og_sel("JSONObjectWithData:options:error:"),bytes,0,0);id normalized=normalize_source(raw,string("https://example.org/catalog.json"));printf("case=%d accepted=%d expected=%d\n",i,normalized!=0,expected[i]);if((normalized!=0)!=expected[i])return 1;if(normalized){id a=at(get(normalized,"apps"),0);if(get(a,"kind")||get(a,"bundle")||!https_url(get(a,"url")))return 2;id again=normalize_source(normalized,get(normalized,"url"));if(!again)return 3;}}
 if(https_url(string("http://example.org/a"))||https_url(string("https://user:pw@example.org/a")))return 4;
 printf("Source normalization guards passed\n");return 0;
}
