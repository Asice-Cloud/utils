#include <coroutine>
#include <print>
#include <string>
#include <string_view>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <memory>
#include <array>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include "../core.h"

using namespace std::chrono_literals;

// WebSocket magic GUID for handshake
constexpr const char* WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// WebSocket opcodes
enum class WSOpcode : uint8_t {
    CONTINUATION = 0x0,
    TEXT = 0x1,
    BINARY = 0x2,
    CLOSE = 0x8,
    PING = 0x9,
    PONG = 0xA
};

// WebSocket frame structure
struct WSFrame {
    bool fin;
    WSOpcode opcode;
    bool masked;
    uint64_t payload_length;
    std::array<uint8_t, 4> mask_key;
    std::vector<uint8_t> payload;
    
    bool is_control_frame() const {
        return static_cast<uint8_t>(opcode) >= 0x8;
    }
};

// Base64 encode
std::string base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &buffer_ptr);
    
    std::string result(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(bio);
    
    return result;
}

// Generate WebSocket accept key
std::string generate_accept_key(const std::string& client_key) {
    std::string combined = client_key + WS_GUID;
    
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), 
         combined.length(), hash);
    
    return base64_encode(hash, SHA_DIGEST_LENGTH);
}

// Parse HTTP headers
std::map<std::string, std::string> parse_headers(const std::string& raw) {
    std::map<std::string, std::string> headers;
    std::istringstream iss(raw);
    std::string line;
    
    // Skip request line
    std::getline(iss, line);
    
    while (std::getline(iss, line) && line != "\r") {
        auto colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 2);
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            headers[key] = value;
        }
    }
    
    return headers;
}

// Async read from socket
task<int> async_read(int socket_fd, char* buffer, size_t size) {
    co_await schedule_on(get_global_executor());
    co_await async_delay(1ms);
    
    ssize_t bytes_read = recv(socket_fd, buffer, size, 0);
    co_return bytes_read > 0 ? static_cast<int>(bytes_read) : 0;
}

// Async write to socket
task<int> async_write(int socket_fd, const void* data, size_t size) {
    co_await schedule_on(get_global_executor());
    co_await async_delay(1ms);
    
    ssize_t bytes_sent = send(socket_fd, data, size, 0);
    co_return bytes_sent > 0 ? static_cast<int>(bytes_sent) : 0;
}

// Helper: case-insensitive string comparison
bool iequals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    return std::equal(a.begin(), a.end(), b.begin(),
                     [](char a, char b) {
                         return std::tolower(a) == std::tolower(b);
                     });
}

// Helper: find header case-insensitively
std::string get_header(const std::map<std::string, std::string>& headers, const std::string& key) {
    for (const auto& [k, v] : headers) {
        if (iequals(k, key)) {
            return v;
        }
    }
    return "";
}

// WebSocket handshake
task<bool> ws_handshake(int client_fd) {
    co_await schedule_on(get_global_executor());
    
    char buffer[4096];
    int bytes_read = co_await async_read(client_fd, buffer, sizeof(buffer) - 1);
    
    if (bytes_read <= 0) {
        co_return false;
    }
    
    buffer[bytes_read] = '\0';
    std::string request(buffer);
    
    // Parse headers
    auto headers = parse_headers(request);
    
    // Check if this is a WebSocket upgrade request
    std::string upgrade = get_header(headers, "Upgrade");
    std::string connection = get_header(headers, "Connection");
    std::string ws_key = get_header(headers, "Sec-WebSocket-Key");
    
    // If not a WebSocket request, send chat room HTML page
    if (!iequals(upgrade, "websocket") || ws_key.empty()) {
        std::println("[HTTP] Regular HTTP request, sending chat room page");
        
        // Read chatroom.html content
        std::ifstream html_file("../examples/chatroom.html");
        std::string html_content;
        
        if (html_file.is_open()) {
            std::ostringstream ss;
            ss << html_file.rdbuf();
            html_content = ss.str();
            html_file.close();
        } else {
            // Fallback minimal HTML if file not found
            html_content = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Chat Room</title></head>"
                          "<body style='font-family: sans-serif; text-align: center; padding: 50px;'>"
                          "<h1>Chat Room</h1><p>Cannot load chat interface. Please check chatroom.html file.</p>"
                          "</body></html>";
        }
        
        std::string html_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: " + std::to_string(html_content.size()) + "\r\n"
            "Connection: close\r\n\r\n" +
            html_content;
        
        co_await async_write(client_fd, html_response.c_str(), html_response.size());
        co_return false;
    }
    
    // Validate WebSocket upgrade request
    if (connection.find("Upgrade") == std::string::npos && 
        connection.find("upgrade") == std::string::npos) {
        std::println("[WS] Invalid Connection header: '{}'", connection);
        co_return false;
    }
    
    // Generate accept key
    std::string accept_key = generate_accept_key(ws_key);
    
    // Send handshake response
    std::ostringstream response;
    response << "HTTP/1.1 101 Switching Protocols\r\n"
             << "Upgrade: websocket\r\n"
             << "Connection: Upgrade\r\n"
             << "Sec-WebSocket-Accept: " << accept_key << "\r\n"
             << "\r\n";
    
    std::string response_str = response.str();
    int bytes_sent = co_await async_write(client_fd, response_str.c_str(), response_str.size());
    
    if (bytes_sent <= 0) {
        co_return false;
    }
    
    std::println("[WS] Handshake successful - FD: {}", client_fd);
    co_return true;
}

