/**
 * @file	tcp_transport.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/12/16
 * @copyright Copyright (C) 2019 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef __JCU_TRANSPORT_TCP_TRANSPORT_H__
#define __JCU_TRANSPORT_TCP_TRANSPORT_H__

#include <jcu/transport/transport.h>

#include <uvw/tcp.hpp>

namespace jcu {
    namespace transport {
        class TcpTransport : public Transport {
        private:
            std::weak_ptr<TcpTransport> self_;

            OnConnectCallback_t on_connect_;
            OnCloseCallback_t on_close_;
            OnErrorCallback_t on_error_;
            OnDataCallback_t on_data_;

            std::shared_ptr<uvw::TCPHandle> sock_handle_;

            std::string remote_ip_;
            int remote_port_;

            TcpTransport(std::shared_ptr<uvw::Loop> loop);

        public:
            static std::shared_ptr<TcpTransport> create(std::shared_ptr<uvw::Loop> loop);

            virtual ~TcpTransport();

            void setRemote(const std::string& remote_ip, int remote_port);

            void connect(const OnConnectCallback_t &on_connect, const OnCloseCallback_t &on_close, const OnErrorCallback_t& on_error) override;
            void reconnect() override;
            void disconnect() override;
            void cleanup() override;

            void onData(const OnDataCallback_t& callback) override;
            void write(std::unique_ptr<char[]> data, size_t length) override;
        };
    }
}

#endif //__JCU_TRANSPORT_TCP_TRANSPORT_H__
