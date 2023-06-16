### The example scripts provided here are based on IEEE 802.11ad, practical network implementation, and "[TGad Evaluation Methodology document](https://nam12.safelinks.protection.outlook.com/?url=https%3A%2F%2Fwww.ieee802.org%2F11%2FReports%2Ftgad_update.htm&amp;data=04%7C01%7Cyuchen.liu%40gatech.edu%7C3392376e47974f632cb808d8e9792e69%7C482198bbae7b4b258b7a6d7f32faa083%7C0%7C0%7C637516053978860685%7CUnknown%7CTWFpbGZsb3d8eyJWIjoiMC4wLjAwMDAiLCJQIjoiV2luMzIiLCJBTiI6Ik1haWwiLCJXVCI6Mn0%3D%7C1000&amp;sdata=tXSAjh36UPl2pFAc3x5igH3BBgFL13GBxQ3fpI7B7Kk%3D&amp;reserved=0)".
(Note: We implemented and mapped exactly the physical lab scenarios, traffic models, and link budget parameters for point-to-point links. The directional antenna model used in this repo is slightly different from the standardized antennas in practice, where an analytical codebook is implemented based on Gaussian directional pattern formula to define the shape of beam patterns. Besides, we have not modeled the hardware impairments and PHY abstractions in this digital twin.)

### 1. "TGad_Functional_test_living_room.cc (Home Network Digital Twin)"

Desciption: This script is used to map the network performance of a living room scenario as defined in digital twin. The scenario consists of several furniture items, 1 DMG AP and six DMG STAs, where three of them are LoS to AP, and others are NLoS to AP. 

<img id="img1" src="figures/func_test.png" width="500" height="280" align="middle">

### 2. "TGad_performance_evaluation_living_room.cc (Home Network Digital Twin 2)"

Description: This script is used to map the network performance of a living room scenario in digital twin. The scenario consists of several furniture items, 1 set-top box (STB) and 1 TV, where uncompressed video traffic is transferred between STB and TV.

<img id="img1" src="figures/eva_living_room.png" width="500" height="280" align="middle">

### 3. "TGad_performance_evaluation_conference_room.cc (Office Network Digital Twin)"

Description: This script is used to map the network performance of office conference room scenario in digital twin. The scenario consists of several furniture items, 1 AP (STA 0) and 8 STAs. Traffic model used among them includes lightly compressed video, local file transfer, and web browsing.

<img id="img1" src="figures/eva_conf_room.png" width="450" height="280" align="middle">

### 4. "TGad_performance_evaluation_enterprise_cubicle.cc (Enterprise Network Digital Twin)"

Description: This script is used to map the network performance of enterprise cubicle scenario in digital twin. The scenario consists of several cubicle furniture items, 1 AP (STA 0) and 3 STAs (this is an example case and users can make the customized cases). Traffic model used among them includes lightly compressed video, local file transfer, web browsing, and hard disk file trasfer.

<img id="img1" src="figures/eva_cubicle_room.png" width="450" height="280" align="middle">

### 5. "TGad_rand_scenario.cc (Customized Network Digital Twin)"

Description: This script is used to map the network performance of random scenario with TGad features in digital twin. The scenario consists of randomly distributed obstacles and STAs. This is an example case, and users can make the customized cases by simply adjusting related parameters.

<img id="img1" src="figures/eva_rand_scenario.png" width="450" height="280" align="middle">

### 6. Mapping signal strengths at any receiver locations (radio maps) in digital twin

Description: This function can be found in our recent research paper "Y. Liu and D. Blough, “Environment-Aware Link Quality Prediction for Millimeter-Wave Wireless LANs”, Proc. of ACM International Symposium on Mobility Management and Wireless Access, 2022. (Best Paper Award)".

<img id="img1" src="figures/attribute.PNG" width="450" height="320" align="middle">

(a) Mapped network environment. (b)-(c) radio maps from the view of AP1 and AP2 (at the height plane of 0.8m), respectively.

