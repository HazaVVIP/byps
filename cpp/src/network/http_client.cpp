#include "network/http_client.hpp"
#include "common/utils.hpp"
#include "common/logger.hpp"
#include "network/tls.hpp"
#include <sstream>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace byps {
namespace network {

class HttpClient::Impl {
public:
    int timeout_ms;
    bool follow_redirects;
    bool verify_ssl;
    std::string user_agent;
    
    Impl() 
        : timeout_ms(30000),
          follow_redirects(true),
          verify_ssl(true),
          user_agent("Byps/0.1.0") {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
    }
    
    ~Impl() {
        EVP_cleanup();
    }
    
    HttpResponse executeRequest(const HttpRequest& request) {
        HttpResponse response;
        auto start_time = std::chrono::high_resolution_clock::now();
        
        try {
            auto parsed = utils::parseURL(request.url);
            
            // Create socket
            int sockfd = createSocket(parsed.host, parsed.port);
            if (sockfd < 0) {
                response.status_code = 0;
                response.status_message = "Connection failed";
                return response;
            }
            
            // Handle HTTPS
            SSL* ssl = nullptr;
            SSL_CTX* ctx = nullptr;
            
            if (parsed.scheme == "https") {
                ctx = SSL_CTX_new(TLS_client_method());
                if (!ctx) {
                    ::close(sockfd);
                    response.status_code = 0;
                    response.status_message = "SSL context creation failed";
                    return response;
                }
                
                if (!verify_ssl) {
                    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
                }
                
                ssl = SSL_new(ctx);
                SSL_set_fd(ssl, sockfd);
                
                if (SSL_connect(ssl) != 1) {
                    SSL_free(ssl);
                    SSL_CTX_free(ctx);
                    ::close(sockfd);
                    response.status_code = 0;
                    response.status_message = "SSL connection failed";
                    return response;
                }
            }
            
            // Build HTTP request
            std::stringstream req_stream;
            req_stream << request.method << " " << parsed.path;
            if (!parsed.query.empty()) {
                req_stream << "?" << parsed.query;
            }
            req_stream << " HTTP/1.1\r\n";
            req_stream << "Host: " << parsed.host << "\r\n";
            req_stream << "User-Agent: " << user_agent << "\r\n";
            req_stream << "Connection: close\r\n";
            
            for (const auto& [key, value] : request.headers) {
                req_stream << key << ": " << value << "\r\n";
            }
            
            if (!request.body.empty()) {
                req_stream << "Content-Length: " << request.body.length() << "\r\n";
            }
            
            req_stream << "\r\n";
            if (!request.body.empty()) {
                req_stream << request.body;
            }
            
            std::string req_str = req_stream.str();
            
            // Send request
            if (ssl) {
                SSL_write(ssl, req_str.c_str(), req_str.length());
            } else {
                ::send(sockfd, req_str.c_str(), req_str.length(), 0);
            }
            
            // Receive response
            std::string resp_str;
            char buffer[4096];
            ssize_t bytes_read;
            
            while (true) {
                if (ssl) {
                    bytes_read = SSL_read(ssl, buffer, sizeof(buffer));
                } else {
                    bytes_read = ::recv(sockfd, buffer, sizeof(buffer), 0);
                }
                
                if (bytes_read <= 0) break;
                resp_str.append(buffer, bytes_read);
            }
            
            // Cleanup
            if (ssl) {
                SSL_shutdown(ssl);
                SSL_free(ssl);
                SSL_CTX_free(ctx);
            }
            ::close(sockfd);
            
            // Parse response
            parseResponse(resp_str, response);
            
        } catch (const std::exception& e) {
            Logger::getInstance().error(std::string("HTTP request failed: ") + e.what());
            response.status_code = 0;
            response.status_message = e.what();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        response.response_time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        return response;
    }
    
private:
    int createSocket(const std::string& host, int port) {
        struct addrinfo hints, *result;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        
        std::string port_str = std::to_string(port);
        if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result) != 0) {
            return -1;
        }
        
        int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (sockfd < 0) {
            freeaddrinfo(result);
            return -1;
        }
        
        if (connect(sockfd, result->ai_addr, result->ai_addrlen) < 0) {
            ::close(sockfd);
            freeaddrinfo(result);
            return -1;
        }
        
        freeaddrinfo(result);
        return sockfd;
    }
    
    void parseResponse(const std::string& resp_str, HttpResponse& response) {
        size_t header_end = resp_str.find("\r\n\r\n");
        if (header_end == std::string::npos) {
            response.status_code = 0;
            response.status_message = "Invalid response";
            return;
        }
        
        std::string headers = resp_str.substr(0, header_end);
        response.body = resp_str.substr(header_end + 4);
        
        std::istringstream iss(headers);
        std::string line;
        
        // Parse status line
        if (std::getline(iss, line)) {
            // Remove \r if present
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            size_t first_space = line.find(' ');
            size_t second_space = line.find(' ', first_space + 1);
            
            if (first_space != std::string::npos && second_space != std::string::npos) {
                response.status_code = std::stoi(line.substr(first_space + 1, second_space - first_space - 1));
                response.status_message = line.substr(second_space + 1);
            }
        }
        
        // Parse headers
        while (std::getline(iss, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string value = line.substr(colon + 1);
                
                // Trim leading whitespace from value
                size_t value_start = value.find_first_not_of(" \t");
                if (value_start != std::string::npos) {
                    value = value.substr(value_start);
                }
                
                response.headers[key] = value;
            }
        }
    }
};

HttpClient::HttpClient() : impl_(std::make_unique<Impl>()) {}
HttpClient::~HttpClient() = default;

HttpResponse HttpClient::execute(const HttpRequest& request) {
    return impl_->executeRequest(request);
}

HttpResponse HttpClient::get(const std::string& url, 
                              const std::map<std::string, std::string>& headers) {
    HttpRequest request;
    request.method = "GET";
    request.url = url;
    request.headers = headers;
    request.timeout_ms = impl_->timeout_ms;
    request.follow_redirects = impl_->follow_redirects;
    request.verify_ssl = impl_->verify_ssl;
    return execute(request);
}

HttpResponse HttpClient::post(const std::string& url, 
                               const std::string& body,
                               const std::map<std::string, std::string>& headers) {
    HttpRequest request;
    request.method = "POST";
    request.url = url;
    request.body = body;
    request.headers = headers;
    request.timeout_ms = impl_->timeout_ms;
    request.follow_redirects = impl_->follow_redirects;
    request.verify_ssl = impl_->verify_ssl;
    return execute(request);
}

HttpResponse HttpClient::head(const std::string& url,
                               const std::map<std::string, std::string>& headers) {
    HttpRequest request;
    request.method = "HEAD";
    request.url = url;
    request.headers = headers;
    request.timeout_ms = impl_->timeout_ms;
    request.follow_redirects = impl_->follow_redirects;
    request.verify_ssl = impl_->verify_ssl;
    return execute(request);
}

void HttpClient::setTimeout(int timeout_ms) {
    impl_->timeout_ms = timeout_ms;
}

void HttpClient::setFollowRedirects(bool follow) {
    impl_->follow_redirects = follow;
}

void HttpClient::setVerifySSL(bool verify) {
    impl_->verify_ssl = verify;
}

void HttpClient::setUserAgent(const std::string& user_agent) {
    impl_->user_agent = user_agent;
}

} // namespace network
} // namespace byps
