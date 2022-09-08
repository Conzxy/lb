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
  
  enum TransferEncoding : unsigned char {
    NOT_SET = 0,
    CHUNKED,
  };
  
  enum ChunkState : unsigned char {
    LENGTH,
    DATA,
    LAST_DATA,
  };

  HttpResponseParser();
  
  ParseResult Parse(kanon::Buffer &buffer);
  
  bool IsChunked() const noexcept { return transfer_encoding_ == CHUNKED; } 
  size_t index() const noexcept { return index_; }
  
  void ResetIndex() noexcept { index_ = 0; }
 private:
  void ChunkReset() noexcept;
  void ContentLengthReset() noexcept;

  ParsePhase phase_;
  TransferEncoding transfer_encoding_ = NOT_SET;
  ChunkState chunk_state_ = LENGTH;
  
  size_t index_;

  size_t content_length_ = 0;
  size_t chunk_length_ = 0;
};
}

#endif // HTTP_RESPONSE_PARSER_H__
