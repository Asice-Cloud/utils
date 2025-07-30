#include <boost/asio.hpp>
#include <boost/bind.hpp>


class session
{
public:
	session(boost::asio::io_service is) : socket_(is) {}

	void start()
	{
		socket_.async_read_some(boost::asio::buffer(data_, max_length),
								boost::bind(&session::handle_read, this, boost::asio::placeholders::error,
											boost::asio::placeholders::bytes_transferred));
	}

private:
	void handle_read(const boost::system::error_code &e, size_t bytes_transferred)
	{
		if (!e)
		{
			boost::asio::async_write(socket_, boost::asio::buffer(data_, bytes_transferred),
									 boost::bind(&session::handle_write, this, boost::asio::placeholders::error));
		}
		else
		{
		}
	}

	void handle_write(const boost::system::error_code &e)
	{
		if (!e)
		{
			socket_.async_read_some(boost::asio::buffer(data_, max_length),
									boost::bind(&session::handle_read, this, boost::asio::placeholders::error,
												boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			delete this;
		}
	}
	boost::asio::ip::tcp::socket socket_;
	enum
	{
		max_length = 1024
	};
	char data_[max_length];
};
