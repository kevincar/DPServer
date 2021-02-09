
#ifndef INCLUDE_NATHP_CLIENT_HPP_
#define INCLUDE_NATHP_CLIENT_HPP_

#include <map>
#include <string>
#include <vector>

#include "inet/MasterConnection.hpp"
#include "nathp/ClientRecord.hpp"
#include "nathp/Packet.hpp"
#include "nathp/constants.hpp"
#include "nathp/type.hpp"

namespace nathp {
class Client {
 public:
  explicit Client(std::string server_ip_address, int port = NATHP_PORT,
                  bool start = false);

  void connect(void);
  ClientRecord getClientRecord(void) const;
  std::vector<ClientRecord> requestClientList(void) const noexcept;
  bool connectToPeer(ClientRecord const& peer) noexcept;
  bool createHolepunch(type holepunch_type, ClientRecord const& client_record);

  int reconnection_attempts = -1;

 private:
  bool connectionHandler(inet::IPConnection const& connection);
  void processRequestPacket(Packet const& packet) noexcept;
  void processResponsePacket(Packet const& packet) const noexcept;

  void clearProcResponseData(void) const noexcept;
  int sendPacketTo(Packet const& packet, inet::IPConnection const& conn) const;

  void awaitResponse(Packet* packet) const noexcept;
  unsigned int requestClientID(void) const noexcept;
  std::string requestPublicAddress(void) const noexcept;
  void requestRegisterPrivateAddress(void) const noexcept;
  bool requestUDPHolepunch(ClientRecord const& client_record);

  bool initHolepunch(ClientRecord const& peer_record);

  unsigned int id = -1;

  std::mutex mutable proc_response_mutex;
  std::condition_variable mutable proc_response_cv;
  std::map<Packet::Message, bool> mutable proc_response_ready;
  std::map<Packet::Message, std::vector<uint8_t>> mutable proc_response_data;

  inet::IPConnection::ConnectionHandler server_connection_handler;
  inet::TCPConnection server_connection;
  std::string server_address;

  inet::UDPConnection holepunch;
  inet::IPConnection::ConnectionHandler holepunch_handler;
};
}  // namespace nathp

#endif  // INCLUDE_NATHP_CLIENT_HPP_
