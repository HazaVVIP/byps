#include "network/connection_pool.hpp"
#include "common/logger.hpp"
#include <map>
#include <vector>
#include <mutex>
#include <algorithm>

namespace byps {
namespace network {

class ConnectionPool::Impl {
public:
    size_t max_connections;
    int idle_timeout_ms;
    std::mutex mutex;
    std::map<std::string, std::vector<std::shared_ptr<Connection>>> pools;
    
    Impl(size_t max_conn, int timeout) 
        : max_connections(max_conn), idle_timeout_ms(timeout) {}
    
    std::string makeKey(const std::string& host, int port) {
        return host + ":" + std::to_string(port);
    }
};

ConnectionPool::ConnectionPool(size_t max_connections, int idle_timeout_ms)
    : impl_(std::make_unique<Impl>(max_connections, idle_timeout_ms)) {}

ConnectionPool::~ConnectionPool() = default;

std::shared_ptr<Connection> ConnectionPool::acquire(const std::string& host, int port) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    std::string key = impl_->makeKey(host, port);
    
    auto& pool = impl_->pools[key];
    
    // Remove expired connections
    pool.erase(
        std::remove_if(pool.begin(), pool.end(),
            [](const std::shared_ptr<Connection>& conn) {
                return !conn || !conn->isValid() || conn->isExpired();
            }),
        pool.end()
    );
    
    // Try to reuse existing connection
    if (!pool.empty()) {
        auto conn = pool.back();
        pool.pop_back();
        Logger::getInstance().debug("Reusing connection from pool");
        return conn;
    }
    
    Logger::getInstance().debug("No available connection in pool, creating new");
    return nullptr;
}

void ConnectionPool::release(const std::string& host, int port, 
                             std::shared_ptr<Connection> conn) {
    if (!conn || !conn->isValid()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(impl_->mutex);
    std::string key = impl_->makeKey(host, port);
    
    auto& pool = impl_->pools[key];
    if (pool.size() < impl_->max_connections) {
        pool.push_back(conn);
        Logger::getInstance().debug("Connection returned to pool");
    } else {
        Logger::getInstance().debug("Pool full, discarding connection");
    }
}

void ConnectionPool::clear() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->pools.clear();
    Logger::getInstance().info("Connection pool cleared");
}

size_t ConnectionPool::getActiveConnections() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    size_t count = 0;
    for (const auto& [key, pool] : impl_->pools) {
        count += pool.size();
    }
    return count;
}

size_t ConnectionPool::getIdleConnections() const {
    return getActiveConnections();
}

} // namespace network
} // namespace byps
