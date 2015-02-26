#include "waypoints_func.h"

float arccosLines(Point2D A, Point2D B, Point2D C, Point2D D){
	Point2D Dprime(D.getX()+(A.getX()-C.getX()),(D.getY()+(A.getY()-C.getY())));
	double va_x = B.getX() - A.getX();
	double va_y = B.getY() - A.getY();

	// 向量b的(x, y)坐标
	double vb_x = Dprime.getX() - A.getX();
	double vb_y = Dprime.getY() - A.getY();

	double productValue = (va_x * vb_x) + (va_y * vb_y);  // 向量的乘积
	double va_val = sqrt(va_x * va_x + va_y * va_y);  // 向量a的模
	double vb_val = sqrt(vb_x * vb_x + vb_y * vb_y);  // 向量b的模
	double cosValue = productValue / (va_val * vb_val);      // 余弦公式

	// acos的输入参数范围必须在[-1, 1]之间，否则会"domain error"
	// 对输入参数作校验和处理
	if(cosValue < -1 && cosValue > -2)
	 cosValue = -1;
	else if(cosValue > 1 && cosValue < 2)
	 cosValue = 1;

	// acos返回的是弧度值，转换为角度值
//	float angle=0.0;
//	angle = acos(cosValue) * 180 / M_PI;
//	printf("angle=%8.3f\n", angle);
	return cosValue;
}

float distPoints(Point2D A, Point2D B){
	return sqrt((A.getX()-B.getX())*(A.getX()-B.getX())+(A.getY()-B.getY())*(A.getY()-B.getY()));
}

float distPointToLine(Point2D P, Line2D L){
	float dist;
	dist=fabs(L.getA()*P.getX()+L.getB()*P.getY()+L.getC())/sqrt(L.getA()*L.getA()+L.getB()*L.getB());
	return dist;
}

Point2D PedalCoordinates(Point2D P, Line2D L){
		float m=(L.getA()*L.getA()+L.getB()*L.getB());
		float x;
		float y;
		if(m==0) {return P;}
		else{
			x= (
				 L.getB()*L.getB()*P.getX()
				-L.getA()*L.getB()*P.getY()
				-L.getA()*L.getC()
				)/m;
			y = (
				-L.getA()*L.getB()*P.getX()
				+L.getA()*L.getA()*P.getY()
				-L.getB()*L.getC()
				)/m;
		}
		return Point2D(x,y);
}

void satelliteNode(Point2D Pedal, Point2D Orig, Point2D pair[2], float safetyDist, float perpendicularLength){
	float m=safetyDist/perpendicularLength;
	pair[0].setX(-2*m*(Orig.getX()-Pedal.getX())+Orig.getX());
	pair[0].setY(-2*m*(Orig.getY()-Pedal.getY())+Orig.getY());
	pair[1].setX(-4*m*(Orig.getX()-Pedal.getX())+Orig.getX());
	pair[1].setY(-4*m*(Orig.getY()-Pedal.getY())+Orig.getY());
}

void satelliteNode(Point2D refPoint, Point2D Pedal[MAX_OBS], Point2D Orig[MAX_OBS], Point2D pair[MAX_OBS*MAX_SAT], Point2D ppd[MAX_OBS*MAX_SAT], float safetyDist, float perpendicularLength[MAX_OBS]){
	float m;
	for(int i=0;i<MAX_OBS;i++){
	m=safetyDist/perpendicularLength[i];
	Point2D perpendicularPoint;
#ifdef DEBUG_satelliteNode
	cout<<"perpendicular length="<<perpendicularLength[i]<<" "<<endl;
#endif
	if(perpendicularLength[i]<5){
		float dist=safetyDist/distPoints(Orig[i],refPoint);
		perpendicularPoint.setX((-(refPoint.getY()-Orig[i].getY())+Orig[i].getX()));
		perpendicularPoint.setY((refPoint.getX()-Orig[i].getX())+Orig[i].getY());
#ifdef DEBUG_satelliteNode
		perpendicularPoint.print("perpendicular point");
		Orig[i].print("origin");
		robot.getGoal().print("goal");
#endif
		pair[i*MAX_SAT].setX(-dist*(perpendicularPoint.getX()-Orig[i].getX())+Orig[i].getX());
		pair[i*MAX_SAT].setY(-dist*(perpendicularPoint.getY()-Orig[i].getY())+Orig[i].getY());
		pair[i*MAX_SAT+1].setX(dist*(perpendicularPoint.getX()-Orig[i].getX())+Orig[i].getX());
		pair[i*MAX_SAT+1].setY(dist*(perpendicularPoint.getY()-Orig[i].getY())+Orig[i].getY());
#ifdef DEBUG_satelliteNode
		pair[i*MAX_SAT].print("pair1");
		pair[i*MAX_SAT+1].print("pair2");
#endif
	}
	else{
		perpendicularPoint.setX(0);
		perpendicularPoint.setY(0);

		pair[i*MAX_SAT].setX(-m*(Orig[i].getX()-Pedal[i].getX())+Orig[i].getX());
		pair[i*MAX_SAT].setY(-m*(Orig[i].getY()-Pedal[i].getY())+Orig[i].getY());
		pair[i*MAX_SAT+1].setX(m*(Orig[i].getX()-Pedal[i].getX())+Orig[i].getX());
		pair[i*MAX_SAT+1].setY(m*(Orig[i].getY()-Pedal[i].getY())+Orig[i].getY());
#ifdef DEBUG_satelliteNode
		pair[i*MAX_SAT].print("pair1");
		pair[i*MAX_SAT+1].print("pair2");
#endif
	}

#if(MAX_SAT==4)
		pair[i*MAX_SAT+2].setX(-(pair[i*MAX_SAT].getY()-Orig[i].getY())+Orig[i].getX());
		pair[i*MAX_SAT+2].setY((pair[i*MAX_SAT].getX()-Orig[i].getX())+Orig[i].getY());
		pair[i*MAX_SAT+3].setX(-(pair[i*MAX_SAT+1].getY()-Orig[i].getY())+Orig[i].getX());
		pair[i*MAX_SAT+3].setY((pair[i*MAX_SAT+1].getX()-Orig[i].getX())+Orig[i].getY());
#endif
	ppd[i]=perpendicularPoint;
	}
}

