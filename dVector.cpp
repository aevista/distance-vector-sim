/*
	Things to work on 
	-----------------  
	user input for event handling
	tidy everything up
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>

using namespace std;

#define INFINITY -1

enum event{dvSend, dvReceive, linkFailure, nodeFailure, DataPsend};

enum connection{down, up};

enum update{periodic, triggered};

bool eventHandler();
void Failure(int *nodeFail, int *failTime, const unsigned int size);
void dataPacket(int *beg, int *end, const unsigned int size);

struct reporting{
	reporting():convergeTime(INFINITY), reason(triggered), status(up){}

	double convergeTime;
	update reason;
	connection status;
};

struct vertici{
	
	int id; 
	double relativeTime;
};

struct edge{
	edge(int cost, double pdtime, connection status): 
	cost(cost), pdtime(pdtime), status(status){}

	vertici v1;
	vertici v2;
	int cost;
	double pdtime;
	connection status;

};

struct destination{
	destination(int cost, int nh): cost(cost), nh(nh){}
	destination(){}

	int cost;
	int nh;
};

struct node{
	node(int id): id(id){}
	node(){}

	int id;
	map <int, destination> dest;

	reporting report;
};

struct dvPacket{
	dvPacket(int dest, int edgecost): dest(dest), edgecost(edgecost){}
	//split horizon 
	//&& doesn't send routing information to a node on how to get to itself
	dvPacket& operator=(const node* const rhs){
		src.id = rhs->id;
		for(map<int,destination>::const_iterator j = rhs->dest.begin(); j != rhs->dest.end(); j++){
			if((j->second.nh != dest) && (j->first != dest))
				src.dest[j->first] = destination(j->second.cost, j->second.nh);
		}
		return *this;
	}

	int dest;
	int edgecost;
	node src;
};

int main(int argc, char *argv[]){


	int v1, v2, cost, maxv = 0;
	double pdtime, totalTime, elapsedTime = 0;

	vector <edge> edgeInfo;
	vector <node> router;
	queue <dvPacket> dv;

	//command line read in
	if (argc !=3 ){ cerr <<"Must contain topology file and totalTime" << endl; exit (-1); }
	ifstream myfile(argv[1]);
	if ((totalTime = strtol(argv[2], NULL, 10)) < 0){ cerr<<"Time must be at least 0" << endl; exit(-1);};


	//storing topology information
	if(myfile.is_open()){
		while(myfile){

			myfile >> v1;
			myfile >> v2;
			myfile >> cost;
			myfile >> pdtime;

			//creating distinct edges
			edge info(cost, pdtime, up);
			info.v1.id = v1;
			info.v1.relativeTime = pdtime;
			info.v2.id = v2;
			info.v2.relativeTime = pdtime;

			edgeInfo.push_back(info);

			//finding max vertici
			if(maxv < v1)
				maxv = v1;
			if(maxv < v2)
				maxv = v2;

		}
	}
	else{ cerr <<"Cannot open topology file" << endl; exit (-1);}

	//allocating routers from topology
	for(int i = 0; i <= maxv; i++){
		node rtr(i);
		router.push_back(rtr);
	}

	//initializing routing table
	for(unsigned int i = 0; i < edgeInfo.size(); i++){

		v1 = edgeInfo[i].v1.id;
		v2 = edgeInfo[i].v2.id;
		cost = edgeInfo[i].cost;

		router.at(v1).dest[v2] = destination(cost, v2);
		router.at(v2).dest[v1] = destination(cost, v1);
	}

	//printing routing table
	for(unsigned int i = 0; i < router.size(); i++){
		cout << "\n\nRouter" << router[i].id << endl;
		cout <<"Dest ---- Cost ---- Next Hop" << endl;
		for(map<int, destination>::const_iterator j = router[i].dest.begin(); j != router[i].dest.end(); j++){
			cout.width(8); cout << left << j->first;
			cout.width(6); cout << internal << j->second.cost;
			cout.width(12); cout << right << j->second.nh << endl;
		}
	}

	//events
	event ev = dvSend;

	//event variables
	int src, dest, dvTo, dvFrom, advertisedCost, totalCost, dvDest, edgenum, edgeCost;
	double shortestTime;
	bool completionTime = false;

	//eventHandler variables
	int nodeFail, failTime, beg, end;
	bool flag = false;

	if(eventHandler())
		Failure(&nodeFail, &failTime, router.size());
	dataPacket(&beg, &end, router.size());

	while(!completionTime){
		switch(ev){
			case dvSend:{
				
					//default variable values 
					edgenum = 0;
					shortestTime = edgeInfo.at(edgenum).v1.relativeTime;
					src = edgeInfo.at(edgenum).v1.id;
					dest = edgeInfo.at(edgenum).v2.id;
					edgeCost = edgeInfo.at(edgenum).cost;
					//sending dvPacket of vertici with smallest time from src to dest
					for(unsigned int i = 0; i < edgeInfo.size(); i++){
						if(shortestTime > edgeInfo.at(i).v1.relativeTime){
							edgenum = i;
							shortestTime = edgeInfo.at(edgenum).v1.relativeTime;
							src = edgeInfo.at(edgenum).v1.id;
							dest = edgeInfo.at(edgenum).v2.id;
							edgeCost = edgeInfo.at(edgenum).cost;

						}
						if(shortestTime > edgeInfo.at(i).v2.relativeTime){
							edgenum = i;
							shortestTime = edgeInfo.at(edgenum).v2.relativeTime;
							src = edgeInfo.at(edgenum).v2.id;
							dest = edgeInfo.at(edgenum).v1.id;
							edgeCost = edgeInfo.at(edgenum).cost;
						}
					}
					//if(edgeInfo.at(edgenum).status == up){
						dvPacket DV(dest, edgeCost);
						DV = &router[src];
						dv.push(DV);
					//}
	
					//updating time
					if(router.at(src).report.reason == triggered){//triggered
						if(src == edgeInfo.at(edgenum).v1.id){ //src == v1
							elapsedTime = edgeInfo.at(edgenum).v1.relativeTime; 
							edgeInfo.at(edgenum).v1.relativeTime += edgeInfo.at(edgenum).pdtime; 
						}
						else{//src == v2
							elapsedTime = edgeInfo.at(edgenum).v2.relativeTime; 
							edgeInfo.at(edgenum).v2.relativeTime += edgeInfo.at(edgenum).pdtime;
						}
					}
					else{// periodic
						if(src == edgeInfo.at(edgenum).v1.id){//src == v1
							elapsedTime = edgeInfo.at(edgenum).v1.relativeTime; 
							edgeInfo.at(edgenum).v1.relativeTime += edgeInfo.at(edgenum).pdtime + 1;
						}
						else {//src == v2 
							elapsedTime = edgeInfo.at(edgenum).v2.relativeTime; 
							edgeInfo.at(edgenum).v2.relativeTime += edgeInfo.at(edgenum).pdtime + 1;
						}
					}
					ev = dvReceive;
				}
				break;

			case dvReceive:{
					
					if(!dv.empty()){
						dvTo = dv.front().dest;
						dvFrom = dv.front().src.id;
						edgeCost = dv.front().edgecost;

						router.at(dvTo).report.reason = periodic;

						//looking in dvPacket
						for(map<int, destination>::const_iterator j = dv.front().src.dest.begin(); j != dv.front().src.dest.end(); j++){
	
							advertisedCost = j->second.cost;
							totalCost = advertisedCost + edgeCost;
							dvDest = j->first;

							//checking if element exists 
							if(router.at(dvTo).dest.size() != router.size() - 1){
								map<int, destination>::const_iterator it = router.at(dvTo).dest.find(dvDest);
								if(it ==  router.at(dvTo).dest.end())
									router.at(dvTo).dest[dvDest] = destination(totalCost, dvFrom);
								router.at(dvTo).report.reason = triggered;
								//checking convergence
								if(router.at(dvTo).dest.size() == router.size() - 1) 
									router.at(dvTo).report.convergeTime = elapsedTime;
							}
							//comparing advertised 
							if(router.at(dvFrom).report.status == down){
								router.at(dvTo).dest[dvFrom].cost = INFINITY; 
								router.at(dvTo).dest[dvFrom].nh = INFINITY;

								if(router.at(dvTo).dest[dvDest].nh == dvFrom){
									router.at(dvTo).dest[dvDest].cost = INFINITY;
									router.at(dvTo).dest[dvDest].nh = INFINITY;
								}
								router.at(dvTo).report.reason = triggered;
							}
							else if(router.at(dvFrom).report.status == up){
								if(advertisedCost == INFINITY){
									router.at(dvTo).dest[dvDest].cost = INFINITY;
									router.at(dvTo).dest[dvDest].nh = INFINITY;
									router.at(dvTo).report.reason = triggered;
								}
								else if(advertisedCost != INFINITY){
									if(INFINITY == router.at(dvTo).dest[dvFrom].cost || edgeCost < router.at(dvTo).dest[dvFrom].cost){
										router.at(dvTo).dest[dvFrom].cost = edgeCost;
										router.at(dvTo).dest[dvFrom].nh = dvFrom;
										router.at(dvTo).report.reason = triggered;
									}
									else if( INFINITY == router.at(dvTo).dest[dvDest].cost || totalCost < router.at(dvTo).dest[dvDest].cost){
										router.at(dvTo).dest[dvDest].cost = totalCost;
										router.at(dvTo).dest[dvDest].nh = dvFrom;
										router.at(dvTo).report.reason = triggered;
									}	
								}
							}
						}
						dv.pop();
						ev = dvSend;
						if(failTime == (int)elapsedTime && flag == false){
							ev = nodeFailure;
							flag = true;

						}
					}
					if(elapsedTime >= totalTime)
						ev = DataPsend;
						
				}
				break;

			//case linkFailure:{
			//			edgeInfo.at(1).status = down;
			//			ev = dvSend;
			//	}
				break;

			case nodeFailure:{
					router.at(nodeFail).report.status = down;
					ev = dvSend;
				}
				break;

			case DataPsend:{
					cout << "\nData Packet sent from router " << beg << " to router " << end << endl;
					cout << "router " << router.at(beg).id << " ";
					while(router.at(beg).id != end){
						cout << " >> router " << router.at(beg).dest[end].nh;
						beg = router.at(beg).dest[end].nh;
					}
				completionTime = true;
				
				}
				break;
		}
		
	}

	int lastToconverge = 0;
	double tempTime = 0;
	//printing routing table
	ofstream output;
	output.open("output.txt");

	for(unsigned int i = 0; i < router.size(); i++){
		output << "\n\nRouter" << router.at(i).id << " converged at " << router.at(i).report.convergeTime << endl;
		if(router.at(i).report.status == down)
			output <<"Router " << i << " is down" << endl;
		//finding last router to converge
		if(tempTime < router.at(i).report.convergeTime){
			lastToconverge = router.at(i).id;
			tempTime = router.at(i).report.convergeTime;
		}
		output <<"Dest ---- Cost ---- Next Hop" << endl;
		for(map<int, destination>::const_iterator j = router.at(i).dest.begin(); j != router.at(i).dest.end(); j++){
			output.width(8); output << left << j->first;
			output.width(6); output << internal << j->second.cost;
			output.width(12); output << right << j->second.nh << endl;
		}
	}
	output.close();
	cout << "\n\nNetwork converged at " << router.at(lastToconverge).report.convergeTime << endl; 

	return 0;

}

bool eventHandler(){
	char response;
	cout <<"\nTest for node failure?(y/n)" << endl;
	cin >> response;
	if(response == 'y' || response == 'Y')
		return true;
	return false; 
}

void Failure(int *nodeFail, int *failTime, const unsigned int size){
	do { cout <<"What node Fails?" << endl;
		cin >> *nodeFail;
	}while( *nodeFail >= (int)size);

	do {
	cout << "Node Failes at what time?" << endl;
	cin >> *failTime;
	}while(*failTime < 0);
}

void dataPacket(int *beg, int *end, const unsigned int size){
	do {
		cout <<"What router sends the data packet?" << endl;
		cin >> *beg;
	}while(*beg >= (int)size);

	do{
		cout <<"Where is the data packet sent to?" << endl;
		cin >> *end;
	}while(*end >= (int)size);
}
