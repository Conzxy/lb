#include "http_request_codec.h"
#include "lb/util/string_util.h"

using namespace lb;
using namespace http;
using namespace kanon;

HttpRequestCodec::HttpRequestCodec(TcpConnectionPtr const &conn)
{
  conn->SetMessageCallback(
      [this](TcpConnectionPtr const &conn, Buffer &buffer, TimeStamp ts) {
        if (parser_.Parse(buffer, &request_) == HttpParser::kGood) {
          request_callback_(conn, request_, ts);
        }
      });
}

void HttpRequestCodec::Send(TcpConnectionPtr const &conn,
                            HttpRequest const &request)
{
  OutputBuffer output;

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

  output.Append(request.body);
  
  LOG_DEBUG << "Readable size = " << output.GetReadableSize();
  conn->Send(output);
}
