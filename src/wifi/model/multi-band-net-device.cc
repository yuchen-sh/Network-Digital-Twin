/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015-2019 IMDEA Networks Institute
 * Author: Hany Assasa <hany.assasa@gmail.com>
 */

#include "ns3/llc-snap-header.h"
#include "ns3/channel.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "dmg-sta-wifi-mac.h"
#include "sta-wifi-mac.h"
#include "multi-band-net-device.h"

#include "regular-wifi-mac.h"
#include "wifi-mac-queue.h"
#include "ns3/node.h"
#include "wifi-phy.h"
#include "ns3/llc-snap-header.h"
#include "ns3/socket.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/net-device-queue-interface.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MultiBandNetDevice");

NS_OBJECT_ENSURE_REGISTERED (MultiBandNetDevice);

TypeId
MultiBandNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MultiBandNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<MultiBandNetDevice> ()
    .SetGroupName ("Wifi")
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH),
                   MakeUintegerAccessor (&MultiBandNetDevice::SetMtu,
                                         &MultiBandNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> (1,MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH))
  ;
  return tid;
}

MultiBandNetDevice::MultiBandNetDevice ()
  : m_configComplete (false)
{
  NS_LOG_FUNCTION_NOARGS ();
}

MultiBandNetDevice::~MultiBandNetDevice ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
MultiBandNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  WifiTechnology *technology;
  m_node = 0;
  for (WifiTechnologyList::iterator item = m_list.begin (); item != m_list.end (); item++)
    {
      technology = &item->second;
      technology->Phy->Dispose ();
      technology->Mac->Dispose ();
      technology->StationManager->Dispose ();
      technology->Phy = 0;
      technology->Mac = 0;
      technology->StationManager = 0;
    }
  NetDevice::DoDispose ();
}

void
MultiBandNetDevice::DoInitialize (void)
{
  WifiTechnology *technology;
  for (WifiTechnologyList::iterator item = m_list.begin (); item != m_list.end (); item++)
    {
      technology = &item->second;
      technology->Phy->Initialize ();
      technology->Mac->Initialize ();
      technology->StationManager->Initialize ();
    }
  NetDevice::DoInitialize ();
}

void
MultiBandNetDevice::CompleteConfig (void)
{
  WifiTechnology *technology;
  if (m_node == 0 || m_configComplete)
    {
      return;
    }

  for (WifiTechnologyList::iterator item = m_list.begin (); item != m_list.end (); item++)
    {
      technology = &item->second;
      if (technology->Mac == 0
          || technology->Phy == 0
          || technology->StationManager == 0)
        {
          return;
        }
    }

  for (WifiTechnologyList::iterator item = m_list.begin (); item != m_list.end (); item++)
    {
      technology = &item->second;
      technology->Mac->SetWifiPhy (technology->Phy);
      technology->Mac->SetWifiRemoteStationManager (technology->StationManager);
      technology->Mac->SetForwardUpCallback (MakeCallback (&MultiBandNetDevice::ForwardUp, this));
      technology->Mac->SetLinkUpCallback (MakeCallback (&MultiBandNetDevice::LinkUp, this));
      technology->Mac->SetLinkDownCallback (MakeCallback (&MultiBandNetDevice::LinkDown, this));
      technology->StationManager->SetupPhy (technology->Phy);
      technology->StationManager->SetupMac (technology->Mac);
    }
  m_configComplete = true;
}

void
MultiBandNetDevice::AddNewWifiTechnology (Ptr<WifiPhy> phy, Ptr<WifiMac> mac, Ptr<WifiRemoteStationManager> station,
                                          WifiPhyStandard standard, bool operational)
{
  WifiTechnology technology;
  technology.Phy = phy;
  technology.Mac = mac;
  technology.StationManager = station;
  technology.Standard = standard;
  technology.Operational = operational;
  m_list[standard] =  technology;
}

WifiTechnologyList
MultiBandNetDevice::GetWifiTechnologyList (void) const
{
  return m_list;
}

void
MultiBandNetDevice::SwitchTechnology (WifiPhyStandard standard)
{
  NS_LOG_FUNCTION (this << standard);
  WifiTechnology *technology = &m_list[standard];
  m_mac = technology->Mac;
  m_phy = technology->Phy;
  m_stationManager = technology->StationManager;
  m_standard = standard;
}

