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

<br /><br />
### **Metrics**
- ```Source Traffic```**:** It is locally generated at a certain rate at each node
- ```Transit Traffic```**:** It is received from a node's child nodes through its MAC layer.
- ```Packet Inter-Arrival Time(ta)```**:** The time interval between two sequential arriving packets from either source or for the transit traffic at the MAC layer
- ```Packet Service Time(ts)```**:** The time interval between when a packet arrives at the MAC layer and when its last bit is successfully transmitted.
- ```Congestion Degree```**:** It is intended to reflect the current congestion level at each sensor node. When the inter-arrival time is smaller than the service time, the congestion degree, d is larger than 1 and the node experiences congestion. Otherwise when the congestion degree, d is smaller than 1, the incoming rate is below the outgoing rate, and hence congestion abates.
```javascript
   d = ts / ta
```


<br /><br />
### **Attributes**
- **```node-option.h```** - contains the relevant attributes that a node must carry to detect ```congestion``` & ```scheduling rate```.
   - ```m_Ta```**:** Mean packet inter-arrival time of a node
   - ```m_Ts```**:** Mean packet service time of a node
   - ```m_Offsprings```**:** Actual number of active offsprings(those who are transmitting packets) of a node
   - ```m_parentOffsprings```**:** What a node knows about the number of active offsprings the parent node has. When some offsprings of the parent node become active or idle, this attribute's value will be different from **m_Offsprings**.
   - ```m_sp```**:** Source traffic priority(SP) of a node
   - ```m_gp```**:** Global priority(GP) of a node. GP = SP + TP where TP is the Transit traffic priority of a node
   - ```m_svc```**:** Scheduling rate of a node
   - ```m_parentDegree```**:** Congestion Degree of a node's parent
   - ```m_parentAddress```**:** Ipv4 address of a node's parent. If the destination device's ipv4 address matches the source node's parent's ipv4 address, the traffic is a upstream traffic.<br /><br />


- **```node.h```** - Every node has a ```NodeOption object``` attached to it.
   - ```m_nodeOption```**:** NodeOption associated with a node
   - ```m_parent```**:** Parent of a node
   - ```m_prev```**:** If a packet has just arrived from node A to node B, node A is the previous node of node B.
   - ```entrTimeInMac```**:** To calculate the service time of the packet just transmitted, this attribute keeps track of the time at which the packet has just entered the MAC layer of the source node.<br /><br />

- **```net-device.h```**
   - ```ipAddr```**:** NetDevice is installed on each node. This attribute stores the ipv4 address associated with each node.<br /><br />

- **```traffic-control-layer.h```**
   - ```pccpQueue```**:** If congestion degree is above a certain threshold at parent node, all the packets sent by the current node is pushed to this queue. Later on, the queued packets are dequeued one by one and sent to their destination.<br /><br />

- **```traffic-control-layer.cc```** - Traffic Control Layer controls the congestion by maintaining a queue. It sits between the Network layer and the MAC layer. 
   - ```ScheduleRate()```**:** It adjusts the scheduling rate associated with each node based on the congestion degree, the number of active offsprings of the parent node and the mean packet service time of the node.
   - ```SrcRate()```**:** The rate at which a node is allowed to transmit packet to its parent node. It depends on ScheduleRate, Source traffic priority(SP), Global Priority(GP).


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








