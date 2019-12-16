/**
 * @file	tcp_transport.cpp
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/12/16
 * @copyright Copyright (C) 2019 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <jcu/transport/tcp_transport.h>

namespace jcu {
    namespace transport {

        class TcpTransportError : public Error {
        public:
            int code_;
            std::string name_;
            std::string what_;

            TcpTransportError(const uvw::ErrorEvent &evt) {
                const char *name = evt.name();
                const char *what = evt.what();
                if(name) name_ = name;
                if(what) what_ = what;
                code_ = evt.code();
            }

            const char *what() const override {
                return what_.c_str();
            }
            const char *name() const override {
                return name_.c_str();
            }
            int code() const override {
                return code_;
            }
            explicit operator bool() const override {
                return true;
            }
        };

        std::shared_ptr<TcpTransport> TcpTransport::create(std::shared_ptr<uvw::Loop> loop) {
            std::shared_ptr<TcpTransport> instance(new TcpTransport(loop));
            instance->self_ = instance;
            return instance;
        }

        TcpTransport::TcpTransport(std::shared_ptr<uvw::Loop> loop) : Transport(loop) {

        }

        TcpTransport::~TcpTransport() {

        }

        void TcpTransport::setRemote(const std::string& remote_ip, int remote_port) {
            remote_ip_ = remote_ip;
            remote_port_ = remote_port;
        }

        void TcpTransport::connect(
            const Transport::OnConnectCallback_t &on_connect,
            const Transport::OnCloseCallback_t &on_close,
            const OnErrorCallback_t& on_error
            ) {
            on_connect_ = on_connect;
            on_close_ = on_close;
            on_error_ = on_error;

            reconnect();
        }
        void TcpTransport::reconnect() {
            std::shared_ptr<TcpTransport> self = self_.lock();
            std::shared_ptr<uvw::TCPHandle> sock_handle = loop_->resource<uvw::TCPHandle>();
            sock_handle->data(self);
            sock_handle->once<uvw::ShutdownEvent>([this](uvw::ShutdownEvent &evt, uvw::TCPHandle &handle) -> void {
            });
            sock_handle->on<uvw::EndEvent>([this](uvw::EndEvent &evt, uvw::TCPHandle &handle) -> void {
                bool cancel = false;
                if(on_end_) {
                    cancel = on_end_(*this);
                }
                if(!cancel) {
                    handle.close();
                }
            });
            sock_handle->once<uvw::ConnectEvent>([this](uvw::ConnectEvent &evt, uvw::TCPHandle &handle) -> void {
              handle.read();
                if(on_connect_) {
                    on_connect_(*this);
                }
            });
            sock_handle->once<uvw::CloseEvent>([this](uvw::CloseEvent &evt, uvw::TCPHandle &handle) -> void {
                if(on_close_) {
                    on_close_(*this);
                }
            });
            sock_handle->on<uvw::ErrorEvent>([this](uvw::ErrorEvent &evt, uvw::TCPHandle &handle) -> void {
                TcpTransportError err(evt);
                if(on_error_) {
                    on_error_(*this, err);
                }
            });
            sock_handle->on<uvw::DataEvent>([this](uvw::DataEvent &evt, uvw::TCPHandle &handle) -> void {
              if(on_data_) {
                  on_data_(*this, std::move(evt.data), evt.length);
              }
            });
            sock_handle->on<uvw::WriteEvent>([this](uvw::WriteEvent &evt, uvw::TCPHandle &handle) -> void {
              // ...
            });
            sock_handle->connect(remote_ip_, remote_port_);
            sock_handle_ = sock_handle;
        }
        void TcpTransport::disconnect() {
            std::shared_ptr<uvw::TCPHandle> sock_handle = sock_handle_.lock();
            if(sock_handle) {
                sock_handle->shutdown();
                sock_handle->close();
            }
        }
        void TcpTransport::cleanup() {
            disconnect();
            on_connect_ = nullptr;
            on_close_ = nullptr;
        }
        void TcpTransport::onData(const OnDataCallback_t &on_data) {
            on_data_ = on_data;
        }
        void TcpTransport::onEnd(const OnEndCallback_t &on_end) {
            on_end_ = on_end;
        }
        void TcpTransport::write(std::unique_ptr<char[]> data, size_t length) {
            std::shared_ptr<uvw::TCPHandle> sock_handle = sock_handle_.lock();
            sock_handle->write(std::move(data), length);
        }
    }
}
