#include "stdafx.h"
#include "c_ping.h"

#include <ws2tcpip.h>
#ifdef _MSC_VER
#include <iostream>
using namespace std;
#else
#include <iostream.h>	
#endif
#include <windows.h>
#include "..\\COMMON\\Compatability.h"

// ICMP packet types
#define ICMP_ECHO_REPLY 0
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11
#define ICMP_ECHO_REQUEST 8

// Minimum ICMP packet size, in bytes
#define ICMP_MIN 8

#ifdef _MSC_VER
// The following two structures need to be packed tightly, but unlike
// Borland C++, Microsoft C++ does not do this by default.
#pragma pack(1)
#endif

// The IP header
struct IPHeader {
	BYTE h_len:4;           // Length of the header in dwords
	BYTE version:4;         // Version of IP
	BYTE tos;               // Type of service
	WORD total_len;       // Length of the packet in dwords
	WORD ident;           // unique identifier
	WORD flags;           // Flags
	BYTE ttl;               // Time to live
	BYTE proto;             // Protocol number (TCP, UDP etc)
	WORD checksum;        // IP checksum
	ULONG source_ip;
	ULONG dest_ip;
};

// ICMP header
struct ICMPHeader {
	BYTE type;          // ICMP packet type
	BYTE code;          // Type sub code
	WORD checksum;
	WORD id;
	WORD seq;
	ULONG timestamp;    // not part of ICMP, but we need it
};

#ifdef _MSC_VER
#pragma pack()
#endif

WORD ip_checksum(WORD* buffer, __int32 size)
{
	unsigned long cksum = 0;

	// Sum all the words together, adding the final byte if size is odd
	while (size > 1)
    {
		cksum += *buffer++;
		size -= sizeof(WORD);
	}
    
	if (size)
    {
		cksum += *(BYTE*)buffer;
	}

	// Do a little shuffling
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	// Return the bitwise complement of the resulting mishmash
	return (WORD)(~cksum);
}



//////////////////////////// setup_for_ping ////////////////////////////
// Creates the Winsock structures necessary for sending and recieving
// ping packets.  host can be either a dotted-quad IP address, or a
// host name.  ttl is the time to live (a.k.a. number of hops) for the
// packet.  The other two parameters are outputs from the function.
// Returns < 0 for failure.

int setup_for_ping(char* host, __int32 ttl, SOCKET& sd, sockaddr_in& dest)
{
	// Create the socket
	sd = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (sd == INVALID_SOCKET)
	{
		//cerr << "Failed to create raw socket: " << WSAGetLastError() <<endl;
		return -1;
	}

	if (setsockopt(sd, IPPROTO_IP, IP_TTL, (const char*)&ttl,
		sizeof(ttl)) == SOCKET_ERROR)
	{
		//cerr << "TTL setsockopt failed: " << WSAGetLastError() << endl;
		return -1;
	}

	// Initialize the destination host info block
	memset(&dest, 0, sizeof(sockaddr_in));

	// Turn first passed parameter into an IP address to ping
	unsigned __int32 addr = inet_addr(host);
	if (addr != INADDR_NONE)
    {
		// It was a dotted quad number, so save result
		dest.sin_addr.s_addr = addr;
		dest.sin_family = AF_INET;
	}
	else
	{
		// Not in dotted quad form, so try and look it up
		hostent* hp = gethostbyname(host);
		if (hp != 0)
		{
			// Found an address for that host, so save it
			memcpy(&dest.sin_addr, hp->h_addr, hp->h_length);
			dest.sin_family = hp->h_addrtype;
		}
		else {
			// Not a recognized hostname either!
			//cerr << "Failed to resolve " << host << endl;
			return -1;
		}
	}

	return 0;
}



/////////////////////////// init_ping_packet ///////////////////////////
// Fill in the fields and data area of an ICMP packet, making it 
// packet_size bytes by padding it with a byte pattern, and giving it
// the given sequence number.  That completes the packet, so we also
// calculate the checksum for the packet and place it in the appropriate
// field.

void init_ping_packet(ICMPHeader* icmp_hdr, __int32 packet_size, __int32 seq_no)
{
	// Set up the packet's fields
	icmp_hdr->type = ICMP_ECHO_REQUEST;
	icmp_hdr->code = 0;
	icmp_hdr->checksum = 0;
	icmp_hdr->id = (WORD)GetCurrentProcessId();
	icmp_hdr->seq = (WORD)seq_no;
	icmp_hdr->timestamp = GetTickCount();

	// "You're dead meat now, packet!"
	const unsigned long __int32 deadmeat = 0xDEADBEEF;
	char* datapart = (char*)icmp_hdr + sizeof(ICMPHeader);
	int bytes_left = packet_size - sizeof(ICMPHeader);
	while (bytes_left > 0)
	{
		memcpy(datapart, &deadmeat, min(int(sizeof(deadmeat)), 
			bytes_left));
		bytes_left -= sizeof(deadmeat);
		datapart += sizeof(deadmeat);
	}

	// Calculate a checksum on the result
	icmp_hdr->checksum = ip_checksum((WORD*)icmp_hdr, packet_size);
}


