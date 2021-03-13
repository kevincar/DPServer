#ifndef INCLUDE_DPPL_DPMESSAGE_HPP_
#define INCLUDE_DPPL_DPMESSAGE_HPP_
#include <experimental/net>
#include <iostream>
#include <vector>

#include "dppl/dplay.h"
namespace dppl {
class DPMessage {
 public:
  template <typename T>
  explicit DPMessage(T* message_data);

  template <typename T>
  explicit DPMessage(std::vector<T>* message_data);

  DPMSG_HEADER* header();

  template <typename T>
  void set_return_addr(T endpoint);
  template <typename T>
  T get_return_addr();

  void set_signature();

  template <typename T>
  T* message();

  template <typename T>
  T* property_data(int offset);

  static int const kSignatureOffset = sizeof(DWORD) + sizeof(sockaddr_in);

 private:
  char* data_;
};

template <typename T>
DPMessage::DPMessage(T* message_data)
    : data_(reinterpret_cast<char*>(message_data)) {}

template <typename T>
DPMessage::DPMessage(std::vector<T>* message_data)
    : data_(reinterpret_cast<char*>(&(*message_data->begin()))) {}

template <typename T>
void DPMessage::set_return_addr(T endpoint) {
  sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(endpoint.data());
  dpsockaddr* paddr = &this->header()->sockAddr;
  paddr->sin_family = addr->sin_family;
  paddr->sin_port = addr->sin_port;
  paddr->sin_addr = addr->sin_addr.s_addr;
}

template <typename T>
T DPMessage::get_return_addr() {
  dpsockaddr* paddr = &this->header()->sockAddr;
  uint16_t port = paddr->sin_port;
  uint32_t ad = paddr->sin_addr;
  port = ((port & 0xff00) >> 8) | ((port & 0xff) << 8);
  ad = ((ad & 0xff000000) >> 24) | ((ad & 0xff0000) >> 8) |
       ((ad & 0xff00) << 8) | ((ad & 0xff) << 24);
  return T(std::experimental::net::ip::address_v4(ad), port);
}

template <typename T>
T* DPMessage::message() {
  return reinterpret_cast<T*>(this->data_ + sizeof(DPMSG_HEADER));
}

template <typename T>
T* DPMessage::property_data(int offset) {
  return reinterpret_cast<T*>(this->data_ + this->kSignatureOffset + offset);
}

}  // namespace dppl

#endif  // INCLUDE_DPPL_DPMESSAGE_HPP_
