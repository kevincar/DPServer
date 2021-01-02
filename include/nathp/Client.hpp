
#ifndef INCLUDE_NATHP_CLIENT_HPP_
#define INCLUDE_NATHP_CLIENT_HPP_

#include <map>
#include <string>
#include <vector>

#include "inet/MasterConnection.hpp"
#include "nathp/ClientRecord.hpp"
#include "nathp/Packet.hpp"
#include "nathp/constants.hpp"

namespace nathp {
class Client {
 public:
  explicit Client(std::string server_ip_address, int port = NATHP_PORT,
                  bool start = true);

  void connect(void);
  // bool connectToPeer(ClientRecord const& peer) const noexcept;

  int reconnection_attempts = -1;

 private:
  bool connectionHandler(inet::IPConnection const& connection);
  void processPacket(Packet const& packet) const noexcept;

  void clearProcResponseData(void) const noexcept;
  int sendPacketTo(Packet const& packet, inet::IPConnection const& conn) const;

  unsigned int requestClientID(void) const noexcept;
  std::string requestPublicAddress(void) const noexcept;
  // std::vector<ClientRecord> requestClientList(void) const noexcept;

  unsigned int id = -1;

  std::mutex mutable proc_response_mutex;
  std::condition_variable mutable proc_response_cv;
  std::map<Packet::Message, bool> mutable proc_response_ready;
  std::map<Packet::Message, std::vector<uint8_t>> mutable proc_response_data;

  inet::IPConnection::ConnectionHandler connection_handler;
  inet::TCPConnection server_connection;
  std::string server_address;
};
}  // namespace nathp

#endif  // INCLUDE_NATHP_CLIENT_HPP_
