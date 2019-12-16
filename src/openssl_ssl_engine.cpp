/**
 * @file	openssl_ssl_engine.cpp
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/12/16
 * @copyright Copyright (C) 2019 jc-lab.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#include <jcu/transport/openssl_ssl_engine.h>

namespace jcu {
    namespace transport {

        std::shared_ptr<OpensslSslEngine> OpensslSslEngine::create(const SSL_METHOD *meth) {
            std::shared_ptr<OpensslSslEngine> instance(new OpensslSslEngine());
            instance->ssl_ctx_ = SSL_CTX_new(meth);
            return instance;
        }

        std::shared_ptr<SslEngine::SocketContext> OpensslSslEngine::createContext(
            std::shared_ptr<Transport> transport,
            HandshakeCallback_t handshake_callback,
            WriteCallback_t write_callback,
            ReadCallback_t read_callback,
            CloseCallback_t close_callback
            ) {
            int r;
            std::shared_ptr<OpensslSocketContext> ctx(new OpensslSocketContext());

            ctx->self_ = ctx;
            ctx->transport_ = transport;
            ctx->handshake_callback_ = handshake_callback;
            ctx->write_callback_ = write_callback;
            ctx->read_callback_ = read_callback;
            ctx->close_callback_ = close_callback;

            ctx->ssl_ = SSL_new(this->ssl_ctx_);

            r = BIO_new_bio_pair(&ctx->ssl_bio_, 0, &ctx->app_bio_, 0);
            if(r != 1) {
                return nullptr;
            }

            SSL_set_bio(ctx->ssl_, ctx->ssl_bio_, ctx->ssl_bio_);

            return ctx;
        }

        OpensslSslEngine::OpensslSocketContext::OpensslSocketContext() {
            ssl_ = NULL;
        }

        OpensslSslEngine::OpensslSocketContext::~OpensslSocketContext() {
            if(ssl_) {
                SSL_free(ssl_);
                ssl_ = NULL;
            }
        }

        void OpensslSslEngine::OpensslSocketContext::handshake() {
            SSL_set_connect_state(ssl_);
            tlsOperation(OP_HANDSHAKE, NULL, 0);
        }

        void OpensslSslEngine::OpensslSocketContext::disconnect() {
            tlsOperation(OP_SHUTDOWN, NULL, 0);
        }

        void OpensslSslEngine::OpensslSocketContext::write(std::unique_ptr<char[]> data, size_t length) {
            tlsOperation(OP_WRITE, data.get(), length);
        }

        int OpensslSslEngine::OpensslSocketContext::feedRead(std::unique_ptr<char[]> data, size_t length) {
            int offset = 0;
            int rv = 0;
            int i  = 0;

            char *data_ptr = data.get();

            // assert( data != NULL && "invalid argument passed");
            // assert( sz > 0 && "Size of data should be positive");
            for( offset = 0; offset < length; offset += i ) {
                //handle error condition
                i =  BIO_write(app_bio_, data_ptr + offset, length - offset);

                //if handshake is not complete, do it again
                if ( SSL_is_init_finished(ssl_) ) {
                    rv = tlsOperation(OP_READ, NULL, 0);
                } else {
                    rv = tlsOperation(OP_HANDSHAKE, NULL, 0);
                }
            }
            return rv;
        }
        
        int OpensslSslEngine::OpensslSocketContext::tlsOperation(OpType op, void *buf, int sz)
        {
            std::shared_ptr<OpensslSocketContext> self = self_.lock();

            int r = 0;
            int bytes = 0;
            char tbuf[16*1024] = {0};

            switch ( op ) {
                case OP_HANDSHAKE: {
                    r = SSL_do_handshake(ssl_);
                    bytes = sendPending();
                    if(bytes < 0) {
                        // ERROR
                        return -1;
                    }
                    if (1 == r || 0 == r) {
                        if(handshake_callback_) {
                            handshake_callback_(this, r);
                        }
                    }
                    break;
                }

                case OP_READ: {
                    r = SSL_read(ssl_, tbuf, sizeof(tbuf));
                    if ( r == 0 ) goto handle_shutdown;

                    if ( r > 0 ) {
                        if ( read_callback_ ) {
                            read_callback_(this, tbuf, r);
                        }
                    }
                    else {
                        //write pending data, if nothing is pending, we assume
                        //that SSL_read failed and triger the read_cb
                        bytes = sendPending();
                        if ( bytes == 0 && read_callback_) {
                            read_callback_(this, tbuf, r);
                        }
                    }
                    break;
                }

                case OP_WRITE: {
                    // assert( sz > 0 && "number of bytes to write should be positive");
                    r = SSL_write(ssl_, buf, sz);
                    if ( 0 == r) goto handle_shutdown;
                    bytes = sendPending();
                    if ( r > 0 && write_callback_) {
                        write_callback_(this, r);
                    }
                    break;
                }

                    /* we initiate shutdown process, send the close_notify but we are not
                     * sure if peer will sent their close_notify hence fire the callback
                     * if the peer replied, it will be processed in SSL_read returning 0
                     * and jump to handle_shutdown
                     *
                     * No check for SSL_shutdown done as it may be possible that user
                     * called close upon receive of EOF.
                     * TODO: Find a elegant way later
                     * */

                case OP_SHUTDOWN: {
                    r = SSL_shutdown(ssl_);
                    bytes = sendPending();
                    if ( close_callback_ ) {
                        close_callback_(this, r);
                    }
                    break;
                }

                default:
                    // assert( 0 && "Unsupported operation");
                    return -1;
                    break;
            }
            return r;

            handle_shutdown:
            r = SSL_shutdown(ssl_);
            //it might be possible that peer send close_notify and close the network
            //hence, no check if sending is complete
            bytes = sendPending();
            if ( (1 == r)  && close_callback_ ) {
                close_callback_(this, r);
            }
            return r;
        }


        int OpensslSslEngine::OpensslSocketContext::sendPending() {
            std::shared_ptr<Transport> transport = transport_.lock();

            // assert( conn != NULL);
            int pending = BIO_pending(app_bio_);
            if ( !(pending > 0) )
                return 0;

            std::unique_ptr<char[]> buf(new char[pending]);

            int p = BIO_read(app_bio_, buf.get(), pending);
            // assert(p == pending);

            // assert( conn->writer != NULL && "You need to set network writer first");
            transport->write(std::move(buf), pending);
            
            return p;
        }

    }
}
