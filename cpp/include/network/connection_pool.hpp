#ifndef BYPS_NETWORK_CONNECTION_POOL_HPP
#define BYPS_NETWORK_CONNECTION_POOL_HPP

#include <string>
#include <memory>
#include <chrono>

namespace byps {
namespace network {

class Connection {
public:
    virtual ~Connection() = default;
    virtual bool isValid() const = 0;
    virtual bool isExpired() const = 0;
    virtual void reset() = 0;
};

class ConnectionPool {
public:
    ConnectionPool(size_t max_connections = 10, 
                   int idle_timeout_ms = 60000);
    ~ConnectionPool();
    
    // Get a connection for a specific host
    std::shared_ptr<Connection> acquire(const std::string& host, int port);
    
    // Return a connection to the pool
    void release(const std::string& host, int port, std::shared_ptr<Connection> conn);
    
    // Clear all connections
    void clear();
    
    // Get pool statistics
    size_t getActiveConnections() const;
    size_t getIdleConnections() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace network
} // namespace byps

#endif // BYPS_NETWORK_CONNECTION_POOL_HPP
