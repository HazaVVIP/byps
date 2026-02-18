#ifndef BYPS_NETWORK_HTTP_CLIENT_HPP
#define BYPS_NETWORK_HTTP_CLIENT_HPP

#include <string>
#include <map>
#include <memory>
#include <vector>

namespace byps {
namespace network {

struct HttpRequest {
    std::string method;
    std::string url;
    std::map<std::string, std::string> headers;
    std::string body;
    int timeout_ms;
    bool follow_redirects;
    bool verify_ssl;
    
    HttpRequest() 
        : method("GET"), 
          timeout_ms(30000),
          follow_redirects(true),
          verify_ssl(true) {}
};

struct HttpResponse {
    int status_code;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::string body;
    double response_time_ms;
    
    HttpResponse() : status_code(0), response_time_ms(0.0) {}
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();
    
    // Execute HTTP request
    HttpResponse execute(const HttpRequest& request);
    
    // Convenience methods
    HttpResponse get(const std::string& url, 
                     const std::map<std::string, std::string>& headers = {});
    HttpResponse post(const std::string& url, 
                      const std::string& body,
                      const std::map<std::string, std::string>& headers = {});
    HttpResponse head(const std::string& url,
                      const std::map<std::string, std::string>& headers = {});
    
    // Configuration
    void setTimeout(int timeout_ms);
    void setFollowRedirects(bool follow);
    void setVerifySSL(bool verify);
    void setUserAgent(const std::string& user_agent);
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace network
} // namespace byps

#endif // BYPS_NETWORK_HTTP_CLIENT_HPP
