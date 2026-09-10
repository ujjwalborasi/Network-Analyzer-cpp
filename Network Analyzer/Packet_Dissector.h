#pragma once
// Write A class to handle packet dissection

#include <string>
#include "Logger.h"
#include <iostream>
#include "Packet.h"
#include "SdpLayer.h"
#include "EthLayer.h"
#include "ChdlcLayer.h"
#include "DhcpLayer.h"
#include "DnsLayer.h"
#include "HttpLayer.h" 
#include "IPv4Layer.h"
#include "IcmpLayer.h"
#include "TcpLayer.h"
#include "RtpLayer.h"
#include "UdpLayer.h"
#include "SipLayer.h"
class PacketDissector {
	public:
	static PacketDissector& GetInstance();

	void DissectPacket(const pcpp::Packet& packet);

	

	private:
		PacketDissector() {}
		PacketDissector(const PacketDissector&) = delete;
		PacketDissector& operator=(const PacketDissector&) = delete;
		};