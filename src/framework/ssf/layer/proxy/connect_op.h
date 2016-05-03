#ifndef SSF_LAYER_PROXY_CONNECT_OP_H_
#define SSF_LAYER_PROXY_CONNECT_OP_H_

#include <boost/system/error_code.hpp>

#include "ssf/error/error.h"
#include "ssf/layer/connect_op.h"
#include "ssf/layer/proxy/http_connect_op.h"

namespace ssf {
namespace layer {
namespace proxy {
namespace detail {

template <class Stream, class Endpoint>
class ConnectOp {
 public:
  ConnectOp(Stream& stream, Endpoint* p_local_endpoint, Endpoint peer_endpoint)
      : stream_(stream),
        p_local_endpoint_(p_local_endpoint),
        peer_endpoint_(std::move(peer_endpoint)) {}

  void operator()(boost::system::error_code& ec) {
    auto& context = peer_endpoint_.endpoint_context();

    if (!context.IsProxyEnabled()) {
      ssf::layer::detail::ConnectOp<Stream, Endpoint>(
          stream_, p_local_endpoint_, std::move(peer_endpoint_))(ec);
      return;
    }

    if (context.HttpProxyEnabled()) {
      detail::HttpConnectOp<Stream, Endpoint>(stream_, p_local_endpoint_,
                                              std::move(peer_endpoint_))(ec);
      return;
    }
  }

 private:
  Stream& stream_;
  Endpoint* p_local_endpoint_;
  Endpoint peer_endpoint_;
};

template <class Protocol, class Stream, class Endpoint, class ConnectHandler>
class AsyncConnectOp {
 public:
  AsyncConnectOp(Stream& stream, Endpoint* p_local_endpoint,
                 Endpoint peer_endpoint, ConnectHandler handler)
      : stream_(stream),
        p_local_endpoint_(p_local_endpoint),
        peer_endpoint_(std::move(peer_endpoint)),
        handler_(std::move(handler)) {}

  void operator()() {
    auto& context = peer_endpoint_.endpoint_context();

    if (!context.IsProxyEnabled()) {
      ssf::layer::detail::AsyncConnectOp<
          Protocol, Stream, Endpoint,
          typename boost::asio::handler_type<
              ConnectHandler, void(boost::system::error_code)>::type>(
          stream_, p_local_endpoint_, std::move(peer_endpoint_),
          std::move(handler_))();
      return;
    }

    if (context.HttpProxyEnabled()) {
      detail::AsyncHttpConnectOp<
          Protocol, Stream, Endpoint,
          typename boost::asio::handler_type<
              ConnectHandler, void(boost::system::error_code)>::type>(
          stream_, p_local_endpoint_, std::move(peer_endpoint_),
          std::move(handler_))();
      return;
    }
  }

 private:
  Stream& stream_;
  Endpoint* p_local_endpoint_;
  Endpoint peer_endpoint_;
  ConnectHandler handler_;
};

}  // detail
}  // proxy
}  // layer
}  // ssf

#endif  // SSF_LAYER_PROXY_CONNECT_OP_H_