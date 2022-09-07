#ifndef HTTP_RESPONSE_PARSER_H__
#define HTTP_RESPONSE_PARSER_H__

#include "kanon/util/noncopyable.h"
#include "kanon/net/buffer.h"

namespace http {

class HttpResponseParser : kanon::noncopyable {
 public:
  enum ParseResult : unsigned char {
    PARSE_OK = 0,
    PARSE_SHORT, 
    PARSE_ERR,
  };
  
  enum ParsePhase : unsigned char {
    STATUS_LINE,
    HEADERS,
    BODY,
    FINISHED,
  };
  
  enum TRANSFER_ENCODING : unsigned char {
    NOT_SET = 0,
    CHUNKED,
  };

  HttpResponseParser();
  
  ParseResult Parse(kanon::Buffer &buffer);
  
  size_t index() const noexcept { return index_; }
  
  void ResetIndex() noexcept { index_ = 0; }
 private:
  ParsePhase phase_;
  size_t index_;
  
  TRANSFER_ENCODING transfer_encoding_ = NOT_SET;

  size_t content_length_ = 0;
};
}

#endif // HTTP_RESPONSE_PARSER_H__
