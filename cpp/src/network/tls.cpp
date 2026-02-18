#include "network/tls.hpp"
#include "common/logger.hpp"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <cstring>
#include <unistd.h>

namespace byps {
namespace network {

// TLSContext implementation
class TLSContext::Impl {
public:
    SSL_CTX* ctx;
    bool verify_peer;
    bool verify_hostname;
    
    Impl() : ctx(nullptr), verify_peer(true), verify_hostname(true) {}
    
    ~Impl() {
        if (ctx) {
            SSL_CTX_free(ctx);
        }
    }
};

TLSContext::TLSContext() : impl_(std::make_unique<Impl>()) {}
TLSContext::~TLSContext() = default;

bool TLSContext::init() {
    impl_->ctx = SSL_CTX_new(TLS_client_method());
    if (!impl_->ctx) {
        Logger::getInstance().error("Failed to create SSL context");
        return false;
    }
    
    SSL_CTX_set_min_proto_version(impl_->ctx, TLS1_2_VERSION);
    SSL_CTX_set_options(impl_->ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
    
    return true;
}

void TLSContext::setVerifyPeer(bool verify) {
    impl_->verify_peer = verify;
    if (impl_->ctx) {
        if (verify) {
            SSL_CTX_set_verify(impl_->ctx, SSL_VERIFY_PEER, nullptr);
        } else {
            SSL_CTX_set_verify(impl_->ctx, SSL_VERIFY_NONE, nullptr);
        }
    }
}

void TLSContext::setVerifyHostname(bool verify) {
    impl_->verify_hostname = verify;
}

bool TLSContext::loadCertificateFile(const std::string& path) {
    if (!impl_->ctx) return false;
    return SSL_CTX_load_verify_locations(impl_->ctx, path.c_str(), nullptr) == 1;
}

bool TLSContext::loadCertificateDirectory(const std::string& path) {
    if (!impl_->ctx) return false;
    return SSL_CTX_load_verify_locations(impl_->ctx, nullptr, path.c_str()) == 1;
}

void* TLSContext::getSSLContext() {
    return impl_->ctx;
}

// TLSSocket implementation
class TLSSocket::Impl {
public:
    TLSContext& context;
    SSL* ssl;
    int sockfd;
    bool connected;
    
    Impl(TLSContext& ctx) 
        : context(ctx), ssl(nullptr), sockfd(-1), connected(false) {}
    
    ~Impl() {
        if (ssl) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
        }
        if (sockfd >= 0) {
            ::close(sockfd);
        }
    }
};

TLSSocket::TLSSocket(TLSContext& context) 
    : impl_(std::make_unique<Impl>(context)) {}

TLSSocket::~TLSSocket() = default;

bool TLSSocket::connect(const std::string& host, int port, int timeout_ms) {
    // Socket creation and connection would go here
    // This is a simplified stub implementation
    Logger::getInstance().info("TLS connection to " + host + ":" + std::to_string(port));
    return false;
}

ssize_t TLSSocket::send(const void* data, size_t len) {
    if (!impl_->ssl) return -1;
    return SSL_write(impl_->ssl, data, len);
}

ssize_t TLSSocket::receive(void* buffer, size_t len) {
    if (!impl_->ssl) return -1;
    return SSL_read(impl_->ssl, buffer, len);
}

void TLSSocket::close() {
    if (impl_->ssl) {
        SSL_shutdown(impl_->ssl);
        SSL_free(impl_->ssl);
        impl_->ssl = nullptr;
    }
    if (impl_->sockfd >= 0) {
        ::close(impl_->sockfd);
        impl_->sockfd = -1;
    }
    impl_->connected = false;
}

bool TLSSocket::isConnected() const {
    return impl_->connected;
}

std::string TLSSocket::getPeerCertificate() const {
    if (!impl_->ssl) return "";
    
    X509* cert = SSL_get_peer_certificate(impl_->ssl);
    if (!cert) return "";
    
    char* subject = X509_NAME_oneline(X509_get_subject_name(cert), nullptr, 0);
    std::string result = subject ? subject : "";
    
    OPENSSL_free(subject);
    X509_free(cert);
    
    return result;
}

} // namespace network
} // namespace byps