/////////////////////////////// send_ping //////////////////////////////
// Send an ICMP echo ("ping") packet to host dest by way of sd with
// packet_size bytes.  packet_size is the total size of the ping packet
// to send, including the ICMP header and the payload area; it is not
// checked for sanity, so make sure that it's at least 
// sizeof(ICMPHeader) bytes, and that send_buf points to at least
// packet_size bytes.  Returns < 0 for failure.

int send_ping(SOCKET sd, const sockaddr_in& dest, ICMPHeader* send_buf,
			  __int32 packet_size, DWORD Timeout)
{
    __int32 Result=0;
    WSAOVERLAPPED SendOver;
    memset(&SendOver,0,sizeof(SendOver));
    SendOver.hEvent = WSACreateEvent();

    WSABUF SendBuf;
    SendBuf.len = packet_size;
    SendBuf.buf =(char*)send_buf;

    DWORD Flags =0;
    DWORD BytesSended=0;

	// Send the ping packet in send_buf as-is
	int bwrote = WSASendTo(sd, &SendBuf, 1, &BytesSended, Flags,(sockaddr*)&dest, sizeof(sockaddr_in), &SendOver, NULL );
	if (bwrote == SOCKET_ERROR)
	{
		if (WSAGetLastError()==WSA_IO_PENDING)
		{
            if(WSAWaitForMultipleEvents(1, &SendOver.hEvent, TRUE, Timeout, FALSE)==WSA_WAIT_EVENT_0)
            {
                WSAGetOverlappedResult(sd, &SendOver, &BytesSended, FALSE, &Flags);
            }else Result=-1;
        }else Result=-1;
	}
    
    if (BytesSended!=SendBuf.len) Result=-1;  
    if (SendOver.hEvent!=NULL)  CloseHandle(SendOver.hEvent);
    
	return Result;
}


/////////////////////////////// recv_ping //////////////////////////////
// Receive a ping reply on sd into recv_buf, and stores address info
// for sender in source.  On failure, returns < 0, 0 otherwise.  
// 
// Note that recv_buf must be larger than send_buf (passed to send_ping)
// because the incoming packet has the IP header attached.  It can also 
// have IP options set, so it is not sufficient to make it 
// sizeof(send_buf) + sizeof(IPHeader).  We suggest just making it
// fairly large and not worrying about wasting space.

int recv_ping(SOCKET sd, sockaddr_in& source, IPHeader* recv_buf,
			  __int32 packet_size, DWORD Timeout)
{
    __int32 Result=0;
	// Wait for the ping reply
	int fromlen = sizeof(source);
    WSAOVERLAPPED RecieveOver;
    memset(&RecieveOver,0,sizeof(RecieveOver));
    RecieveOver.hEvent = WSACreateEvent();

    WSABUF RecvBuf;
    RecvBuf.len = packet_size + sizeof(IPHeader);
    RecvBuf.buf =(char*)recv_buf;

    DWORD Flags =0;
    DWORD BytesRecieved=0;
    __int32 bread=WSARecvFrom(sd, &RecvBuf,1, &BytesRecieved, &Flags,(sockaddr*)&source, &fromlen, &RecieveOver, NULL);
    if(bread==SOCKET_ERROR)
	{
		if (WSAGetLastError()==WSA_IO_PENDING)
		{
            if(WSAWaitForMultipleEvents(1, &RecieveOver.hEvent, TRUE, Timeout, FALSE)==WSA_WAIT_EVENT_0)
            {
                WSAGetOverlappedResult(sd, &RecieveOver, &BytesRecieved, FALSE, &Flags);
            }else Result=-1;
    	}else Result=-1;
	}
    if(RecieveOver.hEvent!=NULL)CloseHandle(RecieveOver.hEvent);

	return Result;
}


///////////////////////////// decode_reply /////////////////////////////
// Decode and output details about an ICMP reply packet.  Returns -1
// on failure, -2 on "try again" and 0 on success.

