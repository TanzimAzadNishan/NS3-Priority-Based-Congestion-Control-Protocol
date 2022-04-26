# Priority-Based-Congestion-Control-Protocol(PCCP)

Implemented in ns-3 for **IEEE 802.11 network**.

```diff
+ The algorithm works in the Network layer to control upstream congestion.
```


## **Project Overview**

```
1. Nodes may have different priority due to their function or location and need to gain different throughput. 
   So the algorithm sets priority to each node.

2. The output rate of a node is controlled by adjusting the scheduling rate which depends on the priority.

3. PCCP maintains a weighted fairness so that the sink can get different(but in a weighted fair way) throughput
   from its child sensor nodes.

4. PCCP measures congestion level(degree) for each node by calculating packet inter-arrival time & packet service 
   time which reflects the present congestion condition.

5. Any node(sink) can inform its child nodes to change transmission rate according to the congestion level.
```



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


**coverageSide means one side of coverage area**
> With the increase in flows, the network is getting more and more congested which reduces the throughput.



### **Normalized Weighted Throughput**
<p align="center">
   <img src="https://github.com/TanzimAzadNishan/Priority-Based-Congestion-Control-Protocol/blob/main/Normalized_Throughput.png" 
        width="400" height="300"/>
</p>


**coverageSide means one side of coverage area**
> When we take the normalized value of the throughput, we see the change in value is very small with the increase in flow. It almost remains constant.




## **Run the Project**

### Installation
Download the latest version of ns-3. ***ns-3.35*** is used in this project.
Extract the tar.bz2 file. You will get a ns-allinone-3.xx folder, which will have these files and directories.

Then enter the following command:
> ./build.py --enable-examples --enable-tests


Now go to the ns-3.xx directory. Then run the commands:
```
./waf clean
./waf configure --build-profile=debug --enable-examples --enable-tests
```


### Modification

Find the files in ***pccp*** folder and copy those in their respective folders mentioned below.

```
1) src/internet/model/
   a) node-option.h
   b) node-option.cc

2) src/network/model
   a) node.h
   b) node.cc

3) src/traffic-control/model
   a) queue-disc.cc
   b) queue-disc.h
   c) traffic-control-layer.cc
   d) traffic-control-layer.h

4) build/ns3/net-device.h

5) src/internet/wscript
```


### Run the Simulation File

> ./waf --run "scratch/pccpTest --nFlows=20" --cwd="Output/pccpTest"


In console, you will see the logs of the simulation. The graphs will be stored in ***Output/pccpTest*** folder.