// Parse WebSocket frame
task<std::optional<WSFrame>> ws_read_frame(int client_fd) {
    co_await schedule_on(get_global_executor());
    
    WSFrame frame;
    char header[2];
    
    // Read first 2 bytes
    int bytes = co_await async_read(client_fd, header, 2);
    if (bytes != 2) {
        co_return std::nullopt;
    }
    
    // Parse header
    frame.fin = (header[0] & 0x80) != 0;
    frame.opcode = static_cast<WSOpcode>(header[0] & 0x0F);
    frame.masked = (header[1] & 0x80) != 0;
    frame.payload_length = header[1] & 0x7F;
    
    // Extended payload length
    if (frame.payload_length == 126) {
        char ext_len[2];
        bytes = co_await async_read(client_fd, ext_len, 2);
        if (bytes != 2) co_return std::nullopt;
        frame.payload_length = (static_cast<uint16_t>(ext_len[0]) << 8) | 
                              static_cast<uint16_t>(ext_len[1]);
    } else if (frame.payload_length == 127) {
        char ext_len[8];
        bytes = co_await async_read(client_fd, ext_len, 8);
        if (bytes != 8) co_return std::nullopt;
        frame.payload_length = 0;
        for (int i = 0; i < 8; i++) {
            frame.payload_length = (frame.payload_length << 8) | 
                                  static_cast<uint8_t>(ext_len[i]);
        }
    }
    
    // Read mask key if present
    if (frame.masked) {
        bytes = co_await async_read(client_fd, 
                                    reinterpret_cast<char*>(frame.mask_key.data()), 4);
        if (bytes != 4) co_return std::nullopt;
    }
    
    // Read payload
    if (frame.payload_length > 0) {
        frame.payload.resize(frame.payload_length);
        size_t total_read = 0;
        
        while (total_read < frame.payload_length) {
            bytes = co_await async_read(client_fd, 
                                        reinterpret_cast<char*>(frame.payload.data() + total_read),
                                        frame.payload_length - total_read);
            if (bytes <= 0) co_return std::nullopt;
            total_read += bytes;
        }
        
        // Unmask payload if masked
        if (frame.masked) {
            for (size_t i = 0; i < frame.payload.size(); i++) {
                frame.payload[i] ^= frame.mask_key[i % 4];
            }
        }
    }
    
    co_return frame;
}

// Send WebSocket frame
task<bool> ws_send_frame(int client_fd, WSOpcode opcode, const std::string& payload) {
    co_await schedule_on(get_global_executor());
    
    std::vector<uint8_t> frame;
    
    // First byte: FIN + opcode
    frame.push_back(0x80 | static_cast<uint8_t>(opcode));
    
    // Payload length
    size_t len = payload.size();
    if (len < 126) {
        frame.push_back(static_cast<uint8_t>(len));
    } else if (len < 65536) {
        frame.push_back(126);
        frame.push_back((len >> 8) & 0xFF);
        frame.push_back(len & 0xFF);
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; i--) {
            frame.push_back((len >> (i * 8)) & 0xFF);
        }
    }
    
    // Payload
    frame.insert(frame.end(), payload.begin(), payload.end());
    
    int bytes_sent = co_await async_write(client_fd, frame.data(), frame.size());
    co_return bytes_sent > 0;
}

// Chat room user structure
struct ChatUser {
    int fd;
    std::string nickname;
    std::chrono::system_clock::time_point join_time;
};

// Chat room state
std::vector<ChatUser> chat_users;
std::mutex users_mutex;
int next_user_id = 1;

// Helper: Find user by fd
std::optional<ChatUser> find_user(int fd) {
    std::lock_guard lock(users_mutex);
    for (const auto& user : chat_users) {
        if (user.fd == fd) {
            return user;
        }
    }
    return std::nullopt;
}

// Helper: Get all usernames
std::vector<std::string> get_all_usernames() {
    std::lock_guard lock(users_mutex);
    std::vector<std::string> names;
    for (const auto& user : chat_users) {
        names.push_back(user.nickname);
    }
    return names;
}

// Broadcast message to all clients
task<void> broadcast_message(const std::string& message, int exclude_fd = -1) {
    co_await schedule_on(get_global_executor());
    
    std::vector<int> client_fds;
    {
        std::lock_guard lock(users_mutex);
        for (const auto& user : chat_users) {
            if (user.fd != exclude_fd) {
                client_fds.push_back(user.fd);
            }
        }
    }
    
    for (int fd : client_fds) {
        co_await ws_send_frame(fd, WSOpcode::TEXT, message);
    }
}

