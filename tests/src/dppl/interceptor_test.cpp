#include "dppl/AppSimulator.hpp"
#include "dppl/hardware_test.hpp"
#include "dppl/interceptor.hpp"
#include "experimental/net"
#include "g3log/g3log.hpp"
#include "gtest/gtest.h"

// Helper function for tests below
TEST(interceptorTest, host_test) {
  std::vector recv_buf(512, '\0');
  std::vector send_buf(512, '\0');
  auto dpsrvr_duration = std::chrono::seconds(5);
  auto transmission_duration = std::chrono::milliseconds(750);
  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer dpsrvr_timer(io_context,
                                                    dpsrvr_duration);
  std::experimental::net::steady_timer internet_timer(io_context,
                                                      transmission_duration);
  std::shared_ptr<dppl::interceptor> interceptor;
  std::vector<uint8_t> enumsession = {
      0x34, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c,
      0x61, 0x79, 0x02, 0x00, 0x0e, 0x00, 0xc0, 0x13, 0x06, 0xbf, 0x79,
      0xde, 0xd0, 0x11, 0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad, 0x4b,
      0x00, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00,
  };

  std::function<void(std::error_code const& ec)> dpsrvr_callback =
      [&](std::error_code const& ec) {
        if (!ec) {
          LOG(DEBUG) << "dpsrver timer sending data";
          send_buf.assign(enumsession.begin(), enumsession.end());
          dppl::DPProxyMessage proxy_message(send_buf, {0, 0}, {0, 0});
          interceptor->dp_deliver(proxy_message.to_vector());
          dpsrvr_timer.expires_at(std::chrono::steady_clock::now() +
                                  dpsrvr_duration);
          dpsrvr_timer.async_wait(dpsrvr_callback);
        } else {
          LOG(DEBUG) << "dpsrvr timer error: " << ec.message();
        }
      };

  std::function<void(std::error_code const&)> internet_callback =
      [&](std::error_code const& ec) {
        if (!ec) {
          dppl::DPProxyMessage proxy_message(recv_buf);
          std::vector<char> dp_message = proxy_message.get_dp_msg();
          send_buf = dppl::AppSimulator::process_message(dp_message);
          // Swap the to and from IDs
          dppl::DPProxyMessage send_proxy_message(send_buf,
                                                  proxy_message.get_from_ids(),
                                                  proxy_message.get_to_ids());

          dppl::DPMessage request(&send_buf);
          LOG(DEBUG) << "Received request from joining peer (id "
                     << send_proxy_message.get_from_ids().systemID
                     << "). Sending back to interceptor. Received: "
                     << request.header()->command;
          switch (request.header()->command) {
            case DPSYS_REQUESTPLAYERID:
              // Nothing to handle
              break;
            case DPSYS_ADDFORWARDREQUEST:
              // Nothing to handle
              break;
            case DPSYS_CREATEPLAYER:
              // Nothing to handle
              break;
            default:
              LOG(DEBUG) << "Unhandled message from server "
                         << request.header()->command;
              return;
          }
          interceptor->dp_deliver(send_proxy_message.to_vector());
        } else {
          LOG(DEBUG) << "internet timer error: " << ec.message();
        }
      };

  std::function<void(std::vector<char>)> send_to_peer =
      [&](std::vector<char> buffer) {
        LOG(DEBUG) << "Sending over the internet back to the peer";
        recv_buf = buffer;
        internet_timer.expires_at(std::chrono::steady_clock::now() +
                                  transmission_duration);
        internet_timer.async_wait(internet_callback);
      };

  std::function<void(std::vector<char>)> dp_callback =
      [&](std::vector<char> buffer) {
        dppl::DPProxyMessage proxy_message(buffer);
        LOG(DEBUG) << "interceptor dp callback from "
                   << proxy_message.get_from_ids().systemID;
        std::vector<char> dp_message = proxy_message.get_dp_msg();
        dppl::DPMessage response(&dp_message);
        if (response.header()->command == DPSYS_ENUMSESSIONSREPLY) {
          dpsrvr_timer.cancel();
        }
        // Simulate the send accross the internet!
        send_to_peer(buffer);
      };

  std::function<void(std::vector<char>)> data_callback =
      [&](std::vector<char> buffer) {
        LOG(DEBUG) << "interceptor data callback";
        dppl::DPProxyMessage proxy_message(buffer);
        std::vector<char> data_message = proxy_message.get_dp_msg();
        DWORD* id = reinterpret_cast<DWORD*>(&(*data_message.begin()));
        ASSERT_EQ(proxy_message.get_from_ids().playerID, *id);
        std::experimental::net::defer([&]() { io_context.stop(); });
      };

  interceptor = std::make_shared<dppl::interceptor>(&io_context, dp_callback,
                                                    data_callback);
  dppl::AppSimulator app(&io_context, true);
  dpsrvr_timer.async_wait(dpsrvr_callback);
  io_context.run();
}

