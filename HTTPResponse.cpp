//
// Created by HP on 2022/2/14.
//
#include "HTTPResponse.h"
#include <ctime>
#include <time.h>
#include <fstream>
#define CacheControlKEYWORDLEN 15

//#define DEBUG_MODE
#ifdef DEBUG_MODE
#include "gtest/gtest.h"
#endif

using namespace std;

/**
 * Find date
 */
void HTTPResponse::parseDate(){
    std::transform(response.begin(),response.end(),response.begin(),::tolower);
    size_t find_date=response.find("date: ");
    if(find_date!=string::npos){
        size_t find_gmt=response.find("\r\n",find_date+1);//TODO +1
        if(find_gmt!=string::npos){
            string datetime=response.substr(find_date+6,find_gmt-find_date-6); //TODO +6
            tm mytime;
            strptime(datetime.c_str(),"%a, %d %b %Y %H:%M:%S", &mytime);
            response_time=mktime(&mytime);
        }
    }
}

// TODO May become parse cache-control in future
void HTTPResponse::parseMax(){
    std::transform(response.begin(),response.end(),response.begin(),::tolower);
    size_t find_cache=response.find("cache-control: ");
    if(find_cache!=string::npos) {
        size_t end = response.find_first_of("\r\n", find_cache + 1);
        if(end!=string::npos){
            string temp = response.substr(find_cache + CacheControlKEYWORDLEN, end - find_cache - CacheControlKEYWORDLEN);
            size_t len=temp.size();
            size_t find_max = response.find("max-age=",find_cache+1);
            if (find_max != string::npos) {
                size_t find_colon = response.find(",", find_max + 1);
                string max_age;
                if (find_colon == string::npos) {
                    max_age=response.substr(find_max+8,len-8);
                }
                else max_age = response.substr(find_max + 8, find_colon - find_max - 8);// TODO the magic value; invalid use of find_colon
                max = atoi(max_age.c_str());
            }
        }
    }
}

void HTTPResponse::parseExpire(){
    std::transform(response.begin(),response.end(),response.begin(),::tolower);
    size_t expire_pos=response.find("expires: ");
    if(expire_pos!=string::npos){
        size_t find_gmt=response.find_first_of("\r\n",expire_pos+1);
        if(find_gmt!=string::npos){
            string datetime=response.substr(expire_pos+9,find_gmt-expire_pos-9);
            cout<<"find_gmt: "<<find_gmt<<"  expire_pos"<<expire_pos<<endl;
            tm mytime;
            strptime(datetime.c_str(),"%a, %d %b %Y %H:%M:%S", &mytime);
            expire_time=mktime(&mytime);
        }

    }
}

void HTTPResponse::parseCache(){
    std::transform(response.begin(),response.end(),response.begin(),::tolower);
    size_t no_cache=response.find("no-cache");
    if(no_cache==string::npos){
        no_cache=false;
    }else{
        no_cache=true;
    }
}

void HTTPResponse::parseEtag(){
    std::transform(response.begin(),response.end(),response.begin(),::tolower);
    size_t find_etag=response.find("etag: ");
    if(find_etag==string::npos){
        Etag="";
    }else{
        size_t end=response.find_first_of("\"",find_etag+7);
        Etag=response.substr(find_etag+7,end-find_etag-7); // TODO fix end
    }
}

void HTTPResponse::parsemodify(){
    std::transform(response.begin(),response.end(),response.begin(),::tolower);
    size_t find_etag=response.find("last-modified: ");
    if(find_etag==string::npos) {
        last_modified="";
    }else{
        size_t end=response.find("\r\n",find_etag+1);
        last_modified=response.substr(find_etag+15,end-find_etag-15); // TODO fix end
    }
}

bool HTTPResponse::isChunked() const {
    return chunked;
}




//// Test case 1:
//TEST(HTTPResponseTest, test1){
//    std::ifstream f("/Users/xg/CLionProjects/http-caching-proxy/Examples/response1.txt");
//    std::stringstream reader;
//    reader << f.rdbuf();
//    HTTPResponse reqs(reader.str());
//    EXPECT_STREQ("Mon, 21 Feb 2022 21:05:37",asctime(gmtime(&reqs.expire_time)));
//}






