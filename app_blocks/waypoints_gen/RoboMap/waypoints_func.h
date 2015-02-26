#include "RoboMap.h"
using namespace RM;

float arccosLines(Point2D A, Point2D B, Point2D C, Point2D D);
float distPoints(Point2D A, Point2D B);
float distPointToLine(Point2D P, Line2D L);
Point2D PedalCoordinates(Point2D P, Line2D L);
void satelliteNode(Point2D refPoint, Point2D Pedal[MAX_OBS], Point2D Orig[MAX_OBS], Point2D pair[MAX_OBS*MAX_SAT], Point2D ppd[MAX_OBS*MAX_SAT], float safetyDist, float perpendicularLength[MAX_OBS]);
void satelliteNode(Point2D Pedal, Point2D Orig, Point2D pair[2], float safetyDist, float perpendicularLength);
void satelliteNode(RoboMe robot, Point2D Pedal[MAX_OBS], Point2D Orig[MAX_OBS], Point2D pair[MAX_OBS*MAX_SAT], Point2D ppd[MAX_OBS*MAX_SAT], float safetyDist, float perpendicularLength[MAX_OBS]);
void printPoint2D(Point2D p);
int checkBlocking(Point2D RPos, Point2D RGoal, Point2D OC, float robotRadius);
int checkBlockingObstacles(Point2D OA, Point2D OB, Point2D* OC, float robotRadius);
