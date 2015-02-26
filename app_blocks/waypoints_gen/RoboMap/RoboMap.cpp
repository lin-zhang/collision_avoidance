/*
 * RoboMap.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: zhanglin
 */

#include "RoboMap.h"


namespace RM {

//MapReg::MapReg(RoboMap *M, int id){
//	Map=M;
//	map_id=id;
//}
//int MapReg::getMapID(){
//	return map_id;
//}
//
//MapReg::~MapReg(){
//	Map=NULL;
//	map_id=0;
//}


RoboMap::RoboMap() {
	// TODO Auto-generated constructor stub
	//mapNumber++;
	//mapID++;
	map_data=new char[DEFAULT_MAP_W*DEFAULT_MAP_H]();
	map_width=DEFAULT_MAP_W;
	map_height=DEFAULT_MAP_H;
	//map_id = mapID;
	//map_id=mapNumber;
	nObstacles=0;

//	MapRegistry.push_back(MapReg(this, map_id));
}

RoboMap::RoboMap(int width, int height) {
	// TODO Auto-generated constructor stub
	//mapNumber++;
	//mapID++;
	map_data=new char[width*height]();
	map_width=width;
	map_height=height;
	//map_id = mapID;
        //map_id=mapNumber;
	nObstacles=0;
//	RoboMap Me=*this;
//	MapRegistry.push_back(MapReg(this, map_id));
}

RoboMap::~RoboMap() {
	// TODO Auto-generated destructor stub
	if (NULL != map_data){
	//mapNumber--;
	delete []map_data;
	map_data=NULL;

//	MapReg int_to_remove;
//	MapRegistry.erase(std::remove(MapRegistry.begin(), MapRegistry.end(), int_to_remove), MapRegistry.end());

//	for (std::vector<int>::iterator it = MapRegistry.begin() ; it != MapRegistry.end(); ++it){
//		if(MapRegistry[it])
//		MapRegistry.push_back(MapReg(this, map_id));
//	}
	}
}

void RoboMap::mark_point(int x, int y){
	if(x>=0&&x<map_width&&y>=0&&y<map_height)
	map_data[x+y*map_width]|=0xFF;
}

void RoboMap::mark_point(Point2D p){
	if(p.getX()>=0&&p.getX()<map_width&&p.getY()>=0&&p.getY()<map_height)
	map_data[(unsigned int)p.getX()+(unsigned int)p.getY()*map_width]|=0xFF;
//	if(map_data[(unsigned int)p.getX()+(unsigned int)p.getY()*map_width]<0x40)
//		map_data[(unsigned int)p.getX()+(unsigned int)p.getY()*map_width]+=0x10;
//	else
//		map_data[(unsigned int)p.getX()+(unsigned int)p.getY()*map_width]=0xC0;
}

void RoboMap::unmark_point(int x, int y){
	if(x>=0&&x<map_width&&y>=0&&y<map_height)
	map_data[x+y*map_width]&=0x00;
}
void RoboMap::unmark_point(Point2D p){
	if(p.getX()>=0&&p.getX()<map_width&&p.getY()>=0&&p.getY()<map_height)
	map_data[(unsigned int)p.getX()+(unsigned int)p.getY()*map_width]&=0x00;
//	if(map_data[(unsigned int)p.getX()+(unsigned int)p.getY()*map_width]>0x05)
//		map_data[(unsigned int)p.getX()+(unsigned int)p.getY()*map_width]-=0x05;
}

void RoboMap::showMap(){
	if(NULL==map_data){
		cout<<"[WARN][MAP] Map not initialized."<<endl;
		return;
	}
	cout<<"Map<"<<map_id<<"> width by height"<<"("<<map_width<<","<<map_height<<")"<<endl;
	for(int i=0;i<map_height;i++){
		for(int j=0;j<map_width;j++){
			printf("%3d",map_data[j+i*map_width]);
		}
		printf("\n");
	}
}

void RoboMap::placeRobot(RoboMe &robot, float x, float y){
	//place a robot from the map
	if(robot.getMapID()){
		cout<<"[WARN][PLACE] Robot ID "<<robot.getID()<<" is already on map "<<robot.getMapID()<<", abort."<<endl;
		return;
	}

	if(x>=robot.getWidth()/2&&x<=map_width-robot.getWidth()/2
			&&y>=robot.getHeight()/2&&y<=map_height-robot.getHeight()/2){
		robot.isOnMap(map_id);
//		printf("%d,%d,%d\n",mapNumber,mapID,robot.getMapID());
	robot.setPosX(x);
	robot.setPosY(y);
//	robot.getBox().showBox();
#ifdef DEBUG
	cout<<"[INFO][PLACE] Robot ID "<<robot.getID()<<" has been placed on map "<<robot.getMapID()<<endl;
#endif
	mark_point(robot.getPos());
	mark_point(robot.getPos().getX()+robot.getWidth()/2, robot.getPos().getY()+robot.getHeight()/2);
	mark_point(robot.getPos().getX()+robot.getWidth()/2, robot.getPos().getY()-robot.getHeight()/2);
	mark_point(robot.getPos().getX()-robot.getWidth()/2, robot.getPos().getY()+robot.getHeight()/2);
	mark_point(robot.getPos().getX()-robot.getWidth()/2, robot.getPos().getY()-robot.getHeight()/2);
	}
#ifdef DEBUG
	else
		cout<<"[WARN][PLACE] Robot ID "<<robot.getID()<<" can NOT be placed on map "<<robot.getMapID()<<", not enough space."<<endl;
#endif

}

void RoboMap::removeRobot(RoboMe &robot){
	//remove a robot from the map
	if(robot.getMapID()!=map_id){
		cout<<"[WARN][REMOVE] Robot ID "<<robot.getID()<<" is not on map "<<map_id<<", abort."<<endl;
		return;
	}
	unmark_point(robot.getPos());
	unmark_point(robot.getPos().getX()+robot.getWidth()/2, robot.getPos().getY()+robot.getHeight()/2);
	unmark_point(robot.getPos().getX()+robot.getWidth()/2, robot.getPos().getY()-robot.getHeight()/2);
	unmark_point(robot.getPos().getX()-robot.getWidth()/2, robot.getPos().getY()+robot.getHeight()/2);
	unmark_point(robot.getPos().getX()-robot.getWidth()/2, robot.getPos().getY()-robot.getHeight()/2);
	robot.isOnMap(NOT_ON_MAP);
//	robot.setPosX(0);
//	robot.setPosY(0);
#ifdef DEBUG
	cout<<"[INFO][REMOVE] Robot ID "<<robot.getID()<<" has been removed from map "<<map_id<<endl;
#endif
}

void RoboMap::placeObstacle(Point2D pObs){
	if(nObstacles>MAX_OBS-1)
		cout<<"[WARN][PLACE] Max number of obstacles is set to "<<MAX_OBS<<", abort."<<endl;
	else if(pObs.getX()==0||pObs.getY()==0){
		cout<<"[WARN][PLACE] obstacle can NOT be placed at ("<<pObs.getX()<<","<<pObs.getY()<<")."<<endl;
	}
	else{
		Obs_coord[nObstacles].setX(pObs.getX());
		Obs_coord[nObstacles].setY(pObs.getY());
		nObstacles++;
		mark_point(pObs);
		cout<<"[INFO][PLACE] obstacle placed at ("<<pObs.getX()<<","<<pObs.getY()<<")."<<endl;

	}
}

void RoboMap::removeObstacle(Point2D pObs){
	if(nObstacles<1)
		cout<<"[WARN][REMOVE] There isn't any obstacle on the map, abort."<<endl;
	else{
		Point2D tempObs_coord[MAX_OBS];
		int tempCoordID=0;
		for(int i=0;i<nObstacles;i++){
			if(Obs_coord[i].getX()==pObs.getX()&&Obs_coord[i].getY()==pObs.getY())
			{
				Obs_coord[i].setX(-1);
				Obs_coord[i].setY(-1);
				nObstacles--;
			}
			else{
				tempObs_coord[tempCoordID].setX(Obs_coord[i].getX());
				tempObs_coord[tempCoordID].setY(Obs_coord[i].getY());
				tempCoordID++;
				unmark_point(pObs);
				cout<<"[INFO][REMOVE] obstacle removed at ("<<pObs.getX()<<","<<pObs.getY()<<")."<<endl;
			}
		}
		for(int i=0;i<nObstacles;i++){
			Obs_coord[i].setX(tempObs_coord[i].getX());
			Obs_coord[i].setY(tempObs_coord[i].getY());
		}
	}
}

Point2D* RoboMap::getObstacles(){
	return Obs_coord;
}

void Point2D::print(const char* name){
	printf("%s: (%6.3f,%6.3f)\n", name, x, y);
}

int isRobotInMap(RoboMe robot){
return 0;
}


RoboMe::RoboMe() {
	// TODO Auto-generated constructor stub

}

RoboMe::RoboMe(float W, float H){
	//NumberOfDefinedRobots++;
	//robotID++;
	width=W;
	height=H;
	map_id=0;
	posX=0;
	posY=0;
	//robot_id=robotID;
	robot_id=0;
	M=RoboBox(width, height);
	Map=NULL;
}

Point2D RoboMe::getPos(){
	return Point2D(posX, posY);
};

Point2D RoboMe::getGoal(){
	return robot_goal;
}
void RoboMe::setGoal(Point2D goal){
	robot_goal.setX(goal.getX());
	robot_goal.setY(goal.getY());
}

float RoboMe::getPosX(){
	return posX;
}
float RoboMe::getPosY(){
	return posY;
}
int RoboMe::getMapID(){
	return map_id;
}
void RoboMe::setPosX(float x){
	posX=x;
}
void RoboMe::setPosY(float y){
	posY=y;
}
float RoboMe::getWidth(){
	return width;
}
float RoboMe::getHeight(){
	return height;
}
int RoboMe::getID(){
	return robot_id;
}
RoboBox RoboMe::getBox(){
	return M;
}


RoboMe::~RoboMe() {
	// TODO Auto-generated destructor stub
	//NumberOfDefinedRobots--;
}

void RoboMe::isOnMap(int new_id){
	map_id=new_id;
//	printf("robot map_id %d\n",map_id);
}

void RoboMe::showStatus(){
	printf("\n\
			robot_id: 	%8d\n\
			width:		%6.2f\n\
			height:		%6.2f\n\
			position:\t      (%6.2f,%6.2f)\n\
			map_id:		%8d\n", robot_id,width,height,posX,posY,map_id);
	printf("robot map_id %d\n",map_id);
}


void RoboMe::moveTo(int direction, float distance, RoboMap &robomap){
	if(getMapID()!=robomap.getMapID()){
		cout<<"[WARN][ROBOT] <Robot "<<robot_id<<"> is NOT on <Map "<<map_id<<">, abort."<<endl;
	}

	int prev_posX=posX;
	int prev_posY=posY;
	robomap.removeRobot(*this);
	switch(direction){
	case DIR_N:
		posX=prev_posX;
		posY-=distance;
		break;
	case DIR_E:
		posX+=distance;
		posY=prev_posY;
		break;
	case DIR_S:
		posX=prev_posX;
		posY+=distance;
		break;
	case DIR_W:
		posX-=distance;
		posY=prev_posY;
		break;
	}
	if(posX>=width/2&&posX<=robomap.getMapWidth()-width/2
			&&posY>=height/2&&posY<=robomap.getMapHeight()-height/2){
#ifdef DEBUG
		cout<<"[INFO][ROBOT-MOVE] <Robot "<<robot_id<<"> moved on <Map "<<map_id<<"> from ("<<prev_posX<<","<<prev_posY<<") to ("<<posX<<","<<posY<<")"<<endl;
#endif
	}
	else{
#ifdef DEBUG
		cout<<"[WARN][ROBOT-MOVE] <Robot "<<robot_id<<"> on <Map "<<map_id<<"> can NOT move from ("<<prev_posX<<","<<prev_posY<<") to ("<<posX<<","<<posY<<") due to size limit."<<endl;
#endif
		posX=prev_posX;
		posY=prev_posY;
	}
	robomap.placeRobot(*this, posX, posY);
}
void RoboMe::moveTo(Point2D dest, RoboMap& robomap){
	int prev_posX=posX;
	int prev_posY=posY;
	robomap.removeRobot(*this);
	posX=dest.getX();
	posY=dest.getY();
	if(posX>=width/2&&posX<=robomap.getMapWidth()-width/2
			&&posY>=height/2&&posY<=robomap.getMapHeight()-height/2){
#ifdef DEBUG
		cout<<"[INFO][ROBOT-MOVE] <Robot "<<robot_id<<"> moved on <Map "<<map_id<<"> from ("<<prev_posX<<","<<prev_posY<<") to ("<<posX<<","<<posY<<")"<<endl;
#endif
	}
	else{
#ifdef DEBUG
		cout<<"[WARN][ROBOT-MOVE] <Robot "<<robot_id<<"> on <Map "<<map_id<<"> can NOT move from ("<<prev_posX<<","<<prev_posY<<") to ("<<posX<<","<<posY<<") due to size limit."<<endl;
#endif
		posX=prev_posX;
		posY=prev_posY;
	}
	robomap.placeRobot(*this, posX, posY);
#ifdef DEBUG
	cout<<"[INFO][ROBOT] <Robot "<<robot_id<<"> moved on <Map "<<map_id<<"> from ("<<prev_posX<<","<<prev_posY<<") to ("<<posX<<","<<posY<<")"<<endl;
#endif
}

//RoboMap& RoboMe::getMap(){
//
//}

int RoboMap::getMapID(){
	return map_id;
}

int RoboMap::getMapWidth(){
	return map_width;
}

int RoboMap::getMapHeight(){
	return map_height;
}
char* RoboMap::getData(){
	return map_data;
};


RoboBox::RoboBox(){

}
RoboBox::RoboBox(Point2D pA, Point2D pB){
	char Quadrant=0x00;

	if (pA.isCollinear(pB)){
		invalid=1;
		cout<<"not a valid box!"<<endl;
		return;
	}
	invalid=0;
	if (pA.getX()>pB.getX()){
		Quadrant|=0x02;
		width=pA.getX()-pB.getX();
	}
	else{
		Quadrant|=0x00;
		width=pB.getX()-pA.getX();
	}

	if (pB.getY()<pA.getY()){
		Quadrant|=0x01;
		height=pA.getY()-pB.getY();
	}
	else{
		Quadrant|=0x00;
		height=pB.getY()-pA.getY();
	}

//
switch((int)Quadrant){
case 0: // first Quadrant
	p[0]=pB;
	p[1]=Point2D(pA.getX(),pB.getY());
	p[2]=Point2D(pB.getX(),pA.getY());
	p[3]=pA;
	break;
case 1: // fourth Quadrant
	p[0]=Point2D(pB.getX(),pA.getY());
	p[1]=pB;
	p[2]=pA;
	p[3]=Point2D(pA.getX(),pB.getY());
	break;
case 2: // second Quadrant
	p[0]=Point2D(pA.getX(),pB.getY());
	p[1]=pA;
	p[2]=pB;
	p[3]=Point2D(pB.getX(),pA.getY());
	break;
case 3: // third Quadrant
	p[0]=pA;
	p[1]=Point2D(pA.getX(),pB.getY());
	p[2]=Point2D(pB.getX(),pA.getY());
	p[3]=pB;
	break;
}

//DEBUG
	cout<<"case "<<(int)Quadrant<<""<<endl;

}

RoboBox::RoboBox(float W, float H){
	width=W;
	height=H;
	invalid=0;
	p[0]=Point2D(W,H);
	p[1]=Point2D(W,0);
	p[2]=Point2D(0,H);
	p[3]=Point2D(0,0);
}

RoboBox::~RoboBox(){

}

void RoboBox::showBox(){
	if(invalid) return;
		cout<<"box size: ("<<width<<" by "<<height<<") width by height"<<endl;
//		cout<<"("<<p[2].getX()<<","<<p[2].getY()<<")";
//		cout<<"("<<p[0].getX()<<","<<p[0].getY()<<")"<<endl;
//		cout<<"("<<p[3].getX()<<","<<p[3].getY()<<")";
//		cout<<"("<<p[1].getX()<<","<<p[1].getY()<<")"<<endl;
		printf("(%6.2f,%6.2f)\t(%6.2f,%6.2f)\n",p[2].getX(),p[2].getY(),p[0].getX(),p[0].getY());
		printf("(%6.2f,%6.2f)\t(%6.2f,%6.2f)\n",p[3].getX(),p[3].getY(),p[1].getX(),p[1].getY());
}

Point2D* RoboBox::getVertex(){
	return p;
}

Point2D::Point2D(){
}

Point2D::~Point2D(){
}

Point2D::Point2D(int m, int n){
	x=m;y=n;
}

float Point2D::getX(){
	return x;
}

float Point2D::getY(){
	return y;

}

void Point2D::setX(float x_val){
	x=x_val;
}

void Point2D::setY(float y_val){
	y=y_val;
}

int Point2D::isCollinear(Point2D p){
	return (x==p.x||y==p.y);
}

Line2D::Line2D(){

}

Line2D::Line2D(Point2D A, Point2D B){
	na=A.getY()-B.getY();
	nb=B.getX()-A.getX();
	nc=-nb*A.getY()-na*A.getX();
}

Line2D::~Line2D(){

}

} /* namespace RM */
