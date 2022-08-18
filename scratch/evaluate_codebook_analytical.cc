/*
 * Copyright (c) 2015-2020 IMDEA Networks Institute
 * Author: Hany Assasa <hany.assasa@gmail.com>
 */
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "common-functions.h"

/**
 * Simulation Objective:
 * Evaluate IEEE 802.11ad beamforming procedure in BTI + A-BFT. After completing BTI and A-BFT
 * access periods, we print both the selected transmit antenna ID and its sector ID for each device.
 *
 * The user defines a custom analytical codebook for each device as following:
 * The DMG AP has a single RF chain with two phased antenna arrays:
 *    Phased Antenna Array (1): 12 Sectors, Antenna Orientation = 0 degree.
 *    Phased Antenna Array (2):  4 Sectors, Antenna Orientation = 180 degree.
 * The DMG STA uses a single RF chain with a single phased antenna array with 6 setors and Antenna
 * orinetation equals 0 degree.
 *
 * Network Topology:
 * Network topology is simple and consists of a single access point and one DMG STA.
 *
 *
 *                      DMG AP (0,0)                    DMG STA (X,Y)
 *
 *
 * Running the Simulation:
 * To run the script, type one of the following commands to change the location of the DMG STA
 * and check the corresponding best antenna sector used for communication:
 * ./waf --run "evaluate_custom_analytical_codebook --x_pos=1  --y_pos=0"
 * ./waf --run "evaluate_custom_analytical_codebook --x_pos=1  --y_pos=1"
 * ./waf --run "evaluate_custom_analytical_codebook --x_pos=0  --y_pos=1"
 * ./waf --run "evaluate_custom_analytical_codebook --x_pos=-1 --y_pos=1"
 * ./waf --run "evaluate_custom_analytical_codebook --x_pos=-1 --y_pos=0"
 * ./waf --run "evaluate_custom_analytical_codebook --x_pos=-1 --y_pos=-1"
 * ./waf --run "evaluate_custom_analytical_codebook --x_pos=0  --y_pos=-1"
 * ./waf --run "evaluate_custom_analytical_codebook --x_pos=1  --y_pos=-1"
 *
 * Simulation Output:
 * 1. The selected transmit antenna ID and its sector ID for each device.
 */

NS_LOG_COMPONENT_DEFINE ("EvaluateCodebookAnalytical");

using namespace ns3;
using namespace std;

Ptr<Node> apWifiNode;
Ptr<Node> staWifiNode;
Ptr<DmgApWifiMac> apWifiMac;
Ptr<DmgStaWifiMac> staWifiMac;

void
SLSCompleted (Ptr<DmgWifiMac> wifiMac, SlsCompletionAttrbitutes attributes)
{
  if (wifiMac == apWifiMac)
    {
      std::cout << "DMG AP " << apWifiMac->GetAddress ()
                << " completed SLS phase with DMG STA " << attributes.peerStation << std::endl;
    }
  else
    {
      std::cout << "DMG STA " << staWifiMac->GetAddress ()
                << " completed SLS phase with DMG AP " << attributes.peerStation << std::endl;
    }
  std::cout << "Best Tx Antenna Configuration: AntennaID=" << uint16_t (attributes.antennaID)
            << ", SectorID=" << uint16_t (attributes.sectorID) << std::endl;
}

