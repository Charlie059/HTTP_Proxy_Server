//
// Created by HP on 2022/2/14.
//
#include "HTTPResponse.h"
#include "Time.h"
#include <ctime>
#include <time.h>
#include <fstream>
#define CacheControlKEYWORDLEN 15

//#define DEBUG_MODE
#ifdef DEBUG_MODE
#include "gtest/gtest.h"
// Test case 1:
TEST(HTTPResponseTest, test1){
    std::ifstream f("/Users/xg/CLionProjects/http-caching-proxy/Examples/response3.txt");
    std::stringstream reader;
    reader << f.rdbuf();
    HTTPResponse reqs(reader.str());
    EXPECT_EQ(false,reqs.isNoStore());
}

#endif

using namespace std;

bool HTTPResponse::findNotModified(std::string response){
    size_t find_line = response.find("HTTP/1.1 304 Not Modified");
    if(find_line!=string::npos) return true;
    return false;
}

//bool HTTPResponse::time_valid(std::string response){
//
//}


//TODO WARNING DO NOT CHANGE RAW DATA
/**
 * Find date
 */
void HTTPResponse::parseDate(){
    string response_temp = this->response;
    std::transform(response_temp.begin(),response_temp.end(),response_temp.begin(),::tolower);
    size_t find_date=response_temp.find("date: ");
    if(find_date!=string::npos){
        size_t find_gmt=response_temp.find("\r\n",find_date+1);//TODO +1
        if(find_gmt!=string::npos){
            string datetime=response_temp.substr(find_date+6,find_gmt-find_date-6); //TODO +6
            tm mytime;
            strptime(datetime.c_str(),"%a, %d %b %Y %H:%M:%S", &mytime);
            response_time=mktime(&mytime);//TODO ???
        }
    }
}

// TODO May become parse cache-control in future
void HTTPResponse::parseMaxAge(){
    string response_temp = this->response;
    std::transform(response_temp.begin(),response_temp.end(),response_temp.begin(),::tolower);
    size_t find_cache=response_temp.find("cache-control: ");
    if(find_cache!=string::npos) {
        size_t end = response_temp.find_first_of("\r\n", find_cache + 1);
        if(end!=string::npos){
            string temp = response_temp.substr(find_cache + CacheControlKEYWORDLEN, end - find_cache - CacheControlKEYWORDLEN);
            size_t len=temp.size();
            size_t find_max = response_temp.find("max-age=",find_cache+1);
            if (find_max != string::npos) {
                size_t find_colon = response_temp.find(",", find_max + 1);
                string max_age;
                if (find_colon == string::npos) {
                    max_age=response.substr(find_max+8,len-8);
                }
                else max_age = response_temp.substr(find_max + 8, find_colon - find_max - 8);// TODO the magic value; invalid use of find_colon
                max = atoi(max_age.c_str());
            }
        }
    }
}


void HTTPResponse::parseExpire(){
    string response_temp = this->response;
    std::transform(response_temp.begin(),response_temp.end(),response_temp.begin(),::tolower);
    size_t expire_pos=response_temp.find("expires: ");
    if(expire_pos!=string::npos){
        size_t find_gmt=response_temp.find_first_of("\r\n",expire_pos+1);
        if(find_gmt!=string::npos){
            string datetime=response_temp.substr(expire_pos+9,find_gmt-expire_pos-9); // TODO the magic value; invalid use of find_colon
            tm mytime;
            strptime(datetime.c_str(),"%a, %d %b %Y %H:%M:%S", &mytime);
            expire_time=mktime(&mytime);
            return;
        }
    }
    expire_time = 0; // Thu Jan  1 00:00:00 1970
}

void HTTPResponse::parseCache(){
    string response_temp = this->response;
    std::transform(response_temp.begin(),response_temp.end(),response_temp.begin(),::tolower);
    size_t no_cache=response_temp.find("no-cache");
    if(no_cache==string::npos){
        no_cache=false;
    }else{
        no_cache=true;
    }
}