void
MultiBandNetDevice::BandChanged (WifiPhyStandard standard, Mac48Address address, bool isInitiator)
{
  NS_LOG_FUNCTION (this << standard << address << isInitiator);
  /* Before switching the current technology, we keep a pointer to the current technology */
  Ptr<RegularWifiMac> oldMac, newMac;
  oldMac = StaticCast<RegularWifiMac> (m_mac);

  /* Switch current active technology for 802.11 */
  SwitchTechnology (standard);

  /* In all cases, we copy the content of all the queues (DCA + EDCA) */
  newMac = StaticCast<RegularWifiMac> (m_mac);
//  m_technologyMap[address] = newMac;

  /* Copy DCA Packets */
  oldMac->GetTxop ()->GetWifiMacQueue ()->TransferPacketsByAddress (address, newMac->GetTxop ()->GetWifiMacQueue ());
  /* Copy EDCA Packets */
  oldMac->GetVOQueue ()->GetWifiMacQueue ()->TransferPacketsByAddress (address, newMac->GetVOQueue ()->GetWifiMacQueue ());
  oldMac->GetVIQueue ()->GetWifiMacQueue ()->TransferPacketsByAddress (address, newMac->GetVIQueue ()->GetWifiMacQueue ());
  oldMac->GetBEQueue ()->GetWifiMacQueue ()->TransferPacketsByAddress (address, newMac->GetBEQueue ()->GetWifiMacQueue ());
  oldMac->GetBKQueue ()->GetWifiMacQueue ()->TransferPacketsByAddress (address, newMac->GetBKQueue ()->GetWifiMacQueue ());

  /* Copy Block ACK aggreements */
  oldMac->GetVOQueue ()->CopyBlockAckAgreements (address, newMac->GetVOQueue ());
  oldMac->GetVIQueue ()->CopyBlockAckAgreements (address, newMac->GetVIQueue ());
  oldMac->GetBEQueue ()->CopyBlockAckAgreements (address, newMac->GetBEQueue ());
  oldMac->GetBKQueue ()->CopyBlockAckAgreements (address, newMac->GetBKQueue ());

  /* Check the type of the BSS */
  if (newMac->GetTypeOfStation () == DMG_STA)
    {
      /* Copy Association State */
      Ptr<DmgStaWifiMac> newDmgMac = StaticCast<DmgStaWifiMac> (newMac);
      newMac->SetBssid (oldMac->GetBssid ());
      newDmgMac->SetState (oldMac->GetMacState ());
    }
  else if (newMac->GetTypeOfStation () == STA)
    {
      /* Copy Association State */
      Ptr<StaWifiMac> newWifiMac = StaticCast<StaWifiMac> (newMac);
      newMac->SetBssid (oldMac->GetBssid ());
      newWifiMac->SetState (oldMac->GetMacState ());
    }
  else if ((newMac->GetTypeOfStation () == AP) || (newMac->GetTypeOfStation () == DMG_AP))
    {
      /* Copy Association State */
      m_stationManager->RecordGotAssocTxOk (Mac48Address (address));
    }

  m_mac->NotifyBandChanged (standard, address, isInitiator);
}

void
MultiBandNetDevice::EstablishFastSessionTransferSession (Mac48Address address)
{
  NS_LOG_FUNCTION (this << address);
  Ptr<RegularWifiMac> mac = StaticCast<RegularWifiMac> (m_mac);
  mac->SetupFSTSession (address);
}

Ptr<WifiMac>
MultiBandNetDevice::GetTechnologyMac (WifiPhyStandard standard)
{
  WifiTechnology *technology = &m_list[standard];
  return technology->Mac;
}

Ptr<WifiPhy>
MultiBandNetDevice::GetTechnologyPhy (WifiPhyStandard standard)
{
  WifiTechnology *technology = &m_list[standard];
  return technology->Phy;
}

Ptr<WifiMac>
MultiBandNetDevice::GetMac (void) const
{
  return m_mac;
}

Ptr<WifiPhy>
MultiBandNetDevice::GetPhy (void) const
{
  return m_phy;
}