void satelliteNode(RoboMe robot, Point2D Pedal[MAX_OBS], Point2D Orig[MAX_OBS], Point2D pair[MAX_OBS*MAX_SAT], Point2D ppd[MAX_OBS*MAX_SAT], float safetyDist, float perpendicularLength[MAX_OBS]){
	float m;
	for(int i=0;i<MAX_OBS;i++){
	m=safetyDist/perpendicularLength[i];
	Point2D perpendicularPoint;
#ifdef DEBUG_satelliteNode
	cout<<"perpendicular length="<<perpendicularLength[i]<<" "<<endl;
#endif
	if(perpendicularLength[i]<5){
		float dist=safetyDist/distPoints(Orig[i],robot.getGoal());
		perpendicularPoint.setX((-(robot.getGoal().getY()-Orig[i].getY())+Orig[i].getX()));
		perpendicularPoint.setY((robot.getGoal().getX()-Orig[i].getX())+Orig[i].getY());
#ifdef DEBUG_satelliteNode
		perpendicularPoint.print("perpendicular point");
		Orig[i].print("origin");
		robot.getGoal().print("goal");
#endif
		pair[i*MAX_SAT].setX(-dist*(perpendicularPoint.getX()-Orig[i].getX())+Orig[i].getX());
		pair[i*MAX_SAT].setY(-dist*(perpendicularPoint.getY()-Orig[i].getY())+Orig[i].getY());
		pair[i*MAX_SAT+1].setX(dist*(perpendicularPoint.getX()-Orig[i].getX())+Orig[i].getX());
		pair[i*MAX_SAT+1].setY(dist*(perpendicularPoint.getY()-Orig[i].getY())+Orig[i].getY());
#ifdef DEBUG_satelliteNode
		pair[i*MAX_SAT].print("pair1");
		pair[i*MAX_SAT+1].print("pair2");
#endif
	}
	else{
		perpendicularPoint.setX(0);
		perpendicularPoint.setY(0);

		pair[i*MAX_SAT].setX(-m*(Orig[i].getX()-Pedal[i].getX())+Orig[i].getX());
		pair[i*MAX_SAT].setY(-m*(Orig[i].getY()-Pedal[i].getY())+Orig[i].getY());
		pair[i*MAX_SAT+1].setX(m*(Orig[i].getX()-Pedal[i].getX())+Orig[i].getX());
		pair[i*MAX_SAT+1].setY(m*(Orig[i].getY()-Pedal[i].getY())+Orig[i].getY());
#ifdef DEBUG_satelliteNode
		pair[i*MAX_SAT].print("pair1");
		pair[i*MAX_SAT+1].print("pair2");
#endif
	}

#if(MAX_SAT==4)
		pair[i*MAX_SAT+2].setX(-(pair[i*MAX_SAT].getY()-Orig[i].getY())+Orig[i].getX());
		pair[i*MAX_SAT+2].setY((pair[i*MAX_SAT].getX()-Orig[i].getX())+Orig[i].getY());
		pair[i*MAX_SAT+3].setX(-(pair[i*MAX_SAT+1].getY()-Orig[i].getY())+Orig[i].getX());
		pair[i*MAX_SAT+3].setY((pair[i*MAX_SAT+1].getX()-Orig[i].getX())+Orig[i].getY());
#endif
	ppd[i]=perpendicularPoint;
	}
}

void printPoint2D(Point2D p){
	cout<<"P.x="<<p.getX()<<", P.y="<<p.getY()<<endl;
}

int checkBlocking(Point2D RPos, Point2D RGoal, Point2D OC, float robotRadius){
	int ret=0;
	//cout<<"check single blocking: ";
			if(
					((OC.getX()-RPos.getX())*(OC.getX()-RGoal.getX()))<0		// RPos.x < OC.X < RGoal.x    and   RPos.x > OC.X > RGoal.x
			||		((OC.getY()-RPos.getY())*(OC.getY()-RGoal.getY()))<0
			){
				if(distPointToLine(OC, Line2D(RPos,RGoal))>robotRadius*2){

	//				cout<<"Pass!";
					ret=0;
				}
				else{
	//				cout<<"Fail!";
					ret=-1;
				}
			}
			else{
	//			cout<<"Pass!";
				ret=0;
			}
	//cout<<endl;
	return ret;
}
int checkBlockingObstacles(Point2D OA, Point2D OB, Point2D* OC, float robotRadius){
	int ret=0;
	for(int i=0;i<MAX_OBS;i++){
		if(checkBlocking(OA, OB, OC[i], robotRadius)!=0) ret++;
	}
//	printf("%d\n",ret);
	return ret;
}
