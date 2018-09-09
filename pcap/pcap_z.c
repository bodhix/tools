#include <pcap/pcap.h>
#include <errno.h>
#include <time.h>
#include "../clog.h"

void dump_devs(pcap_if_t *devs)
{
    pcap_if_t *dev = devs;
    while (dev)
    {
        linfo("name: %s, desc: %s, flags: %u", dev->name, dev->description, dev->flags);
        dev = dev->next;
    }
}

void handler(u_char *userdata, const struct pcap_pkthdr *pkthdr, const u_char *pkt)
{
    linfo("tv_sec: %ld, tv_usec: %ld, userdata: %s, pkthdr->caplen: %u, pkthdr->len: %u",
        pkthdr->ts.tv_sec, pkthdr->ts.tv_usec, userdata, pkthdr->caplen, pkthdr->len);
}


int main(int argc, char const *argv[])
{
    // get all device
    char errbuf[PCAP_ERRBUF_SIZE] = {0};
    pcap_if_t *dev, *devs = NULL;
    pcap_t *handle = NULL;
    struct pcap_pkthdr header;
    u_char *pkt = NULL;
    int ret = 0;

    ret = pcap_findalldevs(&devs, errbuf);
    if (ret < 0)
        lerror_exit("pcap_findalldevs");

    dump_devs(devs);

    dev = devs;
    char *name = dev->name;
    int promisc = 1;
    int timeout = 0;

    linfo("pcap_open_live: %s", name);
    handle = pcap_open_live(name, PCAP_BUF_SIZE, promisc, timeout, errbuf);
    if (handle == NULL)
        lerror_exit("pcap_open_live: %s", pcap_strerror(errno));

    // pkt = pcap_next(handle, &header);

    int pkt_cnt = 10;
    ret = pcap_dispatch(handle, pkt_cnt, handler, pkt);
    linfo("pcap_dispatch ret: %d", ret);

    ret = pcap_loop(handle, pkt_cnt, handler, pkt);
    linfo("pcap_loop ret: %d", ret);

    pcap_close(handle);
    return 0;
}