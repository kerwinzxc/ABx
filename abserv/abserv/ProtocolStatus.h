#pragma once

#include "Protocol.h"
#include "Connection.h"

namespace Net {

class ProtocolStatus final : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = false };
    enum { ProtocolIdentifier = 0xFF };
    enum { UseChecksum = false };
    static const char* ProtocolName() { return "Status Protocol"; };
private:
    using MapIter = std::map<uint32_t, int64_t>::const_iterator;
    static std::map<uint32_t, int64_t> ipConnectMap_;
    void SendStatusString();
    void SendInfo(uint16_t requestedInfo);
public:
    explicit ProtocolStatus(std::shared_ptr<Connection> connection) :
        Protocol(connection)
    {}

    void OnRecvFirstMessage(NetworkMessage& message) final;
};

}
