// Local-only receiver for public vendor documents fetched by the shared browser.
// No arbitrary paths, cookies, credentials, shell commands or outbound requests.
// Existing files are never overwritten. Run with the engineering/sources directory.
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <charconv>
#include <csignal>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <string_view>

namespace {
constexpr std::size_t limit=16*1024*1024;
const std::map<std::string,std::string> files{
    {"/supply-pdf","LE-350-60.pdf"},{"/supply-step","LE-350-60.step"},
    {"/motor-pdf","34E1K-85.pdf"},{"/motor-step","34E1K-85.step"},
    {"/drive-pdf","CL86T-V41.pdf"},{"/drive-step","CL86T-V41.step"},
    {"/feed-motor-pdf","24HS40-5004D-E1000.pdf"},{"/feed-motor-step","24HS40-5004D-E1000.step"},
    {"/feed-drive-pdf","CL57T-V41.pdf"},{"/feed-drive-step","CL57T-V41.step"},
    {"/feed-supply-pdf","LE-350-36.pdf"},{"/feed-supply-step","LE-350-36.step"},
    {"/motor-torque-pdf","34E1K-85-torque.pdf"},
    {"/feed-motor-torque-pdf","24HS40-5004D-E1000-torque.pdf"}
};
void reply(int client,int code,std::string_view text,std::string_view origin) {
    std::string response="HTTP/1.1 "+std::to_string(code)+" Result\r\nConnection: close\r\nContent-Type: text/plain\r\n";
    if(origin=="https://www.omc-stepperonline.com")
        response+="Access-Control-Allow-Origin: "+std::string(origin)+"\r\nAccess-Control-Allow-Methods: POST, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type\r\nAccess-Control-Allow-Private-Network: true\r\n";
    response+="Content-Length: "+std::to_string(text.size())+"\r\n\r\n"+std::string(text);
    std::size_t sent=0;
    while(sent<response.size()) {const auto n=send(client,response.data()+sent,response.size()-sent,0); if(n<=0)break; sent+=static_cast<std::size_t>(n);}
}
void receive(int client,const std::filesystem::path& root) {
    std::array<char,16384> chunk{}; std::string data;
    std::size_t end=std::string::npos;
    while((end=data.find("\r\n\r\n"))==std::string::npos) {
        const auto n=recv(client,chunk.data(),chunk.size(),0); if(n<=0)return;
        data.append(chunk.data(),static_cast<std::size_t>(n)); if(data.size()>65536)return;
    }
    const auto lineEnd=data.find("\r\n"),space=data.find(' '),last=data.find(' ',space+1);
    if(space==std::string::npos||last==std::string::npos||last>lineEnd)return;
    const auto method=data.substr(0,space),path=data.substr(space+1,last-space-1);
    std::string origin; std::size_t length=0; bool hasLength=false;
    for(std::size_t begin=lineEnd+2;begin<end;) {
        const auto stop=data.find("\r\n",begin); const auto colon=data.find(':',begin);
        if(colon<stop) {
            auto key=data.substr(begin,colon-begin); for(auto& c:key) if(c>='A'&&c<='Z')c=static_cast<char>(c-'A'+'a');
            const auto valueStart=data.find_first_not_of(' ',colon+1);
            const auto value=data.substr(valueStart,stop-valueStart);
            if(key=="origin")origin=value;
            if(key=="content-length") {
                const auto result=std::from_chars(value.data(),value.data()+value.size(),length);
                hasLength=result.ec==std::errc{}&&result.ptr==value.data()+value.size();
            }
        }
        begin=stop+2;
    }
    if(origin!="https://www.omc-stepperonline.com") {reply(client,403,"Origin rejected",origin);return;}
    if(method=="OPTIONS") {reply(client,204,"",origin);return;}
    const auto target=files.find(path);
    if(method!="POST"||target==files.end()||!hasLength||length<16||length>limit) {reply(client,400,"Invalid request",origin);return;}
    const auto bodyStart=end+4;
    while(data.size()-bodyStart<length) {
        const auto n=recv(client,chunk.data(),std::min(chunk.size(),length-(data.size()-bodyStart)),0);
        if(n<=0)return; data.append(chunk.data(),static_cast<std::size_t>(n));
    }
    const std::string_view body(data.data()+bodyStart,length);
    const bool pdf=target->second.ends_with(".pdf");
    if(pdf?!body.starts_with("%PDF-"):body.find("ISO-10303-21;")==std::string_view::npos) {reply(client,422,"Document signature rejected",origin);return;}
    const auto destination=root/target->second;
    const int output=open(destination.c_str(),O_WRONLY|O_CREAT|O_EXCL,0644);
    if(output<0) {reply(client,409,"Destination already exists or unavailable",origin);return;}
    std::size_t written=0;
    while(written<length) {const auto n=write(output,body.data()+written,length-written); if(n<=0)break; written+=static_cast<std::size_t>(n);}
    close(output);
    if(written!=length) {std::filesystem::remove(destination);reply(client,500,"Incomplete write removed",origin);return;}
    std::cout<<target->second<<" "<<length<<" bytes saved\n"<<std::flush;
    reply(client,201,"Saved "+target->second+" ("+std::to_string(length)+" bytes)",origin);
}
}
int main(int argc,char** argv) {
    if(argc!=2||!std::filesystem::is_directory(argv[1])) {std::cerr<<"Existing output directory required\n";return 1;}
    std::signal(SIGPIPE,SIG_IGN);
    const int server=socket(AF_INET,SOCK_STREAM,0); if(server<0)return 1;
    int yes=1; setsockopt(server,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
    sockaddr_in address{}; address.sin_family=AF_INET; address.sin_port=htons(8766); address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    if(bind(server,reinterpret_cast<sockaddr*>(&address),sizeof(address))<0||listen(server,4)<0) {perror("Local receiver");close(server);return 1;}
    std::cout<<"Public document receiver on127.0.0.1:8766; fixed destinations; no overwrite\n"<<std::flush;
    for(;;) {
        const int client=accept(server,nullptr,nullptr); if(client<0)break;
        timeval timeout{30,0}; setsockopt(client,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
        try {receive(client,argv[1]);} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';}
        close(client);
    }
    close(server); return 0;
}