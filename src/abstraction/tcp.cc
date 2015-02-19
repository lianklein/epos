// EPOS Transmission Control Protocol (RFC 793) Implementation

#include <tcp.h>

__BEGIN_SYS

// Class attributes
TCP::Observed TCP::_observed;

TCP::Connection::State_Handler TCP::Connection::_handlers[] = {
    &TCP::Connection::listening,
    &TCP::Connection::syn_sent,
    &TCP::Connection::syn_received,
    &TCP::Connection::established,
    &TCP::Connection::fin_wait1,
    &TCP::Connection::fin_wait2,
    &TCP::Connection::close_wait,
    &TCP::Connection::closing,
    &TCP::Connection::last_ack,
    &TCP::Connection::time_wait,
    &TCP::Connection::closed
};


// Methods
void TCP::update(IP::Observed * obs, IP::Protocol prot, NIC::Buffer * pool)
{
    db<TCP>(TRC) << "TCP::update(obs=" << obs << ",prot=" << prot << ",buf=" << pool << ")" << endl;
    db<TCP>(INF) << "TCP::update:buf=" << pool << " => " << *pool << endl;

    Packet * packet = pool->frame()->data<Packet>();
    Segment * segment = packet->data<Segment>();

    unsigned long long id;
    if(segment->flags() == Segment::SYN) // try to notify any eventual listener
        id = Connection::id(segment->to(), 0, IP::Address::NULL);
    else
        id = Connection::id(segment->to(), segment->from(), packet->from());

    db<TCP>(INF) << "TCP::update::condition=" << hex << id << endl;

    if(!_observed.notify(id, pool))
        pool->nic()->free(pool);
}


