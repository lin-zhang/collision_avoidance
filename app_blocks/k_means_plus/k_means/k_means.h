#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>

#define PTS 360
#define K 12
typedef struct { double x, y; int group; } point_s, scalar_pt;


double randf(double m);
void gen_xy_s(point_s pt[PTS], int count, double radius);
void init_xy_s(point_s pt[PTS], int count);
inline double dist2_s(point_s a, point_s b);
inline int
nearest_s(point_s pt, point_s cent[K], int n_cluster, double *d2);
void kpp_s(point_s pts[PTS], int len, point_s cent[PTS], int n_cent);
void lloyd_s(point_s pts[PTS], int len, int n_cluster, point_s cent[K]);
void print_eps_s(point_s pts[PTS], int len, point_s cent[K], int n_cluster);
