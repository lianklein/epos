// EPOS Ethernet Mediator Common Package

#include <nic.h>

#ifndef __ethernet_h
#define __ethernet_h

#include <cpu.h>
#include <utility/list.h>
#include <utility/observer.h>

__BEGIN_SYS

class Ethernet: private NIC_Common
{
protected:
    static const unsigned int MTU = 1500;
    static const unsigned int HEADER_SIZE = 14;


public:
    typedef NIC_Common::Address<6> Address;

    typedef unsigned short Protocol;
    enum
    {
        IP     = 0x0800,
        ARP    = 0x0806,
        RARP   = 0x8035,
        ELP    = 0x8888,
        PTP    = 0x88F7
    };

    typedef unsigned char Data[MTU];
    typedef NIC_Common::CRC32 CRC;


    // The Ethernet Header (RFC 894)
    class Header
    {
    public:
        Header() {}
        Header(const Address & src, const Address & dst, const Protocol & prot):
            _dst(dst), _src(src), _prot(htons(prot)) {}

        friend Debug & operator<<(Debug & db, const Header & h) {
            db << "{" << h._dst << "," << h._src << "," << h.prot() << "}";
            return db;
        }

        const Address & src() const { return _src; }
        const Address & dst() const { return _dst; }

        Protocol prot() const { return ntohs(_prot); }

    protected:
        Address _dst;
        Address _src;
        Protocol _prot;
    } __attribute__((packed, may_alias));


    // The Ethernet Frame (RFC 894)
    class Frame: public Header
    {
    public:
        Frame() {}
        Frame(const Address & src, const Address & dst, const Protocol & prot) : Header(src, dst, prot) {}
        Frame(const Address & src, const Address & dst, const Protocol & prot, const void * data, unsigned int size): Header(src, dst, prot) {
            memcpy(_data, data, size);
        }
        
        Header * header() { return this; }

        template<typename T>
        T * data() { return reinterpret_cast<T *>(&_data); }

        friend Debug & operator<<(Debug & db, const Frame & f) {
            db << "{" << f.dst() << "," << f.src() << "," << f.prot() << "," << f._data << "}";
            return db;
        }
        
    protected:
        Data _data;
        CRC _crc;
    } __attribute__((packed));

    typedef Frame PDU;


    // Buffers used to hold frames across a zero-copy network stack
    class Buffer: private Frame
    {
    public:
        typedef Simple_List<Buffer> List;
        typedef List::Element Element;

    public:
        Buffer(void * back): _lock(false), _nic(0), _back(back), _size(sizeof(Frame)), _link(this) {}
        Buffer(NIC * nic, const Address & src, const Address & dst, const Protocol & prot, unsigned int size):
            Frame(src, dst, prot), _lock(false), _nic(nic), _size(size), _link(this) {}

        Frame * frame() { return this; }

        bool lock() { return !CPU::tsl(_lock); }
        void unlock() { _lock = 0; }

        NIC * nic() const { return _nic; }
        void nic(NIC * n) { _nic = n; }

        template<typename T>
        T * back() const { return reinterpret_cast<T *>(_back); }

        unsigned int size() const { return _size; }
        void size(unsigned int s) { _size = s; }

        Element * link() { return &_link; }

        friend Debug & operator<<(Debug & db, const Buffer & b) {
            db << "{nc=" << b._nic << ",lk=" << b._lock << ",sz=" << b._size << ",bl=" << b._back << "}";
            return db;
        }

    private:
        volatile bool _lock;
        NIC * _nic;
        void * _back;
        unsigned int _size;
        Element _link;
    };


    // Observers of a protocol get a also a pointer to the received buffer
    typedef Data_Observer<Buffer, Protocol> Observer;
    typedef Data_Observed<Buffer, Protocol> Observed;


    // Meaningful statistics for Ethernet
    struct Statistics: public NIC_Common::Statistics
    {
        Statistics(): rx_overruns(0), tx_overruns(0), frame_errors(0), carrier_errors(0), collisions(0) {}

        friend Debug & operator<<(Debug & db, const Statistics & s) {
            db << "{rxp=" << s.rx_packets
               << ",rxb=" <<  s.rx_bytes
               << ",rxorun=" <<  s.rx_overruns
               << ",txp=" <<  s.tx_packets
               << ",txb=" <<  s.tx_bytes
               << ",txorun=" <<  s.tx_overruns
               << ",frm=" <<  s.frame_errors
               << ",car=" <<  s.carrier_errors
               << ",col=" <<  s.collisions
               << "}";
            return db;
        }
        
        unsigned int rx_overruns;
        unsigned int tx_overruns;
        unsigned int frame_errors;
        unsigned int carrier_errors;
        unsigned int collisions;
    };

protected:
    Ethernet() {}

public:
    static const unsigned int mtu() { return MTU; }
    static const Address broadcast() { return Address::BROADCAST; }
};

__END_SYS

#endif
