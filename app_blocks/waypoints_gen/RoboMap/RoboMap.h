/*
 * RoboMap.h
 *
 *  Created on: Nov 28, 2014
 *      Author: zhanglin
 */
#include <cstdio>
#include <iostream>
#include <vector>
#include <iomanip>
#include <iterator>
#include <stdlib.h>
#include <unistd.h>
//#include <opencv/cv.h>
//#include <opencv/highgui.h>
#include <vector>
#include <float.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <math.h>
#ifndef ROBOMAP_H_
#define ROBOMAP_H_

#define DEFAULT_MAP_W 20
#define DEFAULT_MAP_H 20

#define MAX_OBS 12
#define MAX_SAT 4

#define DIR_N 0
#define DIR_E 1
#define DIR_S 2
#define DIR_W 3

#define NOT_ON_MAP 0
using namespace std;
namespace RM {
class RoboMap;
class RoboBox;
class RoboMe;
class Point2D;
class MapReg;
class Line2D;
//class MapReg{
//public:
//	MapReg();
//	MapReg(RoboMap *M, int id);
//	int getMapID();
//	~MapReg();
//private:
//	RoboMap *Map;
//	int map_id;
//};

class Point2D{
public:
	Point2D();
	Point2D(int m, int n);
	~Point2D();
	float getX();
	float getY();
	void setX(float x_val);
	void setY(float y_val);
	int isCollinear(Point2D p);
	void print(const char* name);

private:
	float x;
	float y;
};

class Line2D{
public:
	Line2D();
	Line2D(float a, float b, float c){na=a;nb=b;nc=c;};
	Line2D(Point2D A, Point2D B);
	float getA(){return na;};
	float getB(){return nb;};
	float getC(){return nc;};
	~Line2D();
private:
	float na;
	float nb;
	float nc;
};

class RoboBox{
public:
	RoboBox();
	RoboBox(Point2D pA, Point2D pB);
	RoboBox(float W, float H);
	virtual ~RoboBox();
	void showBox();
	Point2D* getVertex();

private:
	Point2D p[4];
	float width;
	float height;
	int invalid;
};

class RoboMe {
public:
	RoboMe();
	RoboMe(float W, float H);
	virtual ~RoboMe();
	void isOnMap(int map_id);
	void showStatus();
	Point2D getPos();
	void setGoal(Point2D goal);
	Point2D getGoal();
	float getPosX();
	float getPosY();
	int getMapID();
	void setPosX(float x);
	void setPosY(float y);
	float getWidth();
	float getHeight();
	int getID();
	RoboMap& getMap();
	RoboBox getBox();
	void moveTo(int direction, float distance, RoboMap &robomap);
	void moveTo(Point2D dest, RoboMap &robomap);
private:
		float width;
		float height;
		float posX;
		float posY;
		int robot_id;
		//static int NumberOfDefinedRobots;
		//static int robotID;
		int map_id;
		RoboBox M;
		float orientation;
		RoboMap* Map;
		Point2D robot_goal;

};

class RoboMap {
public:
	RoboMap();
	RoboMap(int width, int height);
	virtual ~RoboMap();
	void mark_point(int x, int y);
	void unmark_point(int x, int y);
	void mark_point(Point2D p);
	void unmark_point(Point2D p);
	void printMapSize();
	void placeRobot(RoboMe &robot, float x, float y);
	void removeRobot(RoboMe &robot);
	int isRobotInMap(RoboMe robot, int x, int y);
	void showMap();
	int getMapID();
	int getMapWidth();
	int getMapHeight();
	char* getData();
	void placeObstacle(Point2D pObs);
	void removeObstacle(Point2D pObs);
	Point2D* getObstacles();

private:
		char *map_data;
		int map_width;
		int map_height;
		int map_id;
		int superBlockWidth;
		int superBlockHeight;
		static int mapNumber;
		//static int mapID;
		static int robotNumber;
		RoboMap* prev;
		RoboMap* next;
		static vector<MapReg> MapRegistry;
		int registry_index;
		int nObstacles;
		Point2D Obs_coord[MAX_OBS];
};




} /* namespace RM */

#endif /* ROBOMAP_H_ */