TEST(interceptorTest, join_test) {
  auto internet_delay = std::chrono::milliseconds(750);
  std::vector<char> recv_buf;
  std::vector<char> send_buf;
  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer internet_timer(io_context,
                                                      internet_delay);
  std::shared_ptr<dppl::interceptor> interceptor;
  std::function<void(std::error_code const&)> internet_callback =
      [&](std::error_code const& ec) {
        if (!ec) {
          dppl::DPProxyMessage recv_proxy_message(recv_buf);
          std::vector<char> recv_dp_msg = recv_proxy_message.get_dp_msg();
          send_buf = dppl::AppSimulator::process_message(recv_dp_msg);
          dppl::DPMessage recv_dp_message(&recv_dp_msg);
          dppl::DPMessage send_dp_message(&send_buf);
          dppl::DPProxyMessage send_proxy_message(
              send_buf, recv_proxy_message.get_from_ids(),
              recv_proxy_message.get_to_ids());

          // End of test check
          if (recv_dp_message.header()->command == DPSYS_CREATEPLAYER) {
            DWORD* ptr = reinterpret_cast<DWORD*>(&(*send_buf.begin()));
            DWORD data_to_id = *(++ptr);
            DWORD pm_to_id = send_proxy_message.get_to_ids().playerID;
            ASSERT_EQ(data_to_id, pm_to_id);
            std::experimental::net::defer([&]() { io_context.stop(); });
          }
          LOG(DEBUG) << "Received data from host. Command: "
                     << send_dp_message.header()->command;
          interceptor->dp_deliver(send_proxy_message.to_vector());
        } else {
          LOG(DEBUG) << "Internet timer error: " << ec.message();
        }
      };
  std::function<void(std::vector<char>)> send_to_internet =
      [&](std::vector<char> buffer) {
        LOG(DEBUG) << "Sending data over internet...";
        recv_buf = buffer;
        internet_timer.expires_at(internet_timer.expiry() + internet_delay);
        internet_timer.async_wait(internet_callback);
      };
  std::function<void(std::vector<char>)> dp_callback =
      [&](std::vector<char> buffer) {
        dppl::DPProxyMessage proxy_message(buffer);
        std::vector<char> dp_msg = proxy_message.get_dp_msg();
        dppl::DPMessage dp_message(&dp_msg);
        if (dp_message.header()->command > DPSYS_CREATEPLAYERVERIFY) {
          dp_message = dppl::DPMessage(&buffer);
          proxy_message = dppl::DPProxyMessage(buffer, {0, 0}, {0, 0});
        }
        LOG(DEBUG) << "interceptor dp callback. Command: "
                   << dp_message.header()->command;
        send_to_internet(proxy_message.to_vector());
      };
  std::function<void(std::vector<char>)> data_callback =
      [&](std::vector<char> buffer) {
        LOG(DEBUG) << "interceptor data callback";
      };
  interceptor = std::make_shared<dppl::interceptor>(&io_context, dp_callback,
                                                    data_callback);
  dppl::AppSimulator app(&io_context, false);
  io_context.run();
}

TEST(interceptorTest, join) {
  if (!(hardware_test_check() || test_check("TEST_INTER_JOIN")))
    return SUCCEED();
  std::vector<char> request_data(512, '\0');
  std::vector<char> response_data(512, '\0');
  std::shared_ptr<std::function<void(std::vector<char> const&)>>
      send_to_internet;

  std::experimental::net::io_context io_context;
  std::experimental::net::steady_timer timer(io_context,
                                             std::chrono::milliseconds(100));
  dppl::interceptor interceptor(
      &io_context,
      [&](std::vector<char> const& buffer) {
        std::vector<char> buf = buffer;
        dppl::DPMessage packet(&buf);
        LOG(DEBUG) << "ETHERIAL SPACE: RECEIVED DATA: MESSAGE ID: "
                   << packet.header()->command;

        // Simulate sending this off to the internet
        (*send_to_internet)(buffer);
      },
      [&](std::vector<char> const& buffer) {});

  send_to_internet =
      std::make_shared<std::function<void(std::vector<char> const&)>>(
          [&](std::vector<char> const& buffer) {
            LOG(DEBUG) << "Sending to the internet :)";
            request_data = buffer;
            timer.async_wait([&](std::error_code const& ec) {
              if (ec) {
                LOG(DEBUG) << "Timer failed: " << ec.message();
                return;
              }
              response_data = dppl::AppSimulator::process_message(request_data);
              LOG(DEBUG) << "Got a response from the internet";
              interceptor.dp_deliver(response_data);
              timer.expires_at(timer.expiry() + std::chrono::milliseconds(100));
            });
          });

  LOG(INFO) << "Go Ahead";
  io_context.run();
}

