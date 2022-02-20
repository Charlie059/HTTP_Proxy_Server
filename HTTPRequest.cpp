//
// Created by HP on 2022/2/14.
//
#include <cstring>
#include <exception>
#include <iostream>
#include <fstream>
#include "HTTPRequest.h"
using namespace std;

//#define DEBUG_MODE
#ifdef DEBUG_MODE
#include "gtest/gtest.h"
#endif

// TODO WARNING Case-insentitive
//This function is uesd to get the host and port number
void HTTPRequest::requireHostPort(){
    string raw_temp = this->raw;
    //find the first index where "Host" appears
    std::transform(raw_temp.begin(),raw_temp.end(),raw_temp.begin(),::tolower);
    size_t find_host=raw_temp.find("host: "); //TODO size_t -1
    if(find_host!=string::npos){
        string temp=raw_temp.substr(find_host+6);
        //find the first index where \r or \n appears
        size_t host_end=temp.find_first_of("\r\n"); //TODO size_t -1
        if(host_end!=string::npos){
            string host_temp=temp.substr(0,host_end);
            //find the first index where : appears
            size_t colon=host_temp.find_first_of(":"); //TODO size_t -1
                //truncate the host name

                if(colon!=string::npos){
                    //truncate the port number
                    host=host_temp.substr(0,colon); //TODO size_t -1
                    port=host_temp.substr(colon+1);
                    if(port.empty()) port="80";
                }
                else{
                    host=host_temp;
                    port="80";
                }


        }
    }

}



void HTTPRequest::requireMethod(){ // TODO Change find CONNECT GRT POST directly rather than find_first_of(" "), careful size_t = -1
    size_t find_connect=raw.find("CONNECT");
    size_t find_post=raw.find("POST");
    size_t find_get=raw.find("GET");
    if(find_connect!=string::npos){
        method="CONNECT";
    }
    else if(find_post!=string::npos){
        method="POST";
    }
    else if(find_get!=string::npos){
        method="GET";
    }
    else{
        method="";
    }
}

string HTTPRequest::getRaw() const {
    return raw;
}

void HTTPRequest::requireline(){
size_t end=raw.find_first_of("\r\n");
if(end!=string::npos){line=raw.substr(0,end);}
else{line="";}
}

std::string HTTPRequest::buildHTTPRequest(std::string headerLine, bool IfNoneMatch, bool IfModifiedSince, std::string Etag, std::string LastModified) {
    std::string ans = headerLine + "\n";
    if(IfNoneMatch){
        ans +=  "If-None-Match: " + Etag  + "\n";
    }
    if(IfModifiedSince){
        ans +=  "If-Modified-Since: " + LastModified  + "\n";
    }
    ans += "\r\n";
    return ans;
}


//
//// Test case 1: getport() method
//TEST(HTTPRequestTest, test1){
//    std::ifstream f("/Users/xg/CLionProjects/http-caching-proxy/Examples/GET_Example");
//    std::stringstream reader;
//    reader << f.rdbuf();
//    HTTPRequest req(reader.str());
//    EXPECT_STREQ("12345",req.getport().c_str());
//}

