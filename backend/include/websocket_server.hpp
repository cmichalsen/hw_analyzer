#pragma once

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <iostream>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wincrypt.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "crypt32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#endif

namespace hw_analyzer {

class WebSocketServer {
public:
    using MessageHandler = std::function<std::string(const std::string&)>;

    WebSocketServer(int port) : port_(port) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }

    ~WebSocketServer() {
        stop();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void setMessageHandler(MessageHandler handler) {
        messageHandler_ = std::move(handler);
    }

    void run() {
        running_ = true;
        
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return;
        }

        int opt = 1;
#ifdef _WIN32
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#else
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port_);

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Failed to bind socket" << std::endl;
#ifdef _WIN32
            closesocket(serverSocket);
#else
            close(serverSocket);
#endif
            return;
        }

        if (listen(serverSocket, 5) < 0) {
            std::cerr << "Failed to listen on socket" << std::endl;
#ifdef _WIN32
            closesocket(serverSocket);
#else
            close(serverSocket);
#endif
            return;
        }

        std::cout << "WebSocket server listening on port " << port_ << std::endl;

        while (running_) {
            sockaddr_in clientAddr{};
#ifdef _WIN32
            int clientLen = sizeof(clientAddr);
#else
            socklen_t clientLen = sizeof(clientAddr);
#endif
            int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket < 0) continue;

            std::cout << "Client connected" << std::endl;
            
            // Disable Nagle's algorithm for immediate sending
            int flag = 1;
#ifdef _WIN32
            setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
#else
            setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (void*)&flag, sizeof(int));
#endif
            
            // Handle WebSocket handshake and communication in a thread
            std::thread([this, clientSocket]() {
                handleClient(clientSocket);
            }).detach();
        }

#ifdef _WIN32
        closesocket(serverSocket);
#else
        close(serverSocket);
#endif
    }

    void stop() {
        running_ = false;
    }

    void broadcast(const std::string& message) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        for (int client : clients_) {
            sendWebSocketFrame(client, message);
        }
    }

private:
    // Base64 encoding
    std::string base64Encode(const unsigned char* input, size_t length) {
#ifdef _WIN32
        DWORD encodedLength = 0;
        if (!CryptBinaryToStringA(input, length, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &encodedLength)) {
            return "";
        }
        
        std::vector<char> buffer(encodedLength);
        
        DWORD actualLength = encodedLength;
        if (!CryptBinaryToStringA(input, length, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, buffer.data(), &actualLength)) {
            return "";
        }
        
        return std::string(buffer.data());
#else
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;
        
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, input, length);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);
        
        std::string result(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);
        
        return result;
#endif
    }

    // Compute WebSocket accept key
    std::string computeAcceptKey(const std::string& clientKey) {
        const std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        std::string combined = clientKey + magic;
        
#ifdef _WIN32
        // Use Windows CryptoAPI for SHA-1
        HCRYPTPROV hProv = 0;
        HCRYPTHASH hHash = 0;
        BYTE hash[20];
        DWORD hashLen = 20;
        
        if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            return "";
        }
        
        if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
            CryptReleaseContext(hProv, 0);
            return "";
        }
        
        if (!CryptHashData(hHash, (const BYTE*)combined.c_str(), combined.length(), 0)) {
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            return "";
        }
        
        if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);
            return "";
        }
        
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        
        return base64Encode(hash, hashLen);
#else
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)combined.c_str(), combined.length(), hash);
        return base64Encode(hash, SHA_DIGEST_LENGTH);
#endif
    }

    void handleClient(int clientSocket) {
        // Read HTTP upgrade request
        char buffer[4096];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif
            return;
        }
        buffer[bytesRead] = '\0';

        std::string request(buffer);

        // Extract WebSocket key
        size_t keyPos = request.find("Sec-WebSocket-Key: ");
        if (keyPos == std::string::npos) {
#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif
            return;
        }

        keyPos += 19;
        size_t keyEnd = request.find("\r\n", keyPos);
        std::string key = request.substr(keyPos, keyEnd - keyPos);

        // Compute proper WebSocket accept key
        std::string acceptKey = computeAcceptKey(key);

        // Send WebSocket accept response
        std::string response =
            "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: " + acceptKey + "\r\n\r\n";

        int sent = send(clientSocket, response.c_str(), response.length(), 0);
        
        if (sent != (int)response.length()) {
            std::cerr << "[WS] Error: Failed to send complete handshake response" << std::endl;
        }

        // Add to clients list
        {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            clients_.push_back(clientSocket);
        }

        // Read WebSocket frames
        while (running_) {
            bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0) break;

            // Simple frame parsing (assumes text frames)
            if (bytesRead >= 2) {
                uint8_t* frame = (uint8_t*)buffer;
                bool fin = (frame[0] & 0x80) != 0;
                uint8_t opcode = frame[0] & 0x0F;
                bool masked = (frame[1] & 0x80) != 0;
                uint64_t payloadLen = frame[1] & 0x7F;
                
                size_t headerSize = 2;
                if (payloadLen == 126) {
                    payloadLen = (frame[2] << 8) | frame[3];
                    headerSize = 4;
                } else if (payloadLen == 127) {
                    headerSize = 10;
                }
                
                if (masked) headerSize += 4;
                
                if (bytesRead >= (int)headerSize && opcode == 1) { // Text frame
                    std::string payload;
                    if (masked && bytesRead >= (int)(headerSize + payloadLen)) {
                        uint8_t* mask = frame + headerSize - 4;
                        uint8_t* data = frame + headerSize;
                        for (size_t i = 0; i < payloadLen; i++) {
                            payload += (char)(data[i] ^ mask[i % 4]);
                        }
                        
                        if (messageHandler_) {
                            std::string response = messageHandler_(payload);
                            sendWebSocketFrame(clientSocket, response);
                        }
                    }
                }
            }
        }

        // Remove from clients list
        {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            clients_.erase(std::remove(clients_.begin(), clients_.end(), clientSocket), clients_.end());
        }

#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
        std::cout << "Client disconnected" << std::endl;
    }

    void sendWebSocketFrame(int clientSocket, const std::string& message) {
        std::vector<uint8_t> frame;
        frame.push_back(0x81); // FIN + text frame
        
        if (message.size() < 126) {
            frame.push_back(message.size());
        } else if (message.size() < 65536) {
            frame.push_back(126);
            frame.push_back((message.size() >> 8) & 0xFF);
            frame.push_back(message.size() & 0xFF);
        } else {
            frame.push_back(127);
            for (int i = 7; i >= 0; i--) {
                frame.push_back((message.size() >> (i * 8)) & 0xFF);
            }
        }
        
        frame.insert(frame.end(), message.begin(), message.end());
        send(clientSocket, (char*)frame.data(), frame.size(), 0);
    }

    int port_;
    std::atomic<bool> running_{false};
    MessageHandler messageHandler_;
    std::vector<int> clients_;
    std::mutex clientsMutex_;
};

} // namespace hw_analyzer