int
main (int argc, char *argv[])
{
  string phyMode = "DMG_MCS12";                 /* Type of the Physical Layer. */
  double x_pos = 1.0;                           /* The X position of the DMG STA. */
  double y_pos = 0.0;                           /* The Y position of the DMG STA. */
  bool verbose = false;                         /* Print Logging Information. */
  double simulationTime = 1;                    /* Simulation time in seconds. */
  bool pcapTracing = true;                      /* PCAP Tracing is enabled or not. */

  /* Command line argument parser setup. */
  CommandLine cmd;

  cmd.AddValue ("phyMode", "802.11ad PHY Mode", phyMode);
  cmd.AddValue ("x_pos", "The X position of the DMG STA", x_pos);
  cmd.AddValue ("y_pos", "The Y position of the DMG STA", y_pos);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("pcap", "Enable PCAP Tracing", pcapTracing);
  cmd.Parse (argc, argv);

  /* Configure RTS/CTS and Fragmentation */
  ConfigureRtsCtsAndFragmenatation ();

  /**** DmgWifiHelper is a meta-helper ****/
  DmgWifiHelper wifi;

  /* Turn on logging */
  if (verbose)
    {
      wifi.EnableLogComponents ();
      LogComponentEnable ("EvaluateCodebookAnalytical", LOG_LEVEL_ALL);
    }

  /**** Set up Channel ****/
  DmgWifiChannelHelper wifiChannel ;
  /* Simple propagation delay model */
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  /* Friis model with standard-specific wavelength */
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (56.16e9));

  /**** SETUP ALL NODES ****/
  DmgWifiPhyHelper wifiPhy = DmgWifiPhyHelper::Default ();
  /* Nodes will be added to the channel we set up earlier */
  wifiPhy.SetChannel (wifiChannel.Create ());
  /* All nodes transmit at 10 dBm == 10 mW, no adaptation */
  wifiPhy.Set ("TxPowerStart", DoubleValue (20.0));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (20.0));
  wifiPhy.Set ("TxPowerLevels", UintegerValue (1));
  /* Set operating channel */
  wifiPhy.Set ("ChannelNumber", UintegerValue (2));
  /* Set default algorithm for all nodes to be constant rate */
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode));

  /* Make two nodes and set them up with the phy and the mac */
  NodeContainer wifiNodes;
  wifiNodes.Create (2);
  apWifiNode = wifiNodes.Get (0);
  staWifiNode = wifiNodes.Get (1);

  /**** Allocate DMG Wifi MAC ****/
  DmgWifiMacHelper wifiMac = DmgWifiMacHelper::Default ();

  Ssid ssid = Ssid ("Numerical");
  wifiMac.SetType ("ns3::DmgApWifiMac",
                   "Ssid", SsidValue(ssid),
                   "BE_MaxAmpduSize", UintegerValue (262143), //Enable A-MPDU with the highest maximum size allowed by the standard
                   "BE_MaxAmsduSize", UintegerValue (0),
                   "SSSlotsPerABFT", UintegerValue (8), "SSFramesPerSlot", UintegerValue (8),
                   "EnableBeaconRandomization", BooleanValue (true),
                   "BeaconInterval", TimeValue (MicroSeconds (102400)));

  /* Set Analytical Codebook for the DMG Devices */
  wifi.SetCodebook ("ns3::CodebookAnalytical",
                    "CodebookType", EnumValue (EMPTY_CODEBOOK));

  NetDeviceContainer apDevice;
  apDevice = wifi.Install (wifiPhy, wifiMac, apWifiNode);

  wifiMac.SetType ("ns3::DmgStaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false),
                   "BE_MaxAmpduSize", UintegerValue (262143), //Enable A-MPDU with the highest maximum size allowed by the standard
                   "BE_MaxAmsduSize", UintegerValue (0));

  NetDeviceContainer staDevice;
  staDevice = wifi.Install (wifiPhy, wifiMac, staWifiNode);

  /** Add custom entry to the Analytical Codebook **/
  Ptr<WifiNetDevice> apWifiNetDevice = StaticCast<WifiNetDevice> (apDevice.Get (0));
  Ptr<WifiNetDevice> staWifiNetDevice = StaticCast<WifiNetDevice> (staDevice.Get (0));
  Ptr<CodebookAnalytical> codebook;
  apWifiMac = StaticCast<DmgApWifiMac> (apWifiNetDevice->GetMac ());
  staWifiMac = StaticCast<DmgStaWifiMac> (staWifiNetDevice->GetMac ());

  /* Define DMG PCP/AP Codebook */
  codebook = StaticCast<CodebookAnalytical> (apWifiMac->GetCodebook ());
  /* Add RF chain */
  RFChainID rfchainID = 1;
  codebook->AppendRFChain (rfchainID);
  /* Add Antenna Array with AntennaID = 1 */
  codebook->AppendAntenna (rfchainID, 1, 0, 0);
  codebook->AppendSector (1,  1,   0, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  2,  30, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  3,  60, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  4,  90, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  5, 120, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  6, 150, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  7, 180, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  8, 210, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  9, 240, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1, 10, 270, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1, 11, 300, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1, 12, 330, 30, TX_RX_SECTOR, BHI_SLS_SECTOR);
  /* Add Antenna Array with AntennaID = 2 */
  codebook->AppendAntenna (rfchainID, 2, 180, 0);
  codebook->AppendSector (2,  1,   0, 90, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (2,  2,  90, 90, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (2,  3, 180, 90, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (2,  4, 270, 90, TX_RX_SECTOR, BHI_SLS_SECTOR);

  /* Define STA Codebook */
  codebook = StaticCast<CodebookAnalytical> (staWifiMac->GetCodebook ());
  /* Add RF chain */
  codebook->AppendRFChain (rfchainID);
  /* Add Antenna Array with AntennaID = 1 */
  codebook->AppendAntenna (rfchainID, 1, 0, 0);
  codebook->AppendSector (1,  1,   0, 60, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  2,  60, 60, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  3, 120, 60, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  4, 180, 60, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  5, 240, 60, TX_RX_SECTOR, BHI_SLS_SECTOR);
  codebook->AppendSector (1,  6, 300, 60, TX_RX_SECTOR, BHI_SLS_SECTOR);

  /* Setting mobility model, Initial Position 1 meter apart */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (x_pos, y_pos, 0.0));

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiNodes);

  /* Internet stack*/
  InternetStackHelper stack;
  stack.Install (wifiNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface;
  apInterface = address.Assign (apDevice);
  Ipv4InterfaceContainer staInterface;
  staInterface = address.Assign (staDevice);

  /* Populate routing table */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* Enable Traces */
  if (pcapTracing)
    {
      wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
      wifiPhy.EnablePcap ("Traces/AccessPoint", apDevice, false);
      wifiPhy.EnablePcap ("Traces/Station", staDevice, false);
    }

  /* Connect SLS traces */
  apWifiMac->TraceConnectWithoutContext ("SLSCompleted", MakeBoundCallback (&SLSCompleted, apWifiMac));
  staWifiMac->TraceConnectWithoutContext ("SLSCompleted", MakeBoundCallback (&SLSCompleted, staWifiMac));

  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();

  cout << "End Simulation at " << Simulator::Now ().GetSeconds () << endl;

  Simulator::Destroy ();

  return 0;
}
