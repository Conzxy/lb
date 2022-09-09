#include "lb/http/http_response_parser.h"

#include "lb/http/http_response_buffer.h"

#include <iostream>
#include <gtest/gtest.h>

using namespace http;
using namespace kanon;

TEST(http_response_parser, single_response) {
  HttpResponseParser parser;

  HttpResponseBuffer response;
  response.AddHeaderLine(http::HttpStatusCode::k200OK, HttpVersion::kHttp10); 
  response.AddHeader("xx", "xxx");
  response.AddBlackLine();
  response.AddBody("sadfjkljalkj");
  
  auto &buffer = response.GetBuffer();
  std::cout << buffer.ToStringView().ToString();

  EXPECT_EQ(HttpResponseParser::PARSE_OK, parser.Parse(buffer));
  auto index = parser.index();

  EXPECT_EQ(index, buffer.GetReadableSize()); 
}

TEST(http_response_parser, multiple_response) {
  HttpResponseParser parser;
  HttpResponseBuffer response;
  response.AddHeaderLine(http::HttpStatusCode::k200OK, HttpVersion::kHttp10); 
  response.AddHeader("xx", "xxx");
  response.AddBlackLine();
  response.AddBody("sadfjkljalkj");
  
  auto &buffer = response.GetBuffer(); 
  auto first_index = buffer.GetReadableSize();
  HttpResponseBuffer response2;
  response2.AddHeaderLine(http::HttpStatusCode::k200OK, HttpVersion::kHttp10); 
  response2.AddHeader("a", "b");
  response2.AddBlackLine();
  response2.AddBody("sadfjkljalkjshgaghea");
  buffer.Append(response2.GetBuffer().ToStringView()); 
  auto second_index = buffer.GetReadableSize() - first_index;

  ASSERT_EQ(HttpResponseParser::PARSE_OK, parser.Parse(buffer));
  auto index = parser.index();
  parser.ResetIndex();
  EXPECT_EQ(first_index, index);
  
  buffer.AdvanceRead(index);
  ASSERT_EQ(HttpResponseParser::PARSE_OK, parser.Parse(buffer));
  index = parser.index();
  EXPECT_EQ(second_index, index);
}

TEST(http_response_parser, chunked_response) {
  HttpResponseParser parser;
  HttpResponseBuffer response;
  response.AddHeaderLine(http::HttpStatusCode::k200OK, HttpVersion::kHttp10);
  response.AddHeader("xxx", "xx");
  response.AddChunkedTransferHeader();
  response.AddBlackLine();
  response.AddChunk("XXXXX");
  
  auto buffer = response.GetBuffer();

  std::cout << buffer.ToStringView().ToString();
  auto index = buffer.GetReadableSize();
  
  EXPECT_EQ(HttpResponseParser::PARSE_OK, parser.Parse(buffer));
  EXPECT_EQ(parser.index(), index);
  EXPECT_TRUE(parser.IsChunked());
  parser.ResetIndex();

  HttpResponseBuffer last_response;
  last_response.AddChunk("", 0);
  buffer = std::move(last_response.GetBuffer());
  std::cout << buffer.ToStringView().ToString();
  index = buffer.GetReadableSize();

  EXPECT_EQ(HttpResponseParser::PARSE_OK, parser.Parse(buffer));
  EXPECT_EQ(parser.index(), index);
  EXPECT_FALSE(parser.IsChunked());
  parser.ResetIndex();

}
