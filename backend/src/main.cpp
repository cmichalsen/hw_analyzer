#include "serial_interface.hpp"
#include "websocket_server.hpp"
#include <iostream>
#include <memory>

using namespace hw_analyzer;

int main(int argc, char* argv[]) {
    std::cout << "HW Analyzer Backend Starting..." << std::endl;
    std::cout << "WebSocket server will listen on ws://localhost:9001" << std::endl;
    
    auto server = std::make_unique<WebSocketServer>(9001);
    auto serial = std::make_shared<SerialInterface>();
    
    // Set up serial data callback
    serial->setDataCallback([&server](const std::string& data) {
        // Send received data to all connected clients
        server->broadcast(R"({"type":"rx","data":")" + data + "\"}");
    });
    
    serial->setErrorCallback([&server](const std::string& error) {
        server->broadcast(R"({"type":"error","message":")" + error + "\"}");
    });
    
    // Handle WebSocket messages
    server->setMessageHandler([&serial](const std::string& message) -> std::string {
        std::cout << "Received: " << message << std::endl;
        
        // Simple JSON parsing (for MVP - use nlohmann/json in production)
        if (message.find("\"cmd\":\"list\"") != std::string::npos) {
            auto ports = SerialInterface::listPorts();
            std::string response = R"({"type":"ports","data":[)";
            for (size_t i = 0; i < ports.size(); i++) {
                if (i > 0) response += ",";
                response += R"({"name":")" + ports[i].name + 
                           R"(","desc":")" + ports[i].description + "\"}";
            }
            response += "]}";
            return response;
        }
        else if (message.find("\"cmd\":\"open\"") != std::string::npos) {
            // Extract port name and baud rate (simple parsing)
            size_t portPos = message.find("\"port\":\"");
            size_t baudPos = message.find("\"baud\":");
            
            if (portPos != std::string::npos && baudPos != std::string::npos) {
                portPos += 8; // length of "port":"
                size_t portEnd = message.find("\"", portPos);
                std::string port = message.substr(portPos, portEnd - portPos);
                
                baudPos += 7; // length of "baud":
                size_t baudEnd = message.find_first_of(",}", baudPos);
                int baud = std::stoi(message.substr(baudPos, baudEnd - baudPos));
                
                if (serial->open(port, baud)) {
                    serial->startReadLoop();
                    return R"({"type":"status","message":"Port opened successfully"})";
                }
                return R"({"type":"error","message":"Failed to open port"})";
            }
        }
        else if (message.find("\"cmd\":\"write\"") != std::string::npos) {
            size_t dataPos = message.find("\"data\":\"");
            if (dataPos != std::string::npos) {
                dataPos += 8;
                size_t dataEnd = message.find("\"", dataPos);
                std::string data = message.substr(dataPos, dataEnd - dataPos);
                
                // Unescape newlines
                size_t pos = 0;
                while ((pos = data.find("\\n", pos)) != std::string::npos) {
                    data.replace(pos, 2, "\n");
                    pos += 1;
                }
                
                if (serial->write(data)) {
                    return R"({"type":"status","message":"Data sent"})";
                }
                return R"({"type":"error","message":"Failed to send data"})";
            }
        }
        else if (message.find("\"cmd\":\"close\"") != std::string::npos) {
            serial->stopReadLoop();
            serial->close();
            return R"({"type":"status","message":"Port closed"})";
        }
        
        return R"({"type":"error","message":"Unknown command"})";
    });
    
    std::cout << "Backend ready. Waiting for connections..." << std::endl;
    server->run();
    
    return 0;
}