TEST(interceptorTest, host) {
  if (!(hardware_test_check() || test_check("TEST_INTER_HOST")))
    return SUCCEED();

  // Need this again :)
  GUID app = {0xbf0613c0, 0xde79, 0x11d0, 0x99, 0xc9, 0x00,
              0xa0,       0x24,   0x76,   0xad, 0x4b};
  int simulated_internet_delay = 100;  // In milliseconds

  // Buffers for sending and receiving data
  std::vector<char> request_data(512, '\0');
  std::vector<char> response_data(512, '\0');

  // A pointer to a function defined later for simulating sending data over the
  // internet
  std::shared_ptr<std::function<void(std::vector<char> const&)>>
      send_to_internet;

  std::experimental::net::io_context io_context;

  // Timer for modulating transmission over our simulated internet
  std::experimental::net::steady_timer timer(
      io_context, std::chrono::milliseconds(simulated_internet_delay));

  // Timer for emitting ENUMSESSION requests
  std::experimental::net::steady_timer dpsrvr_timer(io_context,
                                                    std::chrono::seconds(5));

  // Interceptor
  dppl::interceptor interceptor(
      &io_context,
      [&](std::vector<char> const& buffer) {
        std::vector<char> buf = buffer;
        dppl::DPMessage packet(&buf);
        LOG(DEBUG) << "ETHERIAL SPACE: RECEIVED DATA: MESSAGE ID: "
                   << packet.header()->command;

        // Sned off to the interet
        (*send_to_internet)(buffer);
      },
      [&](std::vector<char> const& buffer) {});

  // function to send stuff off to the internet
  send_to_internet =
      std::make_shared<std::function<void(std::vector<char> const&)>>(
          [&](std::vector<char> const& buffer) {
            LOG(DEBUG) << "Sending off to the internet :)";
            request_data = buffer;

            // This is for receiving data from the "internet"
            timer.async_wait([&](std::error_code const& ec) {
              if (ec) {
                LOG(DEBUG) << "Timer Failed: " << ec.message();
                return;
              }
              response_data = dppl::AppSimulator::process_message(request_data);
              LOG(DEBUG) << "Incomming data from the internet";

              // If we received an ENUMSESSIONREPLY let's stop our timer
              dppl::DPMessage response(&response_data);
              if (response.header()->command == DPSYS_ENUMSESSIONSREPLY) {
                dpsrvr_timer.cancel();
              }
              interceptor.dp_deliver(response_data);
              timer.expires_at(timer.expiry() + std::chrono::milliseconds(
                                                    simulated_internet_delay));
            });
          });

  // Call back for our DPSRVR timer
  std::function<void(std::error_code const&)> timer_callback =
      [&](std::error_code const& ec) {
        if (!ec) {
          response_data.resize(512, '\0');
          dppl::DPMessage request(&response_data);
          request.header()->cbSize =
              sizeof(DPMSG_HEADER) + sizeof(DPMSG_ENUMSESSIONS);
          request.header()->token = 0xfab;
          request.set_signature();
          request.header()->command = DPSYS_ENUMSESSIONS;
          request.header()->version = 0xe;
          DPMSG_ENUMSESSIONS* msg = request.message<DPMSG_ENUMSESSIONS>();
          msg->guidApplication = app;
          msg->dwFlags = ENUMSESSIONSFLAGS::allsessions |
                         ENUMSESSIONSFLAGS::passwordprotectedsessions;

          response_data.resize(request.header()->cbSize);

          LOG(DEBUG) << "Timer: Sending sample ENUMSESSIONS request";
          interceptor.dp_deliver(response_data);
          dpsrvr_timer.expires_at(dpsrvr_timer.expiry() +
                                  std::chrono::seconds(5));
          dpsrvr_timer.async_wait(timer_callback);
        } else {
          LOG(WARNING) << "Timer Error: " << ec.message();
        }
      };
  dpsrvr_timer.async_wait(timer_callback);

  LOG(INFO) << "Go Host!";
  io_context.run();
}
