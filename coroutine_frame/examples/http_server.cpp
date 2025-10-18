#include <coroutine>
#include <print>
#include <string>
#include <string_view>
#include <sstream>
#include <map>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <memory>
#include "../core.h"  // Single include!

using namespace std::chrono_literals;

// Fire-and-forget helper: Start a task and detach it
// The task will run independently and clean itself up when done
void start_task(task<void>&& t) {
    // detach() already calls resume() internally, don't call it twice!
    t.detach();    // Release ownership - coroutine will self-destruct when done
}

// HTTP Request structure
struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
};

// HTTP Response structure
struct HttpResponse {
    int status_code = 200;
    std::string status_text = "OK";
    std::map<std::string, std::string> headers;
    std::string body;
    
    std::string to_string() const {
        std::ostringstream oss;
        oss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
        
        for (const auto& [key, value] : headers) {
            oss << key << ": " << value << "\r\n";
        }
        
        oss << "Content-Length: " << body.size() << "\r\n";
        oss << "\r\n";
        oss << body;
        
        return oss.str();
    }
};

// Parse HTTP request (simple implementation)
HttpRequest parse_request(const std::string& raw) {
    HttpRequest req;
    std::istringstream iss(raw);
    
    // Parse request line
    iss >> req.method >> req.path >> req.version;
    
    // Parse headers
    std::string line;
    std::getline(iss, line); // consume newline
    
    while (std::getline(iss, line) && line != "\r") {
        auto colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 2); // skip ": "
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            req.headers[key] = value;
        }
    }
    
    // Read body (if any)
    req.body = std::string(std::istreambuf_iterator<char>(iss), {});
    
    return req;
}

// Async read from socket
task<int> async_read(int socket_fd, char* buffer, size_t size) {
    co_await schedule_on(get_global_executor());
    
    // Simulate async I/O with delay
    co_await async_delay(10ms);
    
    ssize_t bytes_read = recv(socket_fd, buffer, size, 0);
    co_return bytes_read > 0 ? static_cast<int>(bytes_read) : 0;
}

// Async write to socket
task<int> async_write(int socket_fd, const std::string& data) {
    co_await schedule_on(get_global_executor());
    
    // Simulate async I/O with delay
    co_await async_delay(5ms);
    
    send(socket_fd, data.c_str(), data.size(), 0);
    co_return static_cast<int>(data.size());
}

// Simulate database query
task<std::string> query_database(const std::string& query) {
    co_await schedule_on(get_global_executor());
    
    std::println("[DB] Executing query: {}", query);
    co_await async_delay(50ms); // Simulate DB latency
    
    co_return "Database result for: " + query;
}

// Simulate external API call
task<std::string> call_external_api(const std::string& endpoint) {
    co_await schedule_on(get_global_executor());
    
    std::println("[API] Calling external API: {}", endpoint);
    co_await async_delay(100ms); // Simulate network latency
    
    co_return "{\"data\": \"API response from " + endpoint + "\"}";
}

// Route handlers

// Home page
task<HttpResponse> handle_home() {
    co_await schedule_on(get_global_executor());
    
    HttpResponse response;
    response.headers["Content-Type"] = "text/html";
    response.body = R"(
<!DOCTYPE html>
<html>
<head><title>Async HTTP Server</title></head>
<body>
    <h1>Welcome to Async Coroutine HTTP Server!</h1>
    <p>Try these endpoints:</p>
    <ul>
        <li><a href="/api/hello">/api/hello</a> - Simple greeting</li>
        <li><a href="/api/db">/api/db</a> - Database query demo</li>
        <li><a href="/api/external">/api/external</a> - External API call demo</li>
        <li><a href="/api/slow">/api/slow</a> - Slow endpoint (2s delay)</li>
    </ul>
</body>
</html>
    )";
    
    co_return response;
}

// API endpoint: hello
task<HttpResponse> handle_hello() {
    co_await schedule_on(get_global_executor());
    
    HttpResponse response;
    response.headers["Content-Type"] = "application/json";
    response.body = R"({"message": "Hello from async coroutine server!", "timestamp": ")" + 
                    std::to_string(std::time(nullptr)) + "\"}";
    
    co_return response;
}

