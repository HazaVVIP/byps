#ifndef BYPS_NETWORK_TLS_HPP
#define BYPS_NETWORK_TLS_HPP

#include <string>
#include <memory>

namespace byps {
namespace network {

class TLSContext {
public:
    TLSContext();
    ~TLSContext();
    
    // Initialize TLS context
    bool init();
    
    // Set verification mode
    void setVerifyPeer(bool verify);
    void setVerifyHostname(bool verify);
    
    // Load certificates
    bool loadCertificateFile(const std::string& path);
    bool loadCertificateDirectory(const std::string& path);
    
    // Get internal SSL context (for use with OpenSSL)
    void* getSSLContext();
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

class TLSSocket {
public:
    TLSSocket(TLSContext& context);
    ~TLSSocket();
    
    // Connect to host
    bool connect(const std::string& host, int port, int timeout_ms = 30000);
    
    // Send and receive data
    ssize_t send(const void* data, size_t len);
    ssize_t receive(void* buffer, size_t len);
    
    // Close connection
    void close();
    
    // Check connection status
    bool isConnected() const;
    
    // Get peer certificate information
    std::string getPeerCertificate() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace network
} // namespace byps

#endif // BYPS_NETWORK_TLS_HPP
