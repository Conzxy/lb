#include "http_response_parser.h"

#include "kanon/log/logger.h"

using namespace kanon;
using namespace http;

HttpResponseParser::HttpResponseParser()
  : phase_(STATUS_LINE)
  , index_(0)
{

}

auto HttpResponseParser::Parse(kanon::Buffer &buffer) -> ParseResult
{
  auto buffer_view = buffer.ToStringView();
  for (;;) {
    switch (phase_) {
      case STATUS_LINE:
      {
        auto index = buffer_view.find("\r\n");
        if (index != StringView::npos) {
          index_ = index + 2;
          LOG_TRACE << "HEADERS";
          phase_ = HEADERS;
        } else
          return PARSE_SHORT;
      }
      break;
      
      case HEADERS:
      {
        auto index = buffer_view.find("\r\n", index_);
        if (index != StringView::npos) {
          auto colon_pos = buffer_view.find(':', index_);
          auto field_view = buffer_view.substr_range(index_, colon_pos);
          auto value_view = buffer_view.substr_range(colon_pos+2, index);
          if (field_view == "Content-Length") {
            char digit_buf[128];
            memcpy(digit_buf, value_view.data(), value_view.size());
            char *end = NULL;
            content_length_ = strtoull(digit_buf, &end, 10);
            if (content_length_ == 0 && end == digit_buf) {
              return PARSE_ERR;
            }
            LOG_DEBUG << "Content-Length=" << content_length_;
          } else if (field_view == "Transfer-Encoding") {
            if (value_view == "chunked") {
              transfer_encoding_ = CHUNKED;
            }
          }
          
          assert(buffer_view[index] == '\r' && buffer_view[index+1] == '\n');
          index_ = index + 2;
          if (index_ >= buffer_view.size() ||
              index_+1 >= buffer_view.size()) 
          {
            return PARSE_SHORT;
          } 
          else if (buffer_view[index_] == '\r' && buffer_view[index_+1] == '\n') {
            index_ += 2;
            LOG_TRACE << "BODY";
            phase_ = BODY;
          }
        } else 
          return PARSE_SHORT;
      }
      break;

      case BODY:
      {
        if (content_length_ > 0 && 
            buffer.GetReadableSize() - index_ >= content_length_) 
        {
          index_ += content_length_;
          /* reset */
          content_length_ = 0;
          phase_ = STATUS_LINE;
          LOG_TRACE << "Parse complete";
          return PARSE_OK;
        } else if (transfer_encoding_ == CHUNKED) {

        } else {
          return PARSE_SHORT;
        }
      } 
      break;
    }
  }
  
  assert(false);
  return PARSE_ERR;
}