Ptr<WifiRemoteStationManager>
MultiBandNetDevice::GetRemoteStationManager (void) const
{
  return m_stationManager;
}

void
MultiBandNetDevice::SetIfIndex (const uint32_t index)
{
  m_ifIndex = index;
}

uint32_t
MultiBandNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}

Ptr<Channel>
MultiBandNetDevice::GetChannel (void) const
{
  return m_phy->GetChannel ();
}

void
MultiBandNetDevice::SetAddress (Address address)
{
  m_address = Mac48Address::ConvertFrom (address);
}

Address
MultiBandNetDevice::GetAddress (void) const
{
  return m_address;
}

bool
MultiBandNetDevice::SetMtu (const uint16_t mtu)
{
  if (mtu > MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH)
    {
      return false;
    }
  m_mtu = mtu;
  return true;
}

uint16_t
MultiBandNetDevice::GetMtu (void) const
{
  return m_mtu;
}

bool
MultiBandNetDevice::IsLinkUp (void) const
{
  return m_linkUp;
}

void
MultiBandNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  m_linkChanges.ConnectWithoutContext (callback);
}

bool
MultiBandNetDevice::IsBroadcast (void) const
{
  return true;
}

Address
MultiBandNetDevice::GetBroadcast (void) const
{
  return Mac48Address::GetBroadcast ();
}

bool
MultiBandNetDevice::IsMulticast (void) const
{
  return true;
}

Address
MultiBandNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address::GetMulticast (multicastGroup);
}

Address
MultiBandNetDevice::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address::GetMulticast (addr);
}

bool
MultiBandNetDevice::IsPointToPoint (void) const
{
  return false;
}

bool
MultiBandNetDevice::IsBridge (void) const
{
  return false;
}

bool
MultiBandNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
  NS_ASSERT (Mac48Address::IsMatchingType (dest));

  Mac48Address realTo = Mac48Address::ConvertFrom (dest);

  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);

//  m_mac = m_technologyMap[realTo];
  m_mac->NotifyTx (packet);
  m_mac->Enqueue (packet, realTo);
  return true;
}

Ptr<Node>
MultiBandNetDevice::GetNode (void) const
{
  return m_node;
}

void
MultiBandNetDevice::SetNode (Ptr<Node> node)
{
  m_node = node;
  CompleteConfig ();
}

bool
MultiBandNetDevice::NeedsArp (void) const
{
  return true;
}

void
MultiBandNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_forwardUp = cb;
}

void
MultiBandNetDevice::ForwardUp (Ptr<const Packet> packet, Mac48Address from, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << from << to);
  LlcSnapHeader llc;
  NetDevice::PacketType type;
  if (to.IsBroadcast ())
    {
      type = NetDevice::PACKET_BROADCAST;
    }
  else if (to.IsGroup ())
    {
      type = NetDevice::PACKET_MULTICAST;
    }
  else if (to == m_mac->GetAddress ())
    {
      type = NetDevice::PACKET_HOST;
    }
  else
    {
      type = NetDevice::PACKET_OTHERHOST;
    }

  Ptr<Packet> copy = packet->Copy ();
  if (type != NetDevice::PACKET_OTHERHOST)
    {
      m_mac->NotifyRx (packet);
      copy->RemoveHeader (llc);
      m_forwardUp (this, copy, llc.GetType (), from);
    }
  else
    {
      copy->RemoveHeader (llc);
    }

  if (!m_promiscRx.IsNull ())
    {
      m_mac->NotifyPromiscRx (copy);
      m_promiscRx (this, copy, llc.GetType (), from, to, type);
    }
}

void
MultiBandNetDevice::LinkUp (void)
{
  m_linkUp = true;
  m_linkChanges ();
}

void
MultiBandNetDevice::LinkDown (void)
{
  m_linkUp = false;
  m_linkChanges ();
}

bool
MultiBandNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  return false;
}

void
MultiBandNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  WifiTechnology *technology;
  m_promiscRx = cb;
  for (WifiTechnologyList::iterator item = m_list.begin (); item != m_list.end (); item++)
    {
      technology = &item->second;
      technology->Mac->SetPromisc ();
    }
}

bool
MultiBandNetDevice::SupportsSendFrom (void) const
{
  return false;
}

} //namespace ns3
