/**
 * @file	transport.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/12/13
 * @copyright Copyright (C) 2019 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef __JCU_TRANSPORT_TRANSPORT_H__
#define __JCU_TRANSPORT_TRANSPORT_H__

#include <memory>
#include <functional>

#include <uvw/loop.hpp>

#include "error.h"

namespace jcu {
    namespace transport {
        class Transport {
        protected:
            std::shared_ptr<uvw::Loop> loop_;

        public:
            typedef std::function<void(Transport &transport)> OnConnectCallback_t;
            typedef std::function<void(Transport &transport, std::unique_ptr<char[]> data, size_t length)> OnDataCallback_t;
            typedef std::function<void(Transport &transport)> OnCloseCallback_t;
            typedef std::function<void(Transport &transport, Error &err)> OnErrorCallback_t;

            Transport(std::shared_ptr<uvw::Loop> loop) : loop_(loop) {}
            virtual ~Transport() {}

            virtual void connect(const OnConnectCallback_t& on_connect, const OnCloseCallback_t& on_close, const OnErrorCallback_t& on_error) = 0;
            virtual void reconnect() = 0;
            virtual void disconnect() = 0;
            virtual void cleanup() = 0; // remove callbacks (delete shared_ptr references)

            virtual void onData(const OnDataCallback_t& callback) = 0;
            virtual void write(std::unique_ptr<char[]> data, size_t length) = 0;
        };
    }
}


#endif // __JCU_TRANSPORT_TRANSPORT_H__
