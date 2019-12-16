/**
 * @file	openssl_ssl_engine.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/12/16
 * @copyright Copyright (C) 2019 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef __JCU_TRANSPORT_OPENSSL_SSL_ENGINE_H__
#define __JCU_TRANSPORT_OPENSSL_SSL_ENGINE_H__

#include <jcu/transport/config.h>

#include "ssl_engine.h"

#ifdef JCU_TRANSPORT_HAS_OPENSSL

#include <openssl/ssl.h>

#include <deque>

namespace jcu {
    namespace transport {
        class OpensslSslEngine : public SslEngine {
        public:
            class OpensslSocketContext : public SslEngine::SocketContext {
            private:
                enum OpType {
                    OP_HANDSHAKE,
                    OP_READ,
                    OP_WRITE,
                    OP_SHUTDOWN
                };

            public:
                OpensslSocketContext();
                virtual ~OpensslSocketContext();

                void handshake() override;
                void disconnect() override;

                void write(std::unique_ptr<char[]> data, size_t length) override;
                int feedRead(std::unique_ptr<char[]> data, size_t length) override;

                int tlsOperation(OpType op, void *buf, int sz);

                int sendPending();

                std::weak_ptr<OpensslSocketContext> self_;
                std::weak_ptr<Transport> transport_;

                HandshakeCallback_t handshake_callback_;
                WriteCallback_t write_callback_;
                ReadCallback_t read_callback_;
                CloseCallback_t close_callback_;
                ErrorCallback_t error_callback_;

                //Our BIO, all IO should be through this
                BIO     *app_bio_;
                SSL     *ssl_;

                BIO     *ssl_bio_; //the ssl BIO used only by openSSL
            };

        private:
            SSL_CTX *ssl_ctx_;

        public:
            static std::shared_ptr<OpensslSslEngine> create(const SSL_METHOD *meth);
            std::shared_ptr<SocketContext> createContext(std::shared_ptr<Transport> transport,
                                                         HandshakeCallback_t handshake_callback,
                                                         WriteCallback_t write_callback,
                                                         ReadCallback_t read_callback,
                                                         CloseCallback_t close_callback,
                                                         ErrorCallback_t error_callback) override;

            SSL_CTX *getOpensslSslCtx();
        };
    }
}

#endif // JCU_TRANSPORT_HAS_OPENSSL

#endif //__JCU_TRANSPORT_OPENSSL_SSL_ENGINE_H__
