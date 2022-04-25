# Priority-Based-Congestion-Control-Protocol(PCCP)

Implemented for IEEE 802.11 network.

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
![Topology](https://github.com/TanzimAzadNishan/Priority-Based-Congestion-Control-Protocol/blob/main/Topology.png =500x300)

