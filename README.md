## Introduction:
This is a repository for the development of digital twin in next-generation wireless network (mmWave, 5G/6G) in network simulator ns-3.

## Features:
This initial version supports the following new features:

1. Accurate environment mapping and mirroring model.
1. Cuboid-based obstacle model (including both furniture-type obstacles and human obstacles).
1. Accurate network attributes (e.g., signal strength} determination function.
1. Different wireless channel models.
1. Indoor WLAN digital twin examples.
1. Support for emulation with physical LAN and network digital twin evolution (under development and continue to update).



## Building the Project:
The current implementation is based on ns-3.31. Please type the following command when building the project:

    ./waf configure --disable-examples --disable-tests --disable-python --enable-modules='applications','core','internet','point-to-point','wifi','flow-monitor','spectrum'
    ./waf build

Or, to build the project in optimized mode for fast execution type the following command:

    ./waf configure --disable-examples --disable-tests --disable-python --enable-modules='applications','core','internet','point-to-point','wifi','flow-monitor','spectrum' --enable-static -d optimized
    ./waf build
    

## Prerequisites:
Before start using this module, please keep the following in mind:

1. Understand WLAN IEEE 802.11 MAC/PHY, LTE, and 5G NR operations.
1. Get familiar with ns-3 and how to run simulations in ns-3.
1. Understand the existing wireless/Wifi Model in ns-3.

## Contact info:
yuchen.liu.sn at gmail dot com

## Acknowledgement
1. IMDEA Networks Institute, Madrid, Spain
1. Georgia Institute of Technology, Atlanta, USA
1. University of Washington, Seattle, USA
1. North Carolina State University, USA
1. NSF Funding Support CNS-2312138
