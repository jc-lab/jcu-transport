/**
 * @file	tls_transport.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/12/16
 * @copyright Copyright (C) 2019 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef __JCU_TRANSPORT_TLS_TRANSPORT_H__
#define __JCU_TRANSPORT_TLS_TRANSPORT_H__

#include <jcu/transport/transport.h>

#include "ssl_engine.h"

namespace jcu {
    namespace transport {
        class TlsTransport : public Transport {
        private:
            std::weak_ptr<TlsTransport> self_;

            OnConnectCallback_t on_connect_;
            OnCloseCallback_t on_close_;
            OnDataCallback_t on_data_;

            std::shared_ptr<Transport> transport_;
            std::shared_ptr<SslEngine> engine_;
            std::shared_ptr<SslEngine::SocketContext> ssl_socket_;

            TlsTransport(std::shared_ptr<uvw::Loop> loop);

        public:
            static std::shared_ptr<TlsTransport> create(std::shared_ptr<uvw::Loop> loop, std::shared_ptr<Transport> transport, std::shared_ptr<SslEngine> engine);

            virtual ~TlsTransport();

            void connect(const OnConnectCallback_t &on_connect, const OnCloseCallback_t &on_close) override;
            void reconnect() override;
            void disconnect() override;
            void cleanup() override;

            void onData(const OnDataCallback_t& callback) override;
            void write(std::unique_ptr<char[]> data, size_t length) override;
        };
    }
}

#endif //__JCU_TRANSPORT_TLS_TRANSPORT_H__
