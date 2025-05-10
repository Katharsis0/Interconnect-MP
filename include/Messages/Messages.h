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
    uint64_t timestamp; //Tiempo de simulación

    virtual ~MessageBase() = default;
    virtual std::string toString() const = 0;
};

//WRITE_MEM: Escribir en memoria
//PARAMETROS: SRC, ADDR, NUM_OF_CACHE_LINES, START_CACHE_LINE, QoS
struct WriteMemMessage : public MessageBase {
    uint32_t addr;                  //mem address
    uint8_t num_of_cache_lines;     //lineas de cache
    uint32_t start_cache_line;      //primera linea de cache
    std::vector<uint32_t> data;      //Datos

    WriteMemMessage() {
        type = MessageType::WRITE_MEM;
    }

    std::string toString() const override;
};

//READ_MEM: Leer de memoria
//PARAMETROS:SRC, ADDR, SIZE, QoS
struct ReadMemMessage : public MessageBase {
    uint16_t src;
    uint32_t addr;
    uint16_t size;
    uint8_t qos;

    ReadMemMessage() {
        type = MessageType::READ_MEM;
    }

    std::string toString() const override;
};

//BROADCAST_INVALIDATE: Invalidar caché
//PARAMETROS: SRC, CACHE_LINE, QoS

struct BroadcastInvalidateMessage : public MessageBase {
    uint32_t src_cache_line; //Línea del caché a invalidar

    BroadcastInvalidateMessage() {
        type = MessageType::BROADCAST_INVALIDATE;
    }

    std::string toString() const override;
};

//INV_ACK: Acknowledgement del invalidate
//PARAMETROS SRC, QoS
struct InvAckMessage : public MessageBase {
    InvAckMessage() {
        type = MessageType::INV_ACK;
    }

    std::string toString() const override;
};

//INV_COMPLETE: Invalidación completada
//PARAMETROS: DEST, QoS
struct InvCompleteMessage : public MessageBase {
    uint8_t dest; //PE destino (quien generó el BROADCAST_INVALIDATE)

    InvCompleteMessage() {
        type = MessageType::INV_COMPLETE;
    }

    std::string toString() const override;
};

//READ_RESP: Leer respuesta
//PARAMETROS: DEST, DATA, QoS
struct ReadRespMessage : public MessageBase {
    uint8_t dest;               //PE destino
    std::vector<uint32_t> data;  //Datos a leer

    ReadRespMessage() {
        type = MessageType::READ_RESP;
    }

    std::string toString() const override;
};

//WRITE_RESP: Escribir respuesta
//PARAMETROS: DEST, STATUS, QoS
struct WriteRespMessage : public MessageBase {
    uint8_t dest;       //PE destino
    uint8_t status;     //0x1: OK, 0x0: Not_OK
    std::vector<uint32_t> data;

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
MessageType setMessageType(const Message& msg);
uint8_t getMessageSource(const Message& msg);
uint32_t getMessageAddress(const Message& msg);
//const std::vector<unsigned char>& getMessageData(const Message& msg)

const std::vector<uint32_t>&getMessageData (const Message& msg);
size_t getMessageSize(const Message& msg);
uint8_t getMessageQoS(const Message& msg);
uint8_t getNumCacheLines(const Message& msg);
uint32_t getCacheLine(const Message& msg);
std::string messageToString(const Message& msg);
size_t calculateMessageSize(const Message& msg);
#endif //MESSAGE_H
