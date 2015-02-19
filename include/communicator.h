// EPOS Communicator Declarations

#ifndef __communicator_h
#define __communicator_h

#include <network.h>
#include <udp.h>
#include <tcp.h>

__BEGIN_SYS 

// Commonalities for connectionless channels
template<typename Channel, typename Network, bool connectionless = Channel::connectionless>
class Communicator_Common: protected Channel::Observer
{
public:
    // List to hold received Buffers
    typedef NIC::Buffer Buffer;
    typedef Buffer::List List;
    typedef List::Element Element;

    // Channel imports
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;
    typedef typename Channel::Observer::Observing_Condition Observing_Condition;

protected:
    Communicator_Common(const Local_Address & local): _local(local), _ready(0) {
        Channel::attach(this, local);
    }

public:
    ~Communicator_Common() {
        Channel::detach(this, _local);
    }

    int send(const Address & to, const void * data, unsigned int size) {
        return Channel::send(_local, to, data, size);
    }
    int send(const Local_Address & from, const Address & to, const void * data, unsigned int size) {
        return Channel::send(from, to, data, size);
    }

    int receive(void * data, unsigned int size) {
        _ready.p();
        Element * el = _received.remove();
        Buffer * buf = el->object();
        delete el;
        return Channel::receive(buf, data, size);
    }
    int receive(Address * from, void * data, unsigned int size) {
        _ready.p();
        Element * el = _received.remove();
        Buffer * buf = el->object();
        delete el;
        return Channel::receive(buf, from, data, size);
    }

private:
    void update(typename Channel::Observed * obs, Observing_Condition c, Buffer * buf) {
        _received.insert(new (SYSTEM) Element(buf));
        _ready.v();
    }

protected:
    Local_Address _local;
    Semaphore _ready;
    List _received;
};

// Commonalities for connection-oriented channels
template<typename Channel, typename Network>
class Communicator_Common<Channel, Network, false>: protected Channel::Observer
{
public:
    // List to hold received Buffers
    typedef NIC::Buffer Buffer;
    typedef Buffer::List List;
    typedef List::Element Element;

    // Channel imports
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;
    typedef typename Channel::Observer::Observing_Condition Observing_Condition;

protected:
    Communicator_Common(const Local_Address & local, const Address & peer): _local(local), _ready(0) {
        _connection = Channel::attach(this, local, peer);
    }

public:
    ~Communicator_Common() {
        Channel::detach(this, _connection);
    }

    int send(const void * data, unsigned int size) {
        return _connection->send(data, size);
    }

    int receive(void * data, unsigned int size) {
        _ready.p();
        Element * el = _received.remove();
        Buffer * buf = el->object();
        delete el;
        return _connection->receive(buf, data, size);
    }

private:
    void update(typename Channel::Observed * obs, Observing_Condition c, Buffer * buf) {
        _received.insert(new (SYSTEM) Element(buf));
        _ready.v();
    }

protected:
    Local_Address _local;
    Semaphore _ready;
    List _received;

    typename Channel::Connection * _connection;
};


// Link (point-to-point communicator) for connectionless channels
template<typename Channel, typename Network = typename Channel::Network, bool connectionless = Channel::connectionless>
class Link: public Communicator_Common<Channel, Network>
{
private:
    typedef Communicator_Common<Channel, Network, connectionless> Base;

public:
    // Channel imports
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;

public:
    Link(const Local_Address & local, const Address & peer = Address::NULL): Base(local), _peer(peer) {}
    ~Link() {}

    int send(const void * data, unsigned int size) { return Base::send(_peer, data, size); }
    int receive(void * data, unsigned int size) { return Base::receive(data, size); }

    int read(void * data, unsigned int size) { return receive(data, size); }
    int write(const void * data, unsigned int size) { return send(data, size); }

    const Address & peer() const { return _peer;}

private:
    Address _peer;
};

// Link (point-to-point communicator) for connection-oriented channels
template<typename Channel, typename Network>
class Link<Channel, Network, false>: public Communicator_Common<Channel, Network>
{
private:
    typedef Communicator_Common<Channel, Network, false> Base;

public:
    // Channel imports
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;

public:
    Link(const Local_Address & local, const Address & peer = Address::NULL): Base(local, peer), _peer(peer) {}
    ~Link() {}

    int send(const void * data, unsigned int size) { return Base::send(data, size); }
    int receive(void * data, unsigned int size) { return Base::receive(data, size); }

    int read(void * data, unsigned int size) { return receive(data, size); }
    int write(const void * data, unsigned int size) { return send(data, size); }

    const Address & peer() const { return _peer;}

private:
    Address _peer;
};


// Port (1-to-N communicator) for connectionless channels
template<typename Channel, typename Network = typename Channel::Network, bool connectionless = Channel::connectionless>
class Port: public Communicator_Common<Channel, Network, connectionless>
{
private:
    typedef Communicator_Common<Channel, Network, connectionless> Base;

public:
    // Channel imports
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;

public:
    Port(const Local_Address & local): Base(local) {}
    ~Port() {}

    int send(const Address & to, const void * data, unsigned int size) { return Base::send(to, data, size); }
    int receive(Address * from, void * data, unsigned int size) { return Base::receive(from, data, size); }
};

// Port (1-to-N communicator) for connection-oriented channels
template<typename Channel, typename Network>
class Port<Channel, Network, false>: public Communicator_Common<Channel, Network>
{
private:
    typedef Communicator_Common<Channel, Network, false> Base;

public:
    // Channel imports
    typedef typename Channel::Address Address;
    typedef typename Channel::Address::Local Local_Address;

public:
    Port(const Local_Address & local): Base(local) {}
    ~Port() {}

    Link<Channel, Network> * listen() { return new Link<Channel, Network>(Channel::listen(this->_local)); }
    Link<Channel, Network> * connect(const Address & to) { return new Link<Channel, Network>(Channel::connect(this->_local, to)); }
};

__END_SYS

#endif
