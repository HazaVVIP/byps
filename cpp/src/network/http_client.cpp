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
#include <fcntl.h>
#include <errno.h>
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
            
            Logger::getInstance().debug("Attempting connection to " + parsed.host + ":" + std::to_string(parsed.port));
            
            // Create socket
            int sockfd = createSocket(parsed.host, parsed.port);
            if (sockfd < 0) {
                response.status_code = 0;
                response.status_message = "Connection failed";
                Logger::getInstance().error("Failed to connect to " + parsed.host);
                return response;
            }
            
            // Handle HTTPS
            SSL* ssl = nullptr;
            SSL_CTX* ctx = nullptr;
            
            if (parsed.scheme == "https") {
                Logger::getInstance().debug("Establishing SSL connection");
                ctx = SSL_CTX_new(TLS_client_method());
                if (!ctx) {
                    ::close(sockfd);
                    response.status_code = 0;
                    response.status_message = "SSL context creation failed";
                    Logger::getInstance().error("SSL context creation failed");
                    return response;
                }
                
                if (!verify_ssl) {
                    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
                }
                
                ssl = SSL_new(ctx);
                SSL_set_fd(ssl, sockfd);
                
                if (SSL_connect(ssl) != 1) {
                    char err_buf[256];
                    ERR_error_string_n(ERR_get_error(), err_buf, sizeof(err_buf));
                    SSL_free(ssl);
                    SSL_CTX_free(ctx);
                    ::close(sockfd);
                    response.status_code = 0;
                    response.status_message = std::string("SSL connection failed: ") + err_buf;
                    Logger::getInstance().error("SSL connection failed: " + std::string(err_buf));
                    return response;
                }
                Logger::getInstance().debug("SSL connection established");
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
        struct addrinfo hints, *result, *rp;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        
        std::string port_str = std::to_string(port);
        if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &result) != 0) {
            Logger::getInstance().error("Failed to resolve hostname: " + host);
            return -1;
        }
        
        int sockfd = -1;
        // Try each address until we successfully connect
        for (rp = result; rp != nullptr; rp = rp->ai_next) {
            sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sockfd < 0) {
                continue; // Try next address
            }
            
            // Set socket timeouts
            struct timeval tv;
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            
            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                Logger::getInstance().debug("Failed to set socket receive timeout (non-fatal)");
            }
            if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
                Logger::getInstance().debug("Failed to set socket send timeout (non-fatal)");
            }
            
            // Set socket to non-blocking for connect timeout
            int flags = fcntl(sockfd, F_GETFL, 0);
            fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
            
            // Try to connect
            int connect_result = connect(sockfd, rp->ai_addr, rp->ai_addrlen);
            
            if (connect_result < 0) {
                if (errno == EINPROGRESS) {
                    // Wait for connection with timeout
                    fd_set write_fds;
                    FD_ZERO(&write_fds);
                    FD_SET(sockfd, &write_fds);
                    
                    struct timeval connect_tv;
                    connect_tv.tv_sec = timeout_ms / 1000;
                    connect_tv.tv_usec = (timeout_ms % 1000) * 1000;
                    
                    int select_result = select(sockfd + 1, nullptr, &write_fds, nullptr, &connect_tv);
                    
                    if (select_result <= 0) {
                        ::close(sockfd);
                        continue; // Try next address
                    }
                    
                    // Check if connection was successful
                    int error = 0;
                    socklen_t len = sizeof(error);
                    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
                        ::close(sockfd);
                        continue; // Try next address
                    }
                    // Connection successful
                } else {
                    ::close(sockfd);
                    continue; // Try next address
                }
            }
            
            // Set socket back to blocking mode
            fcntl(sockfd, F_SETFL, flags);
            
            // Connection successful
            Logger::getInstance().debug("Successfully connected to " + host + ":" + port_str);
            break;
        }
        
        freeaddrinfo(result);
        
        if (rp == nullptr) {
            // No address succeeded
            Logger::getInstance().error("Failed to connect to " + host + ":" + port_str);
            return -1;
        }
        
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
