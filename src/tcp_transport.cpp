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
            const Transport::OnCloseCallback_t &on_close
            ) {
            on_connect_ = on_connect;
            on_close_ = on_close;

            reconnect();
        }
        void TcpTransport::reconnect() {
            std::shared_ptr<TcpTransport> self = self_.lock();
            sock_handle_ = loop_->resource<uvw::TCPHandle>();
            sock_handle_->data(self);
            sock_handle_->once<uvw::ShutdownEvent>([this](uvw::ShutdownEvent &evt, uvw::TCPHandle &handle) -> void {
            });
            sock_handle_->once<uvw::EndEvent>([this](uvw::EndEvent &evt, uvw::TCPHandle &handle) -> void {
            });
            sock_handle_->once<uvw::ConnectEvent>([this](uvw::ConnectEvent &evt, uvw::TCPHandle &handle) -> void {
              handle.read();
                if(on_connect_) {
                    on_connect_(this);
                }
            });
            sock_handle_->once<uvw::CloseEvent>([this](uvw::CloseEvent &evt, uvw::TCPHandle &handle) -> void {
                if(on_close_) {
                    on_close_(this);
                }
            });
            sock_handle_->on<uvw::ErrorEvent>([this](uvw::ErrorEvent &evt, uvw::TCPHandle &handle) -> void {
            });
            sock_handle_->on<uvw::DataEvent>([this](uvw::DataEvent &evt, uvw::TCPHandle &handle) -> void {
              if(on_data_) {
                  on_data_(this, std::move(evt.data), evt.length);
              }
            });
            sock_handle_->on<uvw::WriteEvent>([this](uvw::WriteEvent &evt, uvw::TCPHandle &handle) -> void {
              // ...
            });
            sock_handle_->connect(remote_ip_, remote_port_);
        }
        void TcpTransport::disconnect() {
            sock_handle_->shutdown();
            sock_handle_->close();
        }
        void TcpTransport::cleanup() {
            on_connect_ = nullptr;
            on_close_ = nullptr;
        }
        void TcpTransport::onData(const OnDataCallback_t &on_data) {
            this->on_data_ = on_data;
        }
        void TcpTransport::write(std::unique_ptr<char[]> data, size_t length) {
            this->sock_handle_->write(std::move(data), length);
        }
    }
}
