#pragma once

#include <gnuradio/network/udp_source.hh>
#include <asio.hpp>
#include <deque>
#include <vector>

#include <gnuradio/buffer_cpu_vmcirc.hh>

namespace gr {
namespace network {

class udp_source_cpu : public udp_source
{
public:
    udp_source_cpu(const typename udp_source::block_args& args);

    virtual work_return_code_t
    work(std::vector<block_work_input_sptr>& work_input,
         std::vector<block_work_output_sptr>& work_output) override;

    bool start() override
    {
        std::cout << "starting udp socket" << std::endl;
        d_socket = std::make_unique<asio::ip::udp::socket>(io_context, d_endpoint);

        do_receive();
        io_context.run();
        return block::start();
    }
    bool stop() override
    {
        GR_LOG_DEBUG(_debug_logger, "STOP called");
        io_context.stop();
        // io_context.restart();
        return block::stop();
    }

    void do_receive()
    {
        // get the buffer and the length from the circular buffer
        buffer_info_t wi;
        d_circbuf->write_info(wi);
        if (wi.n_items < (int)d_pktlen) {
            GR_LOG_DEBUG(_debug_logger, "Circular Buffer is full");
            asio::deadline_timer t(io_context);
            t.expires_from_now(boost::posix_time::milliseconds(100));
            t.async_wait([this](std::error_code ec) { do_receive(); });
            return;
        }

        d_socket->async_receive_from(
            asio::buffer(wi.ptr, wi.n_items),
            d_endpoint,
            [this](std::error_code ec, std::size_t bytes_recvd) {
                // Queue up the received data
                GR_LOG_DEBUG(_debug_logger,
                             "Packet Received with {} bytes and error code {}",
                             bytes_recvd,
                             ec.message());

                d_circbuf->post_write(bytes_recvd);

                // kick the scheduler
                notify_scheduler_output();

                do_receive();
            });
    }

private:
    static const size_t d_max_length = 9000;
    std::unique_ptr<asio::ip::udp::socket> d_socket;
    char d_data[d_max_length];

    asio::io_context io_context;
    asio::ip::udp::endpoint d_endpoint;

    std::deque<std::vector<uint8_t>> d_deque;

    gr::buffer_sptr d_circbuf;
    gr::buffer_reader_sptr d_circbuf_rdr;


    size_t d_pktlen;
};


} // namespace network
} // namespace gr
