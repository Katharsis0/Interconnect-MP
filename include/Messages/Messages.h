//
// Created by katharsis on 4/28/25.
//

#ifndef MESSAGE_H
#define MESSAGE_H


#pragma once

#include <cstdint>
#include <vector>
#include <variant>
#include <string>

//Tipos de mensajes
enum class MessageType {
    WRITE_MEM,
    READ_MEM,
    BROADCAST_INVALIDATE,
    INV_ACK,
    INV_COMPLETE,
    READ_RESP,
    WRITE_RESP
};

//Estructura para todos los mensajes
struct MessageBase {
    MessageType type;
    uint8_t src;        //PE fuente (0x00-0x07)
    uint8_t qos;        //Prioridad (0x00-0xFF)
    uint32_t addr;
    uint64_t timestamp; //Tiempo de simulación

    virtual ~MessageBase() = default;
    virtual std::string toString() const = 0;
};

//WRITE_MEM: Escribir en memoria
struct WriteMemMessage : MessageBase {
    uint32_t addr;                  //mem address
    uint8_t num_of_cache_lines;     //lineas de cache
    uint32_t start_cache_line;      //primera linea de cache
    std::vector<uint8_t> data;      //Datos

    WriteMemMessage() {
        type = MessageType::WRITE_MEM;
    }

    std::string toString() const override;
};

//READ_MEM: Leer de memoria
struct ReadMemMessage : public MessageBase {
    uint32_t addr;
    uint16_t size;

    ReadMemMessage() {
        type = MessageType::READ_MEM;
    }

    std::string toString() const override;
};

//BROADCAST_INVALIDATE: Invalidar caché
struct BroadcastInvalidateMessage : public MessageBase {
    uint32_t src_cache_line; //Línea del caché a invalidar

    BroadcastInvalidateMessage() {
        type = MessageType::BROADCAST_INVALIDATE;
    }

    std::string toString() const override;
};

//INV_ACK: Acknowledgement del invalidate
struct InvAckMessage : public MessageBase {
    InvAckMessage() {
        type = MessageType::INV_ACK;
    }

    std::string toString() const override;
};

//INV_COMPLETE: Invalidación completada
struct InvCompleteMessage : public MessageBase {
    uint8_t dest; //PE destino (quien generó el BROADCAST_INVALIDATE)

    InvCompleteMessage() {
        type = MessageType::INV_COMPLETE;
    }

    std::string toString() const override;
};

//READ_RESP: Leer respuesta
struct ReadRespMessage : public MessageBase {
    uint8_t dest;               //PE destino
    std::vector<uint8_t> data;  //Datos a leer

    ReadRespMessage() {
        type = MessageType::READ_RESP;
    }

    std::string toString() const override;
};

//WRITE_RESP: Escribir respuesta
struct WriteRespMessage : public MessageBase {
    uint8_t dest;       //PE destino
    uint8_t status;     //0x1: OK, 0x0: Not_OK

    WriteRespMessage() {
        type = MessageType::WRITE_RESP;
    }

    std::string toString() const override;
};

//Tipo de variante que engloba todos los tipos de mensajes
using Message = std::variant<
    WriteMemMessage,
    ReadMemMessage,
    BroadcastInvalidateMessage,
    InvAckMessage,
    InvCompleteMessage,
    ReadRespMessage,
    WriteRespMessage
>;

//Utils de mensaje
MessageType getMessageType(const Message& msg);
uint8_t getMessageSource(const Message& msg);
uint8_t getMessageQoS(const Message& msg);
std::string messageToString(const Message& msg);
size_t calculateMessageSize(const Message& msg);
#endif //MESSAGE_H