int decode_reply(IPHeader* reply, __int32 bytes, sockaddr_in* from)
{
	// Skip ahead to the ICMP header within the IP packet
	unsigned short header_len = (reply->h_len)*4;
	ICMPHeader* icmphdr = (ICMPHeader*)((char *)reply + header_len);

	// Make sure the reply is sane
	if (bytes < header_len + ICMP_MIN)
	{
		//cerr << "too few bytes from " << inet_ntoa(from->sin_addr) << endl;
		return -1;
	}
	else if (icmphdr->type != ICMP_ECHO_REPLY)
	{
		if (icmphdr->type != ICMP_TTL_EXPIRE)
		{
			if (icmphdr->type == ICMP_DEST_UNREACH)
            {
                //cerr << "Destination unreachable" << endl;
                return -4;
            }
			else
            {
                //cerr << "Unknown ICMP packet type " << int(icmphdr->type) <<" received" << endl;
                return -4;
            }
		}
		// If "TTL expired", fall through.  Next test will fail if we
		// try it, so we need a way past it.
	}
	else if (icmphdr->id != (WORD)GetCurrentProcessId())
    {
		// Must be a reply for another pinger running locally, so just
		// ignore it.
		return -2;
	}

	// Figure out how far the packet travelled
	int nHops = int(256 - reply->ttl);
	if (nHops == 192)
    {
		// TTL came back 64, so ping was probably to a host on the
		// LAN -- call it a single hop.
		//nHops = 1;
	}
	else if (nHops == 128)
    {
		// Probably localhost
		//nHops = 0;
	}

	// Okay, we ran the gamut, so the packet must be legal -- dump it
//	cout << endl << bytes << " bytes from " <<
//		inet_ntoa(from->sin_addr) << ", icmp_seq " << icmphdr->seq << ", ";
	if (icmphdr->type == ICMP_TTL_EXPIRE)
    {
		//cout << "TTL expired." << endl;
        return -4;
	}
	else
    {
		//cout << nHops << " hop" << (nHops == 1 ? "" : "s");
		//cout << ", time: " << (GetTickCount() - icmphdr->timestamp) << " ms." << endl;
	}

	return 0;
}

/////////////////////////// allocate_buffers ///////////////////////////
// Allocates send and receive buffers.  Returns < 0 for failure.
#define DEFAULT_PACKET_SIZE 32
#define DEFAULT_TTL 30
#define MAX_PING_DATA_SIZE 1024
#define MAX_PING_PACKET_SIZE (MAX_PING_DATA_SIZE + sizeof(IPHeader))


int allocate_buffers(ICMPHeader*& send_buf, IPHeader*& recv_buf,
					 __int32 packet_size)
{
	// First the send buffer
	send_buf = (ICMPHeader*)new char[packet_size];
	if (send_buf == NULL)
	{
		//cerr << "Failed to allocate output buffer." << endl;
		return -1;
	}

	// And then the receive buffer
	recv_buf = (IPHeader*)new char[MAX_PING_PACKET_SIZE];
	if (recv_buf == NULL)
	{
		//cerr << "Failed to allocate output buffer." << endl;
		return -1;
	}

	return 0;
}

void deallocate_buffers(ICMPHeader*& send_buf, IPHeader*& recv_buf)
{
	if (send_buf != NULL)
	{
        delete ((char *)send_buf);
        send_buf=NULL;
	}

	if (recv_buf != NULL)
	{
        delete ((char *)recv_buf);
        recv_buf=NULL;
	}
}

int PingHost(char * HostName, __int32 PacketSize ,int ttl, DWORD Timeout)
{
	// Init some variables at top, so they aren't skipped by the
	// cleanup routines.
    __int32 Result;
	int seq_no = 0;
	ICMPHeader* send_buf = NULL;
	IPHeader* recv_buf = NULL;

	// Figure out how big to make the ping packet
	if (PacketSize == 0) PacketSize=DEFAULT_PACKET_SIZE;
	if ((ttl <= 0) && (ttl >= 255))ttl=DEFAULT_TTL;
	int MinVal=MIN(DWORD,MAX_PING_DATA_SIZE, (DWORD)PacketSize);
	PacketSize = MAX(DWORD,sizeof(ICMPHeader),	MinVal);


	// Set up for pinging
	SOCKET sd;
	sockaddr_in dest, source;
	if (!setup_for_ping(HostName, ttl, sd, dest))
	{
		if (!allocate_buffers(send_buf, recv_buf, PacketSize))
		{
			init_ping_packet(send_buf, PacketSize, seq_no);

			// Send the ping and receive the reply
			if (send_ping(sd, dest, send_buf, PacketSize, Timeout) >= 0)
			{
                DWORD BeginTime = GetTickCount();
				while (true)
				{
                    if ((GetTickCount()-BeginTime)>Timeout)
                    {
                        Result=-1;
                        break;
                    }
					// Receive replies until we either get a successful read,
					// or a fatal error occurs.
					if (recv_ping(sd, source, recv_buf, MAX_PING_PACKET_SIZE, Timeout) < 0)
					{
						// Pull the sequence number out of the ICMP header.  If 
						// it's bad, we just complain, but otherwise we take 
						// off, because the read failed for some reason.
						unsigned short header_len = recv_buf->h_len * 4;
						ICMPHeader* icmphdr = (ICMPHeader*)
							((char*)recv_buf + header_len);
						if (icmphdr->seq != seq_no)
						{
							cerr << "bad sequence number!" << endl;
							continue;
						}
						else
						{
                            Result=-1;
							break;
						}
					}

                    __int32 DecodeResult = decode_reply(recv_buf, PacketSize, &source);

                    if(DecodeResult==0)
                    {//OK
                        Result=0;
                        break;
                    }   
                    else if (DecodeResult==-1)
                    {
                        Result=-1;
                        break;
                    }
				}
			} else Result=-1;
		} else Result=1;
	}else Result=-1;

    shutdown(sd, SD_BOTH);
    closesocket(sd);
    deallocate_buffers(send_buf, recv_buf);
	return Result;
}