void TCP::Segment::sum(const IP::Address & from, const IP::Address & to, const void * data, unsigned int size)
{
    _checksum = 0;

    IP::Pseudo_Header pseudo(from, to, IP::TCP, sizeof(Header) + size);

    unsigned long sum = 0;
    const unsigned char * ptr = reinterpret_cast<const unsigned char *>(&pseudo);
    for(unsigned int i = 0; i < sizeof(IP::Pseudo_Header); i += 2)
        sum += (ptr[i] << 8) | ptr[i+1];

    ptr = reinterpret_cast<const unsigned char *>(header());
    for(unsigned int i = 0; i < sizeof(Header); i += 2)
        sum += (ptr[i] << 8) | ptr[i+1];

    if(data) {
        ptr = reinterpret_cast<const unsigned char *>(data);
        for(unsigned int i = 0; i < size; i += 2)
            sum += (ptr[i] << 8) | ptr[i+1];
        if(size & 1)
            sum += ptr[size - 1];
    }

    while(sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    _checksum = htons(~sum);
}

void TCP::Connection::send(const Flags & flags)
{
    db<TCP>(TRC) << "TCP::Connection::send(flags=" << ((flags & ACK) ? 'A' : '-') << ((flags & RST) ? 'R' : '-') << ((flags & SYN) ? 'S' : '-') << ((flags & FIN) ? 'F' : '-') << ")" << endl;

    _flags = flags;

    Buffer * buf = IP::alloc(peer(), IP::TCP, sizeof(Header), 0);
    if(!buf) {
        db<TCP>(WRN) << "TCP::send: failed to alloc a NIC buffer to send a TCP control segment!" << endl;
        return;
    }
    db<TCP>(INF) << "TCP::send:buf=" << buf << " => " << *buf<< endl;

    Packet * packet = buf->frame()->data<Packet>();
    Segment * segment = packet->data<Segment>();
    memcpy(segment, header(), sizeof(Header));
    segment->sum(packet->from(), packet->to(), 0, 0);

    db<TCP>(INF) << "TCP::Connection::send:conn=" << this << " => " << *this << endl;

    IP::send(buf); // implicitly releases the buffer
}

int TCP::Connection::send(const void * d, unsigned int size)
{
    const unsigned char * data = reinterpret_cast<const unsigned char *>(d);

    db<TCP>(TRC) << "TCP::send(f=" << from() << ",t=" << peer() << ":" << to() << ",d=" << data << ",s=" << size << ")" << endl;

    _flags = 0;

    Buffer * pool = IP::alloc(peer(), IP::TCP, sizeof(Header), size);
    if(!pool)
        return 0;

    unsigned int headers = sizeof(Header);
    for(Buffer::Element * el = pool->link(); el; el = el->next()) {
        Buffer * buf = el->object();
        Packet * packet = buf->frame()->data<Packet>();

        db<TCP>(INF) << "TCP::send:buf=" << buf << " => " << *buf<< endl;

        if(el == pool->link()) {
            Segment * segment = packet->data<Segment>();
            memcpy(segment, header(), sizeof(Header));
            segment->sum(packet->from(), packet->to(), segment, buf->size());
            memcpy(segment->data<void>(), data, buf->size() - sizeof(Header) - sizeof(IP::Header));
            data += buf->size() - sizeof(Header) - sizeof(IP::Header);

            db<TCP>(INF) << "TCP::send:msg=" << segment << " => " << *segment << endl;
        } else {
            memcpy(packet->data<void>(), data, buf->size() - sizeof(IP::Header));
            data += buf->size() - sizeof(IP::Header);
        }

        headers += sizeof(IP::Header);
    }

    return IP::send(pool) - headers; // implicitly releases the pool
}

int TCP::Connection::receive(Buffer * pool, void * d, unsigned int s)
{
    unsigned char * data = reinterpret_cast<unsigned char *>(d);

    db<TCP>(TRC) << "TCP::receive(buf=" << pool << ",d=" << d << ",s=" << s << ")" << endl;

    Buffer::Element * head = pool->link();
    Packet * packet = head->object()->frame()->data<Packet>();
    Segment * segment = packet->data<Segment>();
    unsigned int size = 0;

    for(Buffer::Element * el = head; el && (size <= s); el = el->next()) {
        Buffer * buf = el->object();

        db<UDP>(INF) << "TCP::receive:buf=" << buf << " => " << *buf << endl;

        packet = buf->frame()->data<Packet>();

        unsigned int len = buf->size() - sizeof(IP::Header);
        if(el == head) {
            len -= sizeof(Header);
            memcpy(data, segment->data<void>(), len);

            db<TCP>(INF) << "TCP::receive:msg=" << segment << " => " << *segment << endl;
        } else
            memcpy(data, packet->data<void>(), len);

        db<UDP>(INF) << "TCP::receive:len=" << len << endl;

        data += len;
        size += len;
    }

    pool->nic()->free(pool);

    if(!segment->check(size)) {
        db<TCP>(WRN) << "UDP::update: wrong message checksum!" << endl;
        size = 0;
    }

    return size;
}

void TCP::Connection::update(TCP::Observed * obs, unsigned long long socket, NIC::Buffer * pool)
{

    db<TCP>(TRC) << "TCP::Connection::update(obs=" << obs << ",sock=" << hex << socket << ",buf=" << pool << ")" << endl;

    Packet * packet = pool->frame()->data<Packet>();
    _current = packet->data<Segment>();
    _length = pool->size() - sizeof(IP::Header) - sizeof(TCP::Header);
    _peer_window = htons(_current->window());

    if(_state == LISTENING) {
        _peer = packet->from();
        _to = htons(_current->to());
        TCP::_observed.detach(this, from());
        TCP::_observed.attach(this, id());
    }

    db<TCP>(INF) << "TCP::Connection::update:conn=" << this << " => " << *this << endl;

    if((_state == ESTABLISHED) && _length) {
        db<TCP>(INF) << "TCP::Connection::update: notifying communicator!" << endl;
        if(!notify(socket, pool))
            pool->nic()->free(pool);
    } else
        (this->*_handler)();
}

void TCP::Connection::listen()
{
    db<TCP>(TRC) << "TCP::Connection::listen(at=" << hex << from() << ")" << endl;

    state(LISTENING);
    _transition.wait();
}

void TCP::Connection::connect()
{
    db<TCP>(TRC) << "TCP::Connection::connect(from=" << hex << from() << ",to=" << peer() << ":" << to() << ")" << endl;

    state(SYN_SENT);
//    set_timeout();
    send(SYN);
    _sequence = htonl(sequence() + 1);
    _transition.wait();
}

void TCP::Connection::close()
{
    db<TCP>(TRC) << "TCP::Connection::close()" << endl;

    if(_state == LISTENING)
        state(CLOSED);
    else {
        if(_state == SYN_SENT)
            state(CLOSED);
        else if((_state == ESTABLISHED) || (_state == SYN_RECEIVED))
            state(FIN_WAIT1);
        else if(_state == CLOSE_WAIT)
            state(LAST_ACK);
        //    set_timeout();
        send(FIN);
        _sequence = htonl(sequence() + 1);
        _transition.wait();
    }
}

void TCP::Connection::listening()
{
    db<TCP>(TRC) << "TCP::Connection::listening()" << endl;

    if((_current->flags() & SYN) && !(_current->flags() & RST) && !(_current->flags() & FIN)) {
        _to = ntohs(_current->from());
        _acknowledgment = ntohl(_current->sequence() + 1);
        state(SYN_RECEIVED);
        send(SYN | ACK);
        _sequence = htonl(sequence() + 1);
    }
}

void TCP::Connection::syn_sent()
{
    db<TCP>(TRC) << "TCP::Connection::syn_sent()" << endl;

    if((_current->flags() & RST) || (_current->flags() & FIN)) {
        db<TCP>(WRN) << "TCP::Connection::syn_sent: reset received, closing connection!" << endl;

        state(CLOSED);
        return;
    }

    if((_current->flags() & SYN) && (_current->flags() & ACK)) { // at the connecting side after receiving SYN + ACK
        if(_current->acknowledgment() == (ntohl(_unacknowledged) + 1)) {
            db<TCP>(INF) << "TCP::Connection::syn_sent: connection established!" << endl;

            _acknowledgment = ntohl(_current->sequence() + 1);

            state(ESTABLISHED);

            //       clear_timeout();
            send(ACK);
            _transition.signal();
            return;
        } else {
            db<TCP>(WRN) << "TCP::Connection::syn_sent: bad acknowledgment number!" << endl;

            //            clear_timeout();
            state(CLOSED);
            return;
        }
    }

    db<TCP>(WRN) << "TCP::Connection::syn_sent: bad FSM transaction! Closing connection!" << endl;

    close();
}

void TCP::Connection::syn_received()
{
    db<TCP>(TRC) << "TCP::Connection::syn_received()" << endl;

    if((_current->flags() & RST) || (_current->flags() & FIN)) {
        db<TCP>(WRN) << "TCP::Connection::syn_received: reset received! Closing connection!" << endl;

        //        clear_timeout();
        state(CLOSED);
        return;
    }

    if(_current->flags() & ACK) { // at the listening side after receiving ACK
        db<TCP>(INF) << "TCP::Connection::syn_received: connection established!" << endl;

        _acknowledgment = _current->sequence() + 1;
        state(ESTABLISHED);
        //        clear_timeout();
        _transition.signal();
        return;
    }

    db<TCP>(WRN) << "TCP::Connection::syn_received: bad FSM transaction! Closing connection!" << endl;

    state(CLOSED);
//        clear_timeout();
}

void TCP::Connection::established()
{
    db<TCP>(TRC) << "TCP::Connection::established()" << endl;

    // Timeout ACK
//    if (!check_seq(r,len))
//    {
//        if ((len) && (r.seq_num() < rcv_nxt))
//            send(ACK);();
//        return;
//    }

    if(_current->flags() & RST) {
        db<TCP>(WRN) << "TCP::Connection::established: reset received! Closing connection!" << endl;
        close();
        return;
    }

    if(_current->flags() & FIN) {
        state(CLOSE_WAIT);
        send(ACK);
        // reset timeout
    }
}

void TCP::Connection::fin_wait1()
{
    db<TCP>(TRC) << "TCP::Connection::fin_wait1()" << endl;

    if((_current->flags() & FIN) && (_current->flags() & ACK)) {
        state(TIME_WAIT);
        //        reset_timeout();
        send(ACK);
        return;
    }

    if(_current->flags() & ACK) {
        state(FIN_WAIT2);
        return;
    }

    if(_current->flags() & FIN) {
        state(CLOSING);
        send(ACK);
//        clear_timeout();
    }
}

void TCP::Connection::fin_wait2()
{
    db<TCP>(TRC) << "TCP::Connection::fin_wait1()" << endl;

    if(_current->flags() & FIN) {
        state(TIME_WAIT);
        send(ACK);
        //        clear_timeout();
    }
}

void TCP::Connection::close_wait()
{
    db<TCP>(TRC) << "TCP::Connection::close_wait1()" << endl;

    if(_state == CLOSED) {
        send(FIN);
//        clear_timeout();
    }
}

void TCP::Connection::closing()
{
    db<TCP>(TRC) << "TCP::Connection::closing()" << endl;

    if(_current->flags() & ACK) {
        state(CLOSED);
//        clear_timeout();
        _transition.signal();
    }
}

void TCP::Connection::last_ack()
{
    db<TCP>(TRC) << "TCP::Connection::last_ack()" << endl;
    
    if(_current->flags() & ACK) {
        state(CLOSED);
//        clear_timeout();
    }
}

void TCP::Connection::time_wait()
{
    db<TCP>(TRC) << "TCP::Connection::time_wait()" << endl;

    // Delay
//    clear_timeout();

    state(CLOSED);
}

void TCP::Connection::closed()
{
    // does nothing
}

__END_SYS