// API endpoint: database query
task<HttpResponse> handle_db() {
    co_await schedule_on(get_global_executor());
    
    // Simulate async database query
    std::string db_result = co_await query_database("SELECT * FROM users");
    
    HttpResponse response;
    response.headers["Content-Type"] = "application/json";
    response.body = R"({"status": "success", "data": ")" + db_result + "\"}";
    
    co_return response;
}

// API endpoint: external API
task<HttpResponse> handle_external() {
    co_await schedule_on(get_global_executor());
    
    // Call external API asynchronously
    std::string api_result = co_await call_external_api("https://api.example.com/data");
    
    HttpResponse response;
    response.headers["Content-Type"] = "application/json";
    response.body = api_result;
    
    co_return response;
}

// API endpoint: slow operation
task<HttpResponse> handle_slow() {
    co_await schedule_on(get_global_executor());
    
    std::println("[SLOW] Starting slow operation...");
    co_await async_delay(2000ms); // 2 seconds delay
    std::println("[SLOW] Slow operation completed!");
    
    HttpResponse response;
    response.headers["Content-Type"] = "application/json";
    response.body = R"({"message": "Slow operation completed", "duration": "2s"})";
    
    co_return response;
}

// 404 handler
task<HttpResponse> handle_not_found(const std::string& path) {
    co_await schedule_on(get_global_executor());
    
    HttpResponse response;
    response.status_code = 404;
    response.status_text = "Not Found";
    response.headers["Content-Type"] = "text/html";
    response.body = "<h1>404 Not Found</h1><p>Path: " + path + "</p>";
    
    co_return response;
}

// Route dispatcher
task<HttpResponse> handle_request(const HttpRequest& req) {
    co_await schedule_on(get_global_executor());
    
    std::println("[{}] {} - Thread: {}", 
                 req.method, req.path,
                 std::hash<std::thread::id>{}(std::this_thread::get_id()) % 10000);
    
    HttpResponse response;
    
    if (req.path == "/" || req.path == "/index.html") {
        response = co_await handle_home();
    } else if (req.path == "/api/hello") {
        response = co_await handle_hello();
    } else if (req.path == "/api/db") {
        response = co_await handle_db();
    } else if (req.path == "/api/external") {
        response = co_await handle_external();
    } else if (req.path == "/api/slow") {
        response = co_await handle_slow();
    } else {
        response = co_await handle_not_found(req.path);
    }
    
    co_return response;
}

// Handle client connection
task<void> handle_client(int client_fd) {
    co_await schedule_on(get_global_executor());
    
    try {
        char buffer[4096];
        
        // Read request asynchronously
        int bytes_read = co_await async_read(client_fd, buffer, sizeof(buffer) - 1);
        
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            
            // Parse request
            HttpRequest req = parse_request(std::string(buffer));
            
            // Handle request asynchronously
            HttpResponse response = co_await handle_request(req);
            
            // Send response asynchronously
            std::string response_str = response.to_string();
            co_await async_write(client_fd, response_str);
        }
        
    } catch (const std::exception& e) {
        std::println("[ERROR] Exception handling client: {}", e.what());
    }
    
    close(client_fd);
    co_return;
}

// Simple HTTP server
void run_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::println("Failed to create socket");
        return;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::println("Failed to bind to port {}", port);
        close(server_fd);
        return;
    }
    
    if (listen(server_fd, 10) < 0) {
        std::println("Failed to listen on socket");
        close(server_fd);
        return;
    }
    
    std::println("╔══════════════════════════════════════════╗");
    std::println("║  Async Coroutine HTTP Server            ║");
    std::println("╚══════════════════════════════════════════╝");
    std::println("Server listening on http://localhost:{}", port);
    std::println("Press Ctrl+C to stop\n");
    
    // Accept connections
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            continue;
        }
        
        std::println("[ACCEPT] New connection - FD: {}", client_fd);
        
        // Handle client asynchronously (fire and forget)
        start_task(handle_client(client_fd));
    }
    
    close(server_fd);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    try {
        run_server(port);
    } catch (const std::exception& e) {
        std::println("Server error: {}", e.what());
    }
    
    get_global_executor().shutdown();
    return 0;
}
