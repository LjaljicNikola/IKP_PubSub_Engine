#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "Message.h"
#include <vector>
#include <cstring>
#include <cstdint>

class Serialization {
public:
    // Serialize a Message to binary format
    // Format: [protocol_version(1)] [topic_len(1)] [topic(var)] [host_len(1)] [host(var)] [port(4)] [type(1)] [topicType(1)] [data(4)] [timestamp(8)]
    static std::vector<uint8_t> serialize(const Message& msg) {
        std::vector<uint8_t> buffer;
        
        // Protocol version (1 byte)
        buffer.push_back(1);
        
        // Topic length (1 byte)
        uint8_t topic_len = strlen(msg.topic);
        buffer.push_back(topic_len);
        
        // Topic string
        for (int i = 0; i < topic_len; i++) {
            buffer.push_back(msg.topic[i]);
        }
        
        // Publisher host length (1 byte)
        uint8_t host_len = strlen(msg.publisher_host);
        buffer.push_back(host_len);
        
        // Publisher host string
        for (int i = 0; i < host_len; i++) {
            buffer.push_back(msg.publisher_host[i]);
        }
        
        // Publisher port (4 bytes, big-endian)
        int port = msg.publisher_port;
        buffer.push_back((port >> 24) & 0xFF);
        buffer.push_back((port >> 16) & 0xFF);
        buffer.push_back((port >> 8) & 0xFF);
        buffer.push_back(port & 0xFF);
        
        // MessageType enum (1 byte)
        buffer.push_back(static_cast<uint8_t>(msg.type));
        
        // TopicType enum (1 byte)
        buffer.push_back(static_cast<uint8_t>(msg.topicType));
        
        // Union data (4 bytes) - interpret as uint32_t
        uint32_t data_value;
        if (msg.type == MessageType::ANALOG) {
            // Copy float bits as uint32_t
            memcpy(&data_value, &msg.data.analogValue, sizeof(float));
        } else {
            // StatusValue is enum, fits in uint32_t
            data_value = static_cast<uint32_t>(msg.data.statusValue);
        }
        
        // Add bytes in big-endian order
        buffer.push_back((data_value >> 24) & 0xFF);
        buffer.push_back((data_value >> 16) & 0xFF);
        buffer.push_back((data_value >> 8) & 0xFF);
        buffer.push_back(data_value & 0xFF);
        
        // Timestamp (8 bytes) - long
        long ts = msg.timestamp;
        for (int i = 7; i >= 0; i--) {
            buffer.push_back((ts >> (i * 8)) & 0xFF);
        }
        
        return buffer;
    }
    
    // Deserialize binary format back to Message
    static Message deserialize(const uint8_t* data, size_t len) {
        Message msg;
        
        if (len < 20) {
            return msg; // Invalid data, return default message
        }
        
        size_t pos = 0;
        
        // Skip protocol version
        pos++;
        
        // Read topic length
        uint8_t topic_len = data[pos++];
        
        // Read topic string
        if (pos + topic_len <= len) {
            memcpy(msg.topic, &data[pos], topic_len);
            msg.topic[topic_len] = '\0';
            pos += topic_len;
        }
        
        // Read publisher host length
        if (pos < len) {
            uint8_t host_len = data[pos++];
            
            // Read publisher host string
            if (pos + host_len <= len) {
                memcpy(msg.publisher_host, &data[pos], host_len);
                msg.publisher_host[host_len] = '\0';
                pos += host_len;
            }
        }
        
        // Read publisher port (4 bytes, big-endian)
        if (pos + 4 <= len) {
            msg.publisher_port = ((int)data[pos] << 24) |
                                  ((int)data[pos+1] << 16) |
                                  ((int)data[pos+2] << 8) |
                                  (int)data[pos+3];
            pos += 4;
        }
        
        // Read MessageType
        if (pos < len) {
            msg.type = static_cast<MessageType>(data[pos++]);
        }
        
        // Read TopicType
        if (pos < len) {
            msg.topicType = static_cast<TopicType>(data[pos++]);
        }
        
        // Read union data (4 bytes) in big-endian
        if (pos + 4 <= len) {
            uint32_t data_value = ((uint32_t)data[pos] << 24) |
                                   ((uint32_t)data[pos+1] << 16) |
                                   ((uint32_t)data[pos+2] << 8) |
                                   (uint32_t)data[pos+3];
            pos += 4;
            
            if (msg.type == MessageType::ANALOG) {
                memcpy(&msg.data.analogValue, &data_value, sizeof(float));
            } else {
                msg.data.statusValue = static_cast<StatusValue>(data_value);
            }
        }
        
        // Read timestamp (8 bytes) in big-endian
        if (pos + 8 <= len) {
            long ts = 0;
            for (int i = 0; i < 8; i++) {
                ts = (ts << 8) | data[pos + i];
            }
            msg.timestamp = ts;
            pos += 8;
        }
        
        return msg;
    }
};

#endif // SERIALIZATION_H
