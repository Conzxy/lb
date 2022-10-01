#include "http_request_codec.h"
#include "lb/util/string_util.h"

using namespace lb;
using namespace http;
using namespace kanon;

HttpRequestCodec::HttpRequestCodec(TcpConnectionPtr const &conn)
{
  conn->SetMessageCallback(
      [this](TcpConnectionPtr const &conn, Buffer &buffer, TimeStamp ts) {
        while (1) {
          switch (parser_.Parse(buffer, &request_)) {
          case HttpParser::kGood: {
            request_callback_(conn, request_, ts);
          } break;
          case HttpParser::kError: {
            LOG_ERROR << "Parse request error";
            LOG_ERROR << "Error message: " << parser_.error().msg;
            return;
          }
          case HttpParser::kShort:
            return;
          }
        }
      });
}

void HttpRequestCodec::Send(TcpConnectionPtr const &conn,
                            HttpRequest const &request)
{
  OutputBuffer output;
  
  auto transfer_encoding_field = request.headers.find("Transfer-Encoding");
  if (transfer_encoding_field == request.headers.end() ||
      strcasecmp(transfer_encoding_field->second.c_str(), "chunked") == 0) 
  {
    // Append header line
    std::string header_line =
        util::StrCat(GetMethodString(request.method), " ", request.url, " ",
                     GetHttpVersionString(request.version));
    LOG_DEBUG << "header line = " << header_line;
    output.Append(header_line);
    output.Append("\r\n");

    // Append headers
    std::string header;
    for (auto &[field, value] : request.headers) {
      util::StrAppend(header, field, ": ", value);
      LOG_DEBUG << "header field: " << header;
      output.Append(header);
      output.Append("\r\n");
      /* Reset to reuse space */
      header.resize(0);
    }

    output.Append("\r\n");
  }
  
  LOG_DEBUG << "(After headers)Readable size = " << output.GetReadableSize(); 
  LOG_DEBUG << "Body length = " << request.body.size();
  output.Append(request.body);

  LOG_DEBUG << "Readable size = " << output.GetReadableSize();
  conn->Send(output);
}
