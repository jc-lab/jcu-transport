/**
 * @file	ssl_engine.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/12/16
 * @copyright Copyright (C) 2019 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef __JCU_TRANSPORT_SSL_ENGINE_H__
#define __JCU_TRANSPORT_SSL_ENGINE_H__

#include <jcu/transport/transport.h>

#include <memory>
#include <functional>

#include "error.h"

namespace jcu {
    namespace transport {
        /**
         * One-time initialize per communication-application
         * @return
         */
        class SslEngine {
        public:
            class SocketContext;
            typedef std::function<void(SocketContext *ssl_socket_ctx, int status)> HandshakeCallback_t;
            typedef std::function<void(SocketContext *ssl_socket_ctx, int status)> WriteCallback_t;
            typedef std::function<void(SocketContext *ssl_socket_ctx, const char *buf, int size)> ReadCallback_t;
            typedef std::function<void(SocketContext *ssl_socket_ctx, int status)> CloseCallback_t;
            typedef std::function<void(SocketContext *ssl_socket_ctx, Error& err)> ErrorCallback_t;

            class SocketContext {
            public:
                virtual ~SocketContext() {};

                virtual void handshake() = 0;
                virtual void disconnect() = 0;

                virtual void write(std::unique_ptr<char[]> data, size_t length) = 0;
                virtual int feedRead(std::unique_ptr<char[]> data, size_t length) = 0;
            };

            /**
             * per socket session.
             * @return
             */
            virtual std::shared_ptr<SocketContext> createContext(
                std::shared_ptr<Transport> transport,
                HandshakeCallback_t handshake_callback,
                WriteCallback_t write_callback,
                ReadCallback_t read_callback,
                CloseCallback_t close_callback,
                ErrorCallback_t error_callback
                ) = 0;
        };
    }
}


#endif //__JCU_TRANSPORT_SSL_ENGINE_H__
