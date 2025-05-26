//
// Created by asice-cloud on 5/26/25.
//

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <asio.hpp>
#include <asio/placeholders.hpp>

using asio::ip::tcp;

std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

class tcp_connection
  : public std::enable_shared_from_this<tcp_connection>
{
public:
  typedef std::shared_ptr<tcp_connection> pointer;

  static pointer create(asio::io_context& io_context)
  {
    return pointer(new tcp_connection(io_context));
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  std::string message_;

  void start()
  {
     message_ = make_daytime_string();

    asio::async_write(socket_, asio::buffer(message_),
        std::bind(&tcp_connection::handle_write, shared_from_this(),
         std::placeholders::_1, std::placeholders::_2));
  }

private:
  tcp_connection(asio::io_context& io_context)
    : socket_(io_context)
  {
  }

  void handle_write(const std::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
      std::cout << "Sent: " << message_ << " (" << bytes_transferred << " bytes)" << std::endl;
    }
    else
    {
      std::cerr << "Error on write: " << error.message() << std::endl;
    }
  }

  tcp::socket socket_;
};

class tcp_server
{
public:
  tcp_server(asio::io_context& io_context)
    : io_context_(io_context),
      acceptor_(io_context, tcp::endpoint(tcp::v4(), 8080))
  {
    start_accept();
  }

private:
  void start_accept()
  {
    tcp_connection::pointer new_connection =
      tcp_connection::create(io_context_);

    acceptor_.async_accept(new_connection->socket(),
        [this, new_connection](const std::error_code& error) {
            handle_accept(new_connection, error);
        });
  }

  void handle_accept(tcp_connection::pointer new_connection,
      const std::error_code& error)
  {
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }

  asio::io_context& io_context_;
  tcp::acceptor acceptor_;
};

#endif // TCP_SERVER_H