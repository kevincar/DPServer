#ifndef INCLUDE_INET_MASTERCONNECTION_HPP_
#define INCLUDE_INET_MASTERCONNECTION_HPP_

#include <map>
#include <memory>
#include <vector>

#include "inet/TCPAcceptor.hpp"
#include "inet/TCPConnection.hpp"
#include "inet/UDPConnection.hpp"
#include "inet/config.hpp"

namespace inet {
class MasterConnection {
 public:
  explicit MasterConnection(double const t = 5.0);

  ~MasterConnection(void);

  typedef std::function<bool(IPConnection const&)> ProcessHandler;

  // Listening Control
  bool isListening(void) const;

  // General Connection Control
  unsigned int getNumTCPAcceptors(void) const;
  unsigned int getNumTCPConnections(void) const;
  unsigned int getNumUDPConnections(void) const;
  unsigned int getNumConnections(void) const;

  // TCP Connection Control
  TCPAcceptor* createTCPAcceptor(TCPAcceptor::AcceptHandler const& pAcceptPH,
                                 TCPAcceptor::ProcessHandler const& pChildPH);
  std::vector<TCPAcceptor const*> getAcceptors(void) const;
  void removeTCPAcceptor(unsigned int acceptConnID);

  // UDP Connection Control
  UDPConnection* createUDPConnection(std::unique_ptr<ProcessHandler>&& pPH);
  std::vector<UDPConnection const*> getUDPConnections(void) const;
  void removeUDPConnection(unsigned int connID);

  std::shared_ptr<TCPConnection> const answerIncomingConnection(void) const;

 private:
  double timeout = 5.0;

  // Thread Management
  std::thread listeningThread;
  std::mutex listeningThread_mutex;

  bool listening = false;
  mutable std::mutex listening_mutex;

  // TCP Connections
  std::vector<std::unique_ptr<TCPAcceptor>> acceptors;
  mutable std::mutex acceptor_mutex;

  // UDP Connections
  std::vector<std::unique_ptr<UDPConnection>> udpConnections;
  mutable std::mutex udp_mutex;

  std::map<const unsigned int, std::unique_ptr<ProcessHandler>> processHandlers;
  mutable std::mutex proc_mutex;

  void stopListening(void);
  void setListeningState(bool state);
  void beginListening();
  void startListening();

  std::unique_ptr<std::vector<IPConnection const*>> getAllConnections(
      void) const;

  // Connection Processing
  bool loadFdSetConnections(fd_set*) const;
  bool loadFdSetTCPConnections(fd_set*) const;
  bool loadFdSetUDPConnections(fd_set*) const;
  int waitForFdSetConnections(fd_set*) const;
  void checkAndProcessConnections();
  void checkAndProcessTCPConnections(fd_set* fdSet);
  void checkAndProcessUDPConnections(fd_set* fdSet);

  int getLargestSocket(void) const;
  int getLargestTCPSocket(void) const;
  int getLargestUDPSocket(void) const;
};
}  // namespace inet

#endif  // INCLUDE_INET_MASTERCONNECTION_HPP_
