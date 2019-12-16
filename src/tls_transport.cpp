/**
 * @file	tls_transport.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/12/16
 * @copyright Copyright (C) 2019 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <jcu/transport/tls_transport.h>

namespace jcu {
    namespace transport {

        std::shared_ptr<TlsTransport> TlsTransport::create(std::shared_ptr<uvw::Loop> loop, std::shared_ptr<Transport> transport, std::shared_ptr<SslEngine> engine) {
            std::shared_ptr<TlsTransport> instance(new TlsTransport(loop));
            instance->self_ = instance;
            instance->transport_ = transport;
            instance->engine_ = engine;
            return instance;
        }

        TlsTransport::TlsTransport(std::shared_ptr<uvw::Loop> loop) : Transport(loop) {

        }

        TlsTransport::~TlsTransport() {

        }

        void TlsTransport::connect(const Transport::OnConnectCallback_t &on_connect,
                                   const Transport::OnCloseCallback_t &on_close,
                                   const Transport::OnErrorCallback_t &on_error) {
            on_connect_ = on_connect;
            on_close_ = on_close;
            on_error_ = on_error;

            transport_->onEnd([this](Transport& transport) -> void {
                if(on_end_) {
                    on_end_(*this);
                }
            });
            transport_->onData([this](Transport& transport, std::unique_ptr<char[]> data, size_t length) -> void {
                if(ssl_socket_) {
                    ssl_socket_->feedRead(std::move(data), length);
                }
            });
            transport_->connect([this](Transport& transport) -> void {
              std::shared_ptr<TlsTransport> self = self_.lock();
              ssl_socket_ = engine_->createContext(
                  transport_,
                  [this](SslEngine::SocketContext *socket_context, int status) -> void {
                    // Handshake
                    if(on_connect_) {
                        on_connect_(*this);
                    }
                  },
                  [this](SslEngine::SocketContext *socket_context, int status) -> void {
                    // Write

                  },
                  [this](SslEngine::SocketContext *socket_context, const char *buf, int size) -> void {
                    // Read
					  if (on_data_) {
						  std::unique_ptr<char[]> ubuf(new char[size]);
						  memcpy(ubuf.get(), buf, size);
						  on_data_(*this, std::move(ubuf), size);
					  }
                  },
                  [this](SslEngine::SocketContext *socket_context, int status) -> void {
                    // Close
                    transport_->disconnect();
                    ssl_socket_ = nullptr;
                  },
                  [this](SslEngine::SocketContext *socket_context, Error &err) -> void {
                      if(on_error_) {
                          on_error_(*this, err);
                      }
                  }
              );
              ssl_socket_->handshake();
            }, [this](Transport& transport) -> void {
                if(on_close_) {
                    on_close_(*this);
                }
                ssl_socket_ = nullptr;
            },
            on_error_);
        }
        void TlsTransport::reconnect() {
            transport_->reconnect();
        }
        void TlsTransport::disconnect() {
            ssl_socket_->disconnect();
        }
        void TlsTransport::cleanup() {
            on_connect_ = nullptr;
            on_close_ = nullptr;
        }
        void TlsTransport::onData(const OnDataCallback_t &on_data) {
            this->on_data_ = on_data;
        }
        void TlsTransport::onEnd(const OnEndCallback_t &on_end) {
            on_end_ = on_end;
        }
        void TlsTransport::write(std::unique_ptr<char[]> data, size_t length) {
            this->ssl_socket_->write(std::move(data), length);
        }
    }
}
