#include "Packet_Dissector.h"
#include <ctime>
#include <iomanip>
#include <chrono>
#include "RtpStreamCollector.h"

PacketDissector& PacketDissector::GetInstance() {
	static PacketDissector instance;
	return instance;
}
std::string timespecToString(const timespec& ts, const std::string& format = "%Y-%m-%d %H:%M:%S")
{
	char buffer[100];
	std::time_t time = ts.tv_sec;
	std::tm tm_info;
	localtime_s(&tm_info, &time);  // Use localtime_s instead of localtime
	std::strftime(buffer, sizeof(buffer), format.c_str(), &tm_info);
	snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), ".%09ld", ts.tv_nsec);
	return std::string(buffer);
}
void PacketDissector::DissectPacket(const pcpp::Packet& packet) {
	// write code to get timestamp of packet
	timespec ts = packet.getRawPacket()->getPacketTimeStamp();
	Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO,"    Timestamp: " + std::to_string(ts.tv_sec) + "." + std::to_string(ts.tv_nsec));
	std::string arrivalTimeStr = timespecToString(ts);
	std::string utcArrivalTimeStr = timespecToString(ts, "%Y-%m-%d %H:%M:%S %Z");
	// Convert timespec to epoch time
	double epochArrivalTime = ts.tv_sec + ts.tv_nsec / 1e9;
	Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Arrival Time: " + arrivalTimeStr + " India Standard Time");
	Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    UTC Arrival Time: " + utcArrivalTimeStr + " UTC");
	Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Epoch Arrival Time: " + std::to_string(epochArrivalTime));
	pcpp::EthLayer* eth_layer = packet.getLayerOfType<pcpp::EthLayer>();
	if(eth_layer != nullptr)
	{
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Ethernet Layer");
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Source MAC: " + eth_layer->getSourceMac().toString());
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Destination MAC: " + eth_layer->getDestMac().toString());
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Ethernet Type: " + std::to_string(eth_layer->getEthHeader()->etherType));
	}

	// VLAN: there can be stacked VLANs - iterate
	pcpp::VlanLayer* vlan_layer = packet.getLayerOfType<pcpp::VlanLayer>();
	int vlanIdx = 0;
	while (vlan_layer != nullptr) {
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    VLAN Layer #" + std::to_string(vlanIdx));
		// getVlanID is supported by PcapPlusPlus
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    VLAN ID: " + std::to_string(vlan_layer->getVlanID()));
		// get next stacked VLAN if any
		vlan_layer = packet.getNextLayerOfType<pcpp::VlanLayer>(vlan_layer);
		++vlanIdx;
	}

	pcpp::IPv4Layer* ip_layer = packet.getLayerOfType<pcpp::IPv4Layer>();
	if(ip_layer != nullptr)
	{
		Logger::GetInstance().Log(LogLevel::NETWORK_LAYER_INFO, "    IPv4 Layer");
		Logger::GetInstance().Log(LogLevel::NETWORK_LAYER_INFO, "    Source IP: " + ip_layer->getSrcIPv4Address().toString());
		Logger::GetInstance().Log(LogLevel::NETWORK_LAYER_INFO, "    Destination IP: " + ip_layer->getDstIPv4Address().toString());
		Logger::GetInstance().Log(LogLevel::NETWORK_LAYER_INFO, "    IP ID: " + std::to_string(ip_layer->getIPv4Header()->ipId));
	}

	// IPv6
	pcpp::IPv6Layer* ipv6_layer = packet.getLayerOfType<pcpp::IPv6Layer>();
	if (ipv6_layer != nullptr) {
		Logger::GetInstance().Log(LogLevel::NETWORK_LAYER_INFO, "    IPv6 Layer");
		Logger::GetInstance().Log(LogLevel::NETWORK_LAYER_INFO, "    Source IPv6: " + ipv6_layer->getSrcIPv6Address().toString());
		Logger::GetInstance().Log(LogLevel::NETWORK_LAYER_INFO, "    Destination IPv6: " + ipv6_layer->getDstIPv6Address().toString());
		// payloadLength is in network order - convert to host order without introducing new headers
		uint16_t pl = ipv6_layer->getIPv6Header()->payloadLength;
		uint16_t pl_host = static_cast<uint16_t>((pl >> 8) | (pl << 8));
		Logger::GetInstance().Log(LogLevel::NETWORK_LAYER_INFO, "    Payload Length: " + std::to_string(pl_host));
		Logger::GetInstance().Log(LogLevel::NETWORK_LAYER_INFO, "    Next Header: " + std::to_string(ipv6_layer->getIPv6Header()->nextHeader));
	}

	pcpp::IcmpLayer* icmp_layer = packet.getLayerOfType<pcpp::IcmpLayer>();
	if(icmp_layer != nullptr)
	{
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    ICMP Layer");
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    ICMP Type: " + std::to_string(icmp_layer->getIcmpHeader()->type));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    ICMP Code: " + std::to_string(icmp_layer->getIcmpHeader()->code));
	}

	pcpp::UdpLayer* udp_layer = packet.getLayerOfType<pcpp::UdpLayer>();
	if(udp_layer != nullptr)
	{
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    UDP Layer");
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Source Port: " + std::to_string(udp_layer->getUdpHeader()->portSrc));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Destination Port: " + std::to_string(udp_layer->getUdpHeader()->portDst));
		// add here code to log UDP length and checksum
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Length: " + std::to_string(udp_layer->getUdpHeader()->length));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Checksum: " + std::to_string(udp_layer->getUdpHeader()->headerChecksum));


	
	
	}
	pcpp::TcpLayer* tcp_layer = packet.getLayerOfType<pcpp::TcpLayer>();
	if(tcp_layer != nullptr)
	{
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    TCP Layer");
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Source Port: " + std::to_string(tcp_layer->getTcpHeader()->portSrc));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Destination Port: " + std::to_string(tcp_layer->getTcpHeader()->portDst));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Acknowledgement Number: " + std::to_string(tcp_layer->getTcpHeader()->ackNumber));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Sequence Number: " + std::to_string(tcp_layer->getTcpHeader()->sequenceNumber));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Window Size: " + std::to_string(tcp_layer->getTcpHeader()->windowSize));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Urgent Pointer: " + std::to_string(tcp_layer->getTcpHeader()->urgentPointer));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Acknowledgement Flags: " + std::to_string(tcp_layer->getTcpHeader()->ackFlag));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Reset Flag: " + std::to_string(tcp_layer->getTcpHeader()->rstFlag));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Push Flag: " + std::to_string(tcp_layer->getTcpHeader()->pshFlag));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Syn Flag: " + std::to_string(tcp_layer->getTcpHeader()->synFlag));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Fin Flag: " + std::to_string(tcp_layer->getTcpHeader()->finFlag));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Checksum: " + std::to_string(tcp_layer->getTcpHeader()->headerChecksum));
		Logger::GetInstance().Log(LogLevel::TRANSPORT_LAYER_INFO, "    Data Offset: " + std::to_string(tcp_layer->getTcpHeader()->dataOffset));

	}

	// MPLS
	pcpp::MplsLayer* mpls_layer = packet.getLayerOfType<pcpp::MplsLayer>();
	if (mpls_layer != nullptr) {
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    MPLS Layer: " + mpls_layer->toString());
	}

	// PPPoE (session or discovery)
	pcpp::PPPoESessionLayer* pppoes_layer = packet.getLayerOfType<pcpp::PPPoESessionLayer>();
	if (pppoes_layer != nullptr) {
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    PPPoE Session Layer");
		if (pppoes_layer->getPPPoEHeader()) {
			Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Session ID: " + std::to_string(pppoes_layer->getPPPoEHeader()->sessionId));
		}
	}
	pcpp::PPPoEDiscoveryLayer* pppoed_layer = packet.getLayerOfType<pcpp::PPPoEDiscoveryLayer>();
	if (pppoed_layer != nullptr) {
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    PPPoE Discovery Layer");
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Tags: " + std::to_string(pppoed_layer->getTagCount()));
	}

	pcpp::HttpResponseLayer* http_response_layer = packet.getLayerOfType<pcpp::HttpResponseLayer>();
	if (http_response_layer != nullptr)
	{
		// Log the HTTP response layer
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    HTTP Response Layer");
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    HTTP Version: " + http_response_layer->getFirstLine()->getVersion());
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    HTTP Status Code: " + std::to_string(http_response_layer->getFirstLine()->getStatusCode()));
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    HTTP Status Code Description: " + http_response_layer->getFirstLine()->getStatusCodeString());
	}

	pcpp::HttpRequestLayer* http_request_layer = packet.getLayerOfType<pcpp::HttpRequestLayer>();
	if (http_request_layer != nullptr)
	{
		// Log the HTTP request layer
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    HTTP Request Layer");
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    HTTP Method: " + http_request_layer->getFirstLine()->getMethod());
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    HTTP URI: " + http_request_layer->getFirstLine()->getUri());
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    HTTP Version: " + http_request_layer->getFirstLine()->getVersion());

	}
	// DNS
	pcpp::DnsLayer* dns_layer = packet.getLayerOfType<pcpp::DnsLayer>();
	if (dns_layer != nullptr) {
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    DNS Layer");
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    Queries: " + std::to_string(dns_layer->getQueryCount()));
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    Answers: " + std::to_string(dns_layer->getAnswerCount()));
	}

	pcpp::ArpLayer* arp_layer = packet.getLayerOfType<pcpp::ArpLayer>();
	if (arp_layer != nullptr)
	{
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    ARP Layer");
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Sender MAC: " + arp_layer->getSenderMacAddress().toString());
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Sender IP: " + arp_layer->getSenderIpAddr().toString());
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Target MAC: " + arp_layer->getTargetMacAddress().toString());
		Logger::GetInstance().Log(LogLevel::LINK_LAYER_INFO, "    Target IP: " + arp_layer->getTargetIpAddr().toString());
	}
	pcpp::RtpLayer* rtp_layer = packet.getLayerOfType<pcpp::RtpLayer>();
	if (rtp_layer != nullptr)
	{
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    RTP Layer");
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    Sequence Number: " + std::to_string(rtp_layer->getSequenceNumber()));
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    Timestamp: " + std::to_string(rtp_layer->getTimestamp()));
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    Raw header timestamp: " + std::to_string(rtp_layer->getRtpHeader()->timestamp)); // ADD THIS
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    SSRC: " + std::to_string(rtp_layer->getSSRC()));

		RtpStreamCollector::GetInstance().CollectFrame(*rtp_layer);
	}
	pcpp::UdpLayer* udp_layer_for_rtp = packet.getLayerOfType<pcpp::UdpLayer>();
	if (rtp_layer == nullptr && udp_layer_for_rtp != nullptr) {
		uint16_t srcPort = udp_layer_for_rtp->getUdpHeader()->portSrc;
		uint16_t dstPort = ntohs(54550); // known from SDP - compare against network-order port field directly instead; see note below

		// Simplify: compare against SDP-known port 54550 on either side
		if (udp_layer_for_rtp->getSrcPort() == 54550 || udp_layer_for_rtp->getDstPort() == 54550) {
			const uint8_t* udpPayload = udp_layer_for_rtp->getLayerPayload();
			size_t udpPayloadLen = udp_layer_for_rtp->getLayerPayloadSize();

			if (udpPayloadLen >= 12 && ((udpPayload[0] >> 6) & 0x03) == 2) { // version == 2 check
				RtpFrame frame;
				frame.payloadType = udpPayload[1] & 0x7F;
				frame.seqNum = (udpPayload[2] << 8) | udpPayload[3];
				frame.timestamp = (udpPayload[4] << 24) | (udpPayload[5] << 16) | (udpPayload[6] << 8) | udpPayload[7];
				uint32_t ssrc = (udpPayload[8] << 24) | (udpPayload[9] << 16) | (udpPayload[10] << 8) | udpPayload[11];
				frame.payload.assign(udpPayload + 12, udpPayload + udpPayloadLen);

				Logger::GetInstance().Log(LogLevel::DEBUG, "    RTP Layer (MANUAL FALLBACK - not auto-detected by PcapPlusPlus)");
				RtpStreamCollector::GetInstance().CollectFrame(ssrc, frame);
			}
		}
	}
	pcpp::SdpLayer* sdp_layer = packet.getLayerOfType<pcpp::SdpLayer>();
	if (sdp_layer != nullptr)
	{
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    SDP Layer");
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    Raw SDP:\n" + sdp_layer->toString());
	}
	pcpp::SipRequestLayer* sip_req = packet.getLayerOfType<pcpp::SipRequestLayer>();
	if (sip_req != nullptr) {
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    SIP Request Layer found");
	}
	pcpp::SipResponseLayer* sip_resp = packet.getLayerOfType<pcpp::SipResponseLayer>();
	if (sip_resp != nullptr) {
		Logger::GetInstance().Log(LogLevel::APPLICATION_LAYER_INFO, "    SIP Response Layer found");
	}
}