// Send user list update to all clients
task<void> broadcast_user_list() {
    co_await schedule_on(get_global_executor());
    
    auto usernames = get_all_usernames();
    std::ostringstream oss;
    oss << "{\"type\":\"userlist\",\"users\":[";
    for (size_t i = 0; i < usernames.size(); i++) {
        if (i > 0) oss << ",";
        oss << "\"" << usernames[i] << "\"";
    }
    oss << "]}";
    
    co_await broadcast_message(oss.str());
}

// Handle WebSocket client (Chat Room)
task<void> handle_websocket_client(int client_fd) {
    co_await schedule_on(get_global_executor());
    
    std::string user_nickname;
    bool user_registered = false;
    
    try {
        // Perform WebSocket handshake
        bool handshake_ok = co_await ws_handshake(client_fd);
        if (!handshake_ok) {
            std::println("[CHAT] Handshake failed - FD: {}", client_fd);
            close(client_fd);
            co_return;
        }
        
        // Ask for nickname
        std::string welcome_msg = "{\"type\":\"system\",\"message\":\"Welcome to Chat Room! Please enter your nickname:\"}";
        co_await ws_send_frame(client_fd, WSOpcode::TEXT, welcome_msg);
        
        // Message loop
        while (true) {
            auto frame_opt = co_await ws_read_frame(client_fd);
            
            if (!frame_opt) {
                std::println("[CHAT] Connection closed - FD: {}", client_fd);
                break;
            }
            
            WSFrame frame = *frame_opt;
            
            if (frame.opcode == WSOpcode::CLOSE) {
                std::println("[CHAT] {} requested close", user_nickname.empty() ? std::to_string(client_fd) : user_nickname);
                co_await ws_send_frame(client_fd, WSOpcode::CLOSE, "");
                break;
            }
            
            if (frame.opcode == WSOpcode::PING) {
                co_await ws_send_frame(client_fd, WSOpcode::PONG, 
                                      std::string(frame.payload.begin(), frame.payload.end()));
                continue;
            }
            
            if (frame.opcode == WSOpcode::TEXT) {
                std::string message(frame.payload.begin(), frame.payload.end());
                
                // If user not registered yet, this is their nickname
                if (!user_registered) {
                    user_nickname = message;
                    if (user_nickname.empty() || user_nickname.size() > 20) {
                        user_nickname = "User" + std::to_string(next_user_id++);
                    }
                    
                    // Add user to chat room
                    {
                        std::lock_guard lock(users_mutex);
                        chat_users.push_back({
                            client_fd,
                            user_nickname,
                            std::chrono::system_clock::now()
                        });
                    }
                    
                    user_registered = true;
                    std::println("[CHAT] User registered: {} (FD: {})", user_nickname, client_fd);
                    
                    // Send confirmation
                    std::ostringstream confirm;
                    confirm << "{\"type\":\"system\",\"message\":\"Welcome, " << user_nickname << "!\"}";
                    co_await ws_send_frame(client_fd, WSOpcode::TEXT, confirm.str());
                    
                    // Broadcast join message
                    std::ostringstream join_msg;
                    join_msg << "{\"type\":\"join\",\"user\":\"" << user_nickname << "\"}";
                    co_await broadcast_message(join_msg.str(), client_fd);
                    
                    // Send user list to everyone
                    co_await broadcast_user_list();
                    
                    continue;
                }
                
                // Regular chat message
                std::println("[CHAT] {}: {}", user_nickname, message);
                
                // Broadcast to all users
                std::ostringstream chat_msg;
                chat_msg << "{\"type\":\"message\",\"user\":\"" << user_nickname 
                        << "\",\"message\":\"" << message << "\"}";
                co_await broadcast_message(chat_msg.str());
            }
        }
        
    } catch (const std::exception& e) {
        std::println("[CHAT] Exception ({}): {}", user_nickname, e.what());
    }
    
    // Remove from chat room
    {
        std::lock_guard lock(users_mutex);
        chat_users.erase(
            std::remove_if(chat_users.begin(), chat_users.end(),
                          [client_fd](const ChatUser& u) { return u.fd == client_fd; }),
            chat_users.end()
        );
    }
    
    // Notify others if user was registered
    if (user_registered) {
        std::ostringstream leave_msg;
        leave_msg << "{\"type\":\"leave\",\"user\":\"" << user_nickname << "\"}";
        co_await broadcast_message(leave_msg.str());
        
        // Update user list
        co_await broadcast_user_list();
        
        std::println("[CHAT] {} left the chat", user_nickname);
    }
    
    close(client_fd);
    co_return;
}

// WebSocket server
void run_websocket_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::println("Failed to create socket");
        return;
    }
    
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
    std::println("║  💬 WebSocket Chat Room (C++23)         ║");
    std::println("╚══════════════════════════════════════════╝");
    std::println("🚀 Server listening on ws://localhost:{}", port);
    std::println("📝 HTTP UI: http://localhost:{}", port);
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
        
        // Handle client asynchronously
        handle_websocket_client(client_fd).detach();
    }
    
    close(server_fd);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    try {
        run_websocket_server(port);
    } catch (const std::exception& e) {
        std::println("Server error: {}", e.what());
    }
    
    get_global_executor().shutdown();
    return 0;
}
