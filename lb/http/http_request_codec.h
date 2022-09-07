#ifndef LB_HTTP_REQUEST_CODEC_H__
#define LB_HTTP_REQUEST_CODEC_H__

#include "kanon/util/noncopyable.h"
#include "http_parser.h"
#include "http_request.h"
#include "kanon/net/tcp_connection.h"


namespace lb {

class HttpRequestCodec : kanon::noncopyable {
  using RequestCallback = std::function<void(kanon::TcpConnectionPtr const &, http::HttpRequest &, kanon::TimeStamp)>;
 public:
  HttpRequestCodec(kanon::TcpConnectionPtr const &conn); 

  void SetRequestCallback(RequestCallback cb) { request_callback_ = std::move(cb); }

  void Send(kanon::TcpConnectionPtr const &conn, http::HttpRequest const &req);
 private:
   http::HttpParser parser_;
   http::HttpRequest request_; 
   RequestCallback request_callback_;


};

}
#endif // <LB_HTTP_CODEC_H__
