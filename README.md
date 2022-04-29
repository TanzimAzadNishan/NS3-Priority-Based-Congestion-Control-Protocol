# Priority-Based-Congestion-Control-Protocol(PCCP)
Implemented in **```ns-3```** for **```IEEE 802.11 network```**.

```diff
+ The algorithm works in the Network layer to control upstream congestion.
```


## **Project Overview**


1) Nodes may have different **```priority```** due to their function or location and need to gain different throughput. So the algorithm sets priority to each node.
2) The output rate of a node is controlled by adjusting the **```scheduling rate```** which depends on the priority.
3) PCCP maintains a **```weighted fairness```** so that the sink can get different(but in a weighted fair way) throughput from its child sensor nodes.
4) PCCP measures congestion level(degree) for each node by calculating **```packet inter-arrival time```** & **```packet service time```** which reflects the present congestion condition.
5) Any node(sink) can inform its child nodes to change transmission rate according to the congestion level.


<br /><br />
## **Topology**
<p align="center">
   <img src="https://github.com/TanzimAzadNishan/Priority-Based-Congestion-Control-Protocol/blob/main/Topology.png" 
        width="400" height="250"/>
</p>



## **Result**

### **Throughput**
<p align="center">
   <img src="https://github.com/TanzimAzadNishan/Priority-Based-Congestion-Control-Protocol/blob/main/Throughput.png" 
        width="400" height="300"/>
</p>


**```coverageSide means one side of coverage area```**
> With the increase in flows, the network is getting more and more congested which reduces the throughput.



### **Normalized Weighted Throughput**
<p align="center">
   <img src="https://github.com/TanzimAzadNishan/Priority-Based-Congestion-Control-Protocol/blob/main/Normalized_Throughput.png" 
        width="400" height="300"/>
</p>


**```coverageSide means one side of coverage area```**
> When we take the normalized value of the throughput, we see the change in value is very small with the increase in flow. It almost remains constant.

<br /><br />
### **Modification**

Find the files in **```pccp```** folder of this repository and copy those in their respective folders of **```ns-3.xx```** 
mentioned below.

1. src/internet/model/
   - node-option.h
   - node-option.cc

2. src/network/model
   - node.h
   - node.cc

3. src/traffic-control/model
   - queue-disc.cc
   - queue-disc.h
   - traffic-control-layer.cc
   - traffic-control-layer.h

4. build/ns3/net-device.h

5. src/internet/wscript

### **Attributes**
- **```node-option.h```**
   - ```m_Ta```**:** Mean packet inter-arrival time of a node
   - ```m_Ts```**:** Mean packet service time of a node
   - ```m_Offsprings```**:** Actual number of active offsprings(those who are transmitting packets) of a node
   - ```m_parentOffsprings```**:** What a node knows about the number of active offsprings the parent node has. When some offsprings of the parent node become active or idle, this attribute's value will be different from **m_Offsprings**.
   - ```m_sp```**:** Source traffic priority(SP) of a node
   - ```m_gp```**:** Global priority(GP) of a node. GP = SP + TP where TP is the Transit traffic priority of a node
   - ```m_svc```**:** Scheduling rate of a node
   - ```m_parentDegree```**:** Congestion Degree of a node's parent
   - ```m_parentAddress```**:** Ipv4 address of a node's parent. If the destination device's ipv4 address matches the source node's parent's ipv4 address, the traffic is a upstream traffic.
   - ```m_pktsSinceLastICD```**:**


- **```node.h```**

- **```queue-disc.h```**

- **```traffic-control-layer.h```**


<br /><br />
## **Run the Project**

### Installation
Download the latest version of ns-3. **```ns-3.35```** is used in this project.
Extract the tar.bz2 file. You will get a ns-allinone-3.xx folder, which will have these files and directories.

Then enter the following command:
> ./build.py --enable-examples --enable-tests


Now go to the ns-3.xx directory. Then run the commands:
```
./waf clean
./waf configure --build-profile=debug --enable-examples --enable-tests
```


### Run the Simulation File

> ./waf --run "scratch/pccpTest --nFlows=20" --cwd="Output/pccpTest"


In console, you will see the logs of the simulation. The graphs will be stored in **```Output/pccpTest```** folder.


<br /><br />
## **Developers**
#### [Md. Tanzim Azad](https://github.com/TanzimAzadNishan)