void HTTPResponse::parseEtag(){
    string response_temp = this->response;
    std::transform(response_temp.begin(),response_temp.end(),response_temp.begin(),::tolower);
    size_t find_etag=response.find("etag: ");
    if(find_etag==string::npos){
        Etag="";
    }else{
        size_t end=response.find_first_of("\"",find_etag+7);
        Etag=response.substr(find_etag+7,end-find_etag-7); // TODO fix end
    }
}

void HTTPResponse::parsemodify(){
    string response_temp = this->response;
    std::transform(response_temp.begin(),response_temp.end(),response_temp.begin(),::tolower);
    size_t find_etag=response_temp.find("last-modified: ");
    if(find_etag==string::npos) {
        last_modified="";
    }else{
        size_t end=response_temp.find("\r\n",find_etag+1);
        last_modified=response_temp.substr(find_etag+15,end-find_etag-15); // TODO fix end
    }
}

bool HTTPResponse::isChunked() const {
    return chunked;
}

const string &HTTPResponse::getLine() const {
    return line;
}

//TODO find content length, ensure only set data for ContentLength or -1 and careful of find
void HTTPResponse::parseContentLength() {
    string response_temp = this->response;
    std::transform(response_temp.begin(),response_temp.end(),response_temp.begin(),::tolower);

}

int HTTPResponse::getContentLength() const {
    return content_length;
}


string HTTPResponse::buildResponse(int HTTPCODE){
    switch (HTTPCODE) {
        case 200:
            return "HTTP/1.1 200 OK\n"
                   "Content-Type: application/json; charset=UTF-8\n"
                   "Content-Length: 0\n"
                   "\r\n";
        case 400:
            return "HTTP/1.1 400 Bad Request\n"
                   "Content-Type: application/json; charset=UTF-8\n"
                   "Content-Length: 0\n"
                   "\r\n";
        default:
            return "HTTP/1.1 400 Bad Request\n"
                   "Content-Type: application/json; charset=UTF-8\n"
                   "Content-Length: 0\n"
                   "\r\n";
    }
}

bool HTTPResponse::isNoCache() const {
    return no_cache;
}

bool HTTPResponse::isNoStore() const {
    return no_store;
}

bool HTTPResponse::isPrivate() const {
    return is_private;
}

bool HTTPResponse::isCacheable() {
    if(this->isNoStore() == true || this->isPrivate() == true) return false;
    else return true;
}

void HTTPResponse::setRecvTime() {
    Time t;
    this->recv_time = t.getCurrentTm();
}

time_t HTTPResponse::getRecvTime() const {
    return recv_time;
}

const string &HTTPResponse::getEtag() const {
    return Etag;
}

const string &HTTPResponse::getLastModified() const {
    return last_modified;
}

time_t HTTPResponse::getExpireTime() const {
    return expire_time;
}

int HTTPResponse::getMaxAge() const {
    return maxAge;
}


void HTTPResponse::parseSMaxAge() {
    string response_temp = this->response;
    std::transform(response_temp.begin(),response_temp.end(),response_temp.begin(),::tolower);
    size_t find_cache=response_temp.find("cache-control: ");
    if(find_cache!=string::npos) {
        size_t end = response_temp.find_first_of("\r\n", find_cache + 1);
        if(end!=string::npos){
            string temp = response_temp.substr(find_cache + CacheControlKEYWORDLEN, end - find_cache - CacheControlKEYWORDLEN);
            size_t len=temp.size();
            size_t find_max = response_temp.find("s-maxage=",find_cache+1);
            if (find_max != string::npos) {
                size_t find_colon = response_temp.find(",", find_max + 1);
                string s_max_age;
                if (find_colon == string::npos) {
                    s_max_age=response.substr(find_max + 9, len - 9);
                }
                else s_max_age = response.substr(find_max + 9, find_colon - find_max - 9);// TODO the magic value; invalid use of find_colon
                sMaxAge = atoi(s_max_age.c_str());
            }
        }
    }
}

int HTTPResponse::getSMaxAge() const {
    return sMaxAge;
}









