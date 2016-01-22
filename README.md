# distance-vector-sim
simulation using dv 

This program reads in a pre generated text file, such as topology1.txt, and creates a network (topology) to find the most cost effective route.

The data structure is set up using an array for optimal performance. Each array element consists of a node struct. Each element index corresponds to its node number. eg. array[0] = node 0. Each node contains a map structure, which where each key also corresponds to each node in the network. 

After the network is initialized, there is a dvPacket packet that is sent off to that nodes neighbors( a dvPacket consists of all the routes known to that node). The dvPackets are sent depending on their propogation time delay, which is organized precicely in another array called edgeInfo.

The node that receives a dvPacket compares with its known routes, and decides if the advertised cost from the sender is better. If the cost is better, then it changes its route to that of the sender, if there is no route, then it adds that route by default.

little hiccups
-----------------------
the routes are not always the best when testing using any node failures, most noticeably when the network is larger(I've only tested it with 30 nodes in a network).

the initial times are very accurate for triggered dvPackets. However, with distance vector there is a periodic exchange of dvPackets after a while. I would say that the time does lose some accuracy, but is still good enough to work as an actual predictor. 

the linkFailure case is something to work on



