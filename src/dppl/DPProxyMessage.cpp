#include "dppl/DPProxyMessage.hpp"

namespace dppl {
DPProxyMessage::DPProxyMessage(std::vector<char> message_data, proxy const& to,
                               proxy const& from)
    : data_(DPProxyMessage::pack_message(message_data, to, from)) {}

DPProxyMessage::DPProxyMessage(std::vector<char> message_data,
                               DPProxyEndpointIDs toIDs, proxy const& from)
    : data_(DPProxyMessage::pack_message(message_data, toIDs, from)) {}

DPProxyMessage::DPProxyMessage(std::vector<char> message_data, proxy const& to,
                               DPProxyEndpointIDs fromIDs)
    : data_(DPProxyMessage::pack_message(message_data, to, fromIDs)) {}

DPProxyMessage::DPProxyMessage(std::vector<char> message_data,
                               DPProxyEndpointIDs toIDs,
                               DPProxyEndpointIDs fromIDs)
    : data_(DPProxyMessage::pack_message(message_data, toIDs, fromIDs)) {}

DPProxyMessage::DPProxyMessage(DPProxyMessage const& other)
    : data_(other.data_) {}

DPProxyMessage::DPProxyMessage(std::vector<char> data) : data_(data) {}

std::vector<char> DPProxyMessage::to_vector() const { return this->data_; }

std::vector<char> DPProxyMessage::get_dp_msg() const {
  std::vector<char> dp_msg;
  DPPROXYMSG const* imsg =
      reinterpret_cast<DPPROXYMSG const*>(&(*this->data_.begin()));
  auto start = this->data_.begin() + sizeof(DPPROXYMSG);
  auto end = this->data_.end();
  dp_msg.assign(start, end);
  return dp_msg;
}

DPProxyEndpointIDs DPProxyMessage::get_to_ids() const {
  DPPROXYMSG const* msg =
      reinterpret_cast<DPPROXYMSG const*>(&(*this->data_.begin()));
  return {msg->toSystemID, msg->toPlayerID};
}

DPProxyEndpointIDs DPProxyMessage::get_from_ids() const {
  DPPROXYMSG const* msg =
      reinterpret_cast<DPPROXYMSG const*>(&(*this->data_.begin()));
  return {msg->fromSystemID, msg->fromPlayerID};
}

std::vector<char> DPProxyMessage::pack_message(std::vector<char> message_data,
                                               proxy const& to,
                                               proxy const& from) {
  DPProxyEndpointIDs toIDs = DPProxyMessage::proxy_to_ids(to);
  DPProxyEndpointIDs fromIDs = DPProxyMessage::proxy_to_ids(from);
  return DPProxyMessage::pack_message(message_data, toIDs, fromIDs);
}

std::vector<char> DPProxyMessage::pack_message(std::vector<char> message_data,
                                               proxy const& to,
                                               DPProxyEndpointIDs fromIDs) {
  DPProxyEndpointIDs toIDs = DPProxyMessage::proxy_to_ids(to);
  return DPProxyMessage::pack_message(message_data, toIDs, fromIDs);
}

std::vector<char> DPProxyMessage::pack_message(std::vector<char> message_data,
                                               DPProxyEndpointIDs toIDs,
                                               proxy const& from) {
  DPProxyEndpointIDs fromIDs = DPProxyMessage::proxy_to_ids(from);
  return DPProxyMessage::pack_message(message_data, toIDs, fromIDs);
}

std::vector<char> DPProxyMessage::pack_message(std::vector<char> message_data,
                                               DPProxyEndpointIDs toIDs,
                                               DPProxyEndpointIDs fromIDs) {
  std::size_t new_len = sizeof(DPPROXYMSG) + message_data.size();
  std::vector<char> retval(new_len, '\0');
  DPPROXYMSG* imsg = reinterpret_cast<DPPROXYMSG*>(&(*retval.begin()));
  imsg->toSystemID = toIDs.systemID;
  imsg->toPlayerID = toIDs.playerID;
  imsg->fromSystemID = fromIDs.systemID;
  imsg->fromPlayerID = fromIDs.playerID;
  char* dest = reinterpret_cast<char*>(&imsg->dp_message);
  std::copy(message_data.begin(), message_data.end(), dest);
  return retval;
}

DPProxyEndpointIDs DPProxyMessage::proxy_to_ids(proxy const& p) {
  DWORD system_id = static_cast<int>(p);
  DWORD player_id = static_cast<uint32_t>(p);
  DPProxyEndpointIDs ids = {system_id, player_id};
  return ids;
}
}  // namespace dppl