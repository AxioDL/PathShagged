#pragma once

#include <memory>

namespace hecl::blender {
class Connection;

class Token {
  std::unique_ptr<Connection> m_conn;

public:
  Connection& getBlenderConnection();
  void shutdown();

  Token() = default;
  ~Token();
  Token(const Token&) = delete;
  Token& operator=(const Token&) = delete;
  Token(Token&&) = default;
  Token& operator=(Token&&) = default;
};

} // namespace hecl::blender
