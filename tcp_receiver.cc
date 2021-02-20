#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    size_t length = seg.length_in_sequence_space();
    static size_t abs_seq = 0;
    WrappingInt32 seqno(seg.header().seqno);
    if(seg.header().syn)
    {
        if(syn_flag)
            return;
        syn_flag = true;
        isn = seg.header().seqno.raw_value();
        abs_seq = 1;
        length--;
    }
    else if(!syn_flag)
        return;
    else
    {
        size_t checkpoint = _reassembler.stream_out().bytes_written();
        abs_seq = unwrap(WrappingInt32(seg.header().seqno.raw_value()),WrappingInt32(isn),checkpoint);
    }

    if(seg.header().fin)
    {
        if(fin_flag)
            return;
        fin_flag = true;
        length--;
    }
    _reassembler.push_substring(seg.payload().copy(),abs_seq-1,seg.header().fin);
    return;
}

optional<WrappingInt32> TCPReceiver::ackno() const 
{
    if(!syn_flag)
        return {};
    if(_reassembler.stream_out().input_ended())
        return WrappingInt32(wrap(_reassembler.stream_out().bytes_written()+1,WrappingInt32(isn)))+1;
    return WrappingInt32(wrap(_reassembler.stream_out().bytes_written()+1,WrappingInt32(isn)));
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
