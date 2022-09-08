#include "http_response_parser.h"

#include "kanon/log/logger.h"
#include "kanon/string/string_view_util.h"

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
    case STATUS_LINE: {
      auto index = buffer_view.find("\r\n");
      /* Entire status line */
      if (index != StringView::npos) {
        auto status_line = buffer_view.substr(0, index);
        LOG_DEBUG << "status line: " << status_line;
        /* Extract the version message */
        auto space_pos = buffer_view.find(' ');

        if (space_pos == StringView::npos)
          return PARSE_ERR;

        LOG_DEBUG << "Http version: " << buffer_view.substr(0, space_pos);
        
        /* Status code */ 
        auto last_space_pos = space_pos; 
        space_pos = buffer_view.find(' ', space_pos+1);
        
        if (space_pos == StringView::npos)
          return PARSE_ERR;
        
        LOG_DEBUG << "Status code: " << buffer_view.substr_range(last_space_pos+1, space_pos);
        
        /* Reason phrase  */ 
        last_space_pos = space_pos;

        LOG_DEBUG << "Reason phrase: " << buffer_view.substr_range(last_space_pos+1, index);

        index_ = index + 2;
        LOG_TRACE << "HEADERS";
        phase_ = HEADERS;
      } else
        return PARSE_SHORT;
    } break;

    case HEADERS: {
      auto index = buffer_view.find("\r\n", index_);
      if (index != StringView::npos) {
        auto colon_pos = buffer_view.find(':', index_);
        auto field_view = buffer_view.substr_range(index_, colon_pos);
        auto value_view = buffer_view.substr_range(colon_pos + 2, index);
        if (field_view == "Content-Length") {
          LOG_DEBUG << "Content-Length view=" << value_view;
          auto length = StringViewToU32(value_view);
          if (!length) {
            return PARSE_ERR;
          }
          content_length_ = *length;
          LOG_DEBUG << "Content-Length=" << content_length_;
        } else if (field_view == "Transfer-Encoding") {
          if (value_view == "chunked") {
            transfer_encoding_ = CHUNKED;
          }
        }

        assert(buffer_view[index] == '\r' && buffer_view[index + 1] == '\n');
        index_ = index + 2;
        if (index_ >= buffer_view.size() || index_ + 1 >= buffer_view.size()) {
          return PARSE_SHORT;
        } else if (buffer_view[index_] == '\r' &&
                   buffer_view[index_ + 1] == '\n') {
          index_ += 2;
          LOG_TRACE << "BODY";
          phase_ = BODY;
        }
      } else
        return PARSE_SHORT;
    } break;

    case BODY: {
      if (content_length_ > 0) {
        if (buffer.GetReadableSize() - index_ >= content_length_) {
          index_ += content_length_;
          ContentLengthReset();
          /* reset */
          LOG_TRACE << "Parse complete";
          return PARSE_OK;
        } else return PARSE_SHORT;
      } else if (transfer_encoding_ == CHUNKED) {
        auto index = buffer_view.find("\r\n", index_);
        
        if (index != StringView::npos) {
          switch (chunk_state_) {
          case LENGTH: {
            auto length_sv = buffer_view.substr_range(index_, index);
            LOG_DEBUG << "chunked length bytes = " << index - index_;
            LOG_DEBUG << "chunked length: " << length_sv;
            auto length = StringViewToU32(length_sv, 16);
            if (!length) {
              ChunkReset();
              return PARSE_ERR;
            }

            chunk_length_ = *length;
            LOG_DEBUG << "Chunk length = " << chunk_length_;
            index_ = index + 2;
            if (chunk_length_ == 0)
              chunk_state_ = LAST_DATA;
            else
              chunk_state_ = DATA;
          } break;

          case DATA: {
            /* Chunked data */
            LOG_DEBUG << "Cached chunked data length=" << index - index_;
            if (chunk_length_ == index - index_) {
              index_ = index + 2;
              chunk_length_ = 0;
              chunk_state_ = LENGTH;
              return PARSE_OK;
            } else {
              return PARSE_ERR;
            }
          } break;

          case LAST_DATA: {
            index_ = index + 2;
            ChunkReset();
            return PARSE_OK;
          } break;
          }
        } else {
          return PARSE_SHORT;
        }
      } else {
        /* No body */
        phase_ = STATUS_LINE;
        return PARSE_OK;
      }
    } break;
    }

  }

  assert(false);
  return PARSE_ERR;
}

void HttpResponseParser::ChunkReset() noexcept
{
  chunk_length_ = 0;
  phase_ = STATUS_LINE;
  chunk_state_ = LENGTH;
  transfer_encoding_ = NOT_SET;
}

void HttpResponseParser::ContentLengthReset() noexcept
{
  content_length_ = 0;
  phase_ = STATUS_LINE;
}
