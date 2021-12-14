/* -*- c++ -*- */
/*
 * Copyright 2004,2009,2010,2012,2018 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "udp_source_cpu.hh"
#include "udp_source_cpu_gen.hh"
#include <volk/volk.h>

namespace gr {
namespace network {

udp_source_cpu::udp_source_cpu(const typename udp_source::block_args& args)
    : sync_block("udp_source"),
      udp_source(args),
      d_endpoint(asio::ip::udp::endpoint(asio::ip::udp::v4(), args.port)),
      d_pktlen(args.pktlen)
{
    auto bufprops = std::make_shared<buffer_cpu_vmcirc_properties>();
    d_circbuf = gr::buffer_cpu_vmcirc::make(9000 * 400, 1, bufprops);
    d_circbuf_rdr = d_circbuf->add_reader(bufprops, 1);
}

work_return_code_t udp_source_cpu::work(std::vector<block_work_input_sptr>& work_input,
                                        std::vector<block_work_output_sptr>& work_output)
{
    auto out = work_output[0]->items<void>();
    auto itemsize = work_output[0]->buffer->item_size();

    // Is there data in the circular buffer
    buffer_info_t ri;
    d_circbuf_rdr->read_info(ri);
    // write what we got
    size_t items_on_output = work_output[0]->n_items;
    size_t items_on_circbuf = ri.n_items / itemsize;
    size_t items_to_write = std::min(items_on_circbuf, items_on_output);
    size_t n = items_to_write * itemsize;

    if (n > 0) {
        memcpy(out, ri.ptr, n);
        d_circbuf_rdr->post_read(n);
    }

    gr_log_debug(_debug_logger, "producing: {} - output is {} , circbuf is {}", n, items_on_output, items_on_circbuf );
    gr_log_debug(_debug_logger, "total_written: {}", work_output[0]->nitems_written() + n );
    produce_each(n, work_output);
    return work_return_code_t::WORK_OK;
}

} /* namespace network */
} /* namespace gr */
