/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015-2019 IMDEA Networks Institute
 * Author: Hany Assasa <hany.assasa@gmail.com>
 */
#ifndef DMG_ADHOC_WIFI_MAC_H
#define DMG_ADHOC_WIFI_MAC_H

#include "dmg-wifi-mac.h"

#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"

#include "supported-rates.h"
#include "amsdu-subframe-header.h"

namespace ns3  {

class MgtAddBaRequestHeader;
class UniformRandomVariable;

/**
 * \ingroup wifi
 *
 * Experimental Station mode for P2P communication (No beacon header interval, only data transmission).
 */
class DmgAdhocWifiMac : public DmgWifiMac
{
public:
  static TypeId GetTypeId (void);

  DmgAdhocWifiMac ();
  virtual ~DmgAdhocWifiMac ();

  /**
   * Get Association Identifier (AID).
   * \return The AID of the station.
   */
  virtual uint16_t GetAssociationID (void);
  virtual void SetAddress (Mac48Address address);
  void SetLinkUpCallback (Callback<void> linkUp);
  virtual void Enqueue (Ptr<Packet> packet, Mac48Address to);
  /**
   * Add manually the best antenna configuration for communication with a specific DMG AD-HOC STA.
   * \param txSectorID The ID of the transmit sector.
   * \param txAntennaID The ID of the transmit phased antenna array.
   * \param rxSectorID The ID of the receive sector.
   * \param rxAntennaID The ID of the receive phased antenna array..
   * \param address The MAC address of the peer DMG AD-HOC STA.
   */
  void AddAntennaConfig (SectorID txSectorID, AntennaID txAntennaID,
                         SectorID rxSectorID, AntennaID rxAntennaID,
                         Mac48Address address);
  /**
   * Add manually the best antenna configuration for communication with a specific DMG AD-HOC STA.
   * \param txSectorID The ID of the transmit sector.
   * \param txAntennaID The ID of the transmit phased antenna array.
   * \param address The MAC address of the peer DMG AD-HOC STA.
   */
  void AddAntennaConfig (SectorID txSectorID, AntennaID txAntennaID, Mac48Address address);

protected:
  virtual void DoDispose (void);
  virtual void DoInitialize (void);

  void StartBeaconInterval (void);
  void EndBeaconInterval (void);
  void StartBeaconTransmissionInterval (void);
  void StartAssociationBeamformTraining (void);
  void StartAnnouncementTransmissionInterval (void);
  void StartDataTransmissionInterval (void);
  void FrameTxOk (const WifiMacHeader &hdr);
  void BrpSetupCompleted (Mac48Address address);
  virtual void NotifyBrpPhaseCompleted (void);
  /**
   * Get MultiBand Element corresponding to this DMG STA.
   * \return Pointer to the MultiBand element.
   */
  Ptr<MultiBandElement> GetMultiBandElement (void) const;

private:
  virtual void Receive (Ptr<WifiMacQueueItem> mpdu);
  /**
   * Return the DMG capability of the current STA.
   *
   * \return the DMG capability that we support
   */
  Ptr<DmgCapabilities> GetDmgCapabilities (void) const;
  /**
   * The packet we sent was successfully received by the receiver
   * (i.e. we received an ACK from the receiver). If the packet
   * was an association response to the receiver, we record that
   * the receiver is now associated with us.
   *
   * \param hdr the header of the packet that we successfully sent
   */
  virtual void TxOk (Ptr<const Packet> packet, const WifiMacHeader &hdr);

};

} // namespace ns3

#endif /* DMG_ADHOC_WIFI_MAC_H */
