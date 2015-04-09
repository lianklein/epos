#ifndef __net_traits_h
#define __net_traits_h

#include <system/config.h>

__BEGIN_SYS

template<> struct Traits<Network>: public Traits<void>
{
    static const bool enabled = (Traits<Build>::NODES > 1);

//    static const bool debugged = true;

    static const unsigned int NODES = Traits<Build>::NODES;
    static const unsigned int RETRIES = 3;
    static const unsigned int TIMEOUT = 10; // s

    // This list is positional, with one network for each NIC in traits<NIC>::NICS
    typedef LIST<IP> NETWORKS;
};

template<> struct Traits<ARP<NIC, IP> >: public Traits<Network>
{
};

template<> struct Traits<IP>: public Traits<Network>
{
    enum {STATIC, MAC, INFO, RARP, DHCP};

    struct Default_Config {
        static const unsigned int  TYPE    = DHCP;
        static const unsigned long ADDRESS = 0;
        static const unsigned long NETMASK = 0;
        static const unsigned long GATEWAY = 0;
    };

    template<unsigned int UNIT>
    struct Config: public Default_Config {};

    static const unsigned int TTL  = 0x40; // Time-to-live
};

template<> struct Traits<IP>::Config<0> //: public Traits<IP>::Default_Config
{
    static const unsigned int  TYPE      = MAC;
    static const unsigned long ADDRESS   = 0x0a000100;   // 10.0.1.x x=MAC[5]
    static const unsigned long NETMASK   = 0xffffff00;   // 255.255.255.0
    static const unsigned long GATEWAY   = 0;   // 10.0.1.1
};

template<> struct Traits<IP>::Config<1>: public Traits<IP>::Default_Config
{
};

template<> struct Traits<UDP>: public Traits<Network>
{
    static const bool checksum = true;
};

template<> struct Traits<TCP>: public Traits<Network>
{
    static const bool debugged = true;
    static const unsigned int WINDOW = 4096;
};

template<> struct Traits<DHCP>: public Traits<Network>
{
};

__END_SYS

#endif
