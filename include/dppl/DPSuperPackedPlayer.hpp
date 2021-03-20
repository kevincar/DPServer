#ifndef INCLUDE_DPPL_DPSUPERPACKEDPLAYER_HPP_
#define INCLUDE_DPPL_DPSUPERPACKEDPLAYER_HPP_
#include "experimental/net"
#include "dppl/dplay.h"
namespace dppl {
class DPSuperPackedPlayer {
 public:
  explicit DPSuperPackedPlayer(DPLAYI_SUPERPACKEDPLAYER* player);
  char16_t* getShortName();
  std::size_t getShortNameSize();
  char16_t* getLongName();
  std::size_t getLongNameSize();

  std::size_t getPlayerDataSize();
  char* getPlayerData();

  std::size_t getServiceProviderSize();
  dpsockaddr* getServiceProviders();
  void setStreamEndpoint(std::experimental::net::ip::tcp::endpoint endpoint);
  void setDataEndpoint(std::experimental::net::ip::udp::endpoint endpoint);

  template <typename T>
  void setEndpoint(int n, T endpoint);

  std::size_t getNumPlayerIDs();
  DWORD* getPlayerIDs();

  DWORD* getParentID();

  std::size_t getNumShortcutIDs();
  DWORD* getShortcutIDs();

  std::size_t size();

 private:
  /*
   * These private functions return the pointer to the byte in the data
   * structure where the element should exist even if it doesn't
   */
  char16_t* _getShortName();
  char16_t* _getLongName();
  char* _getPlayerDataSize();
  char* _getPlayerData();
  char* _getServiceProviderSize();
  dpsockaddr* _getServiceProviders();
  char* _getNumPlayerIDs();
  DWORD* _getPlayerIDs();
  DWORD* _getParentID();
  char* _getNumShortcutIDs();
  DWORD* _getShortcutIDs();
  char* _endPtr();

  DPLAYI_SUPERPACKEDPLAYER* sp_data_;
  char* data_;
};

template <typename T>
void DPSuperPackedPlayer::setEndpoint(int n, T endpoint) {
  sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(endpoint.data());
  dpsockaddr* dpaddr = this->getServiceProviders();
  dpaddr += n;
  dpaddr->sin_family = addr->sin_family;
  dpaddr->sin_port = addr->sin_port;
  dpaddr->sin_addr = addr->sin_addr.s_addr;
}
}  // namespace dppl
#endif  // INCLUDE_DPPL_SUPERPACKEDPLAYER_HPP_
