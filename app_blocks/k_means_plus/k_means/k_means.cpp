#include "k_means.h"

double randf(double m)
{
	return m * rand() / (RAND_MAX - 1.);
}

void gen_xy_s(point_s pt[PTS], int count, double radius)
{
	double ang, r;
	/* note: this is not a uniform 2-d distribution */
	for (int i=0;i<count;i++) {
		ang = randf(2 * M_PI);
		r = randf(radius);
		pt[i].x = r * cos(ang);
		pt[i].y = r * sin(ang);
		pt[i].group =0;
	}
}

void init_xy_s(point_s pt[PTS], int count)
{
	/* note: this is not a uniform 2-d distribution */
	for (int i=0;i<count;i++) {
		pt[i].x = 0;
		pt[i].y = 0;
		pt[i].group =0;
	}
}

inline double dist2_s(point_s a, point_s b)
{
	double x = a.x - b.x, y = a.y - b.y;
	return x*x + y*y;
}

inline int nearest_s(point_s pt, point_s cent[K], int n_cluster, double *d2)
{
	int i, min_i;
	double d, min_d;

		min_d = HUGE_VAL;
		min_i = pt.group;
		for (i = 0; i < n_cluster; i++) {
			if (min_d > (d = dist2_s(cent[i], pt))) {
				min_d = d; min_i = i;
			}
		}
	if (d2) *d2 = min_d;
	return min_i;
}
void kpp_s(point_s pts[PTS], int len, point_s cent[PTS], int n_cent)
{
	int j;
	int n_cluster;
	double sum, *d = (double *)malloc(sizeof(double) * len);

	cent[0] = pts[ rand() % len ];
	for (n_cluster = 1; n_cluster < n_cent; n_cluster++) {
		sum = 0;
		for (j = 0; j < len; j++) {
			nearest_s(pts[j], cent, n_cluster, d + j);
			sum += d[j];
		}
		sum = randf(sum);
		for (j = 0; j < len; j++) {
			if ((sum -= d[j]) > 0) continue;
			cent[n_cluster] = pts[j];
			break;
		}
	}
	for (j = 0; j < len; j++) pts[j].group = nearest_s(pts[j], cent, n_cluster, 0);
	free(d);
}



void lloyd_s(point_s pts[PTS], int len, int n_cluster, point_s cent[K])
{
	int i, j, min_i;
	int changed;

	/* assign init grouping randomly */
	//for (j = 0, p = pts; j < len; j++, p++) p->group = j % n_cluster;

	/* or call k++ init */
	kpp_s(pts, len, cent, n_cluster);

	do {
		/* group element for centroids are used as counters */
//		for (c = cent, i = 0; i < n_cluster; i++, c++) { c->group = 0; c->x = c->y = 0; }
		for (i = 0; i < n_cluster; i++) {
			cent[i].group = 0;
			cent[i].x = cent[i].y = 0;
		}

		for (j = 0; j < len; j++) {
//			c = cent + p->group;
			cent[pts[j].group].group++;
//			c->group++;
//			c->x += p->x; c->y += p->y;
			cent[pts[j].group].x+=pts[j].x;
			cent[pts[j].group].y+=pts[j].y;

		}
//		for (c = cent, i = 0; i < n_cluster; i++, c++) { c->x /= c->group; c->y /= c->group; }
		for (i = 0; i < n_cluster; i++) { cent[i].x /= cent[i].group; cent[i].y /= cent[i].group; }
		changed = 0;
		/* find closest centroid of each point */
		for (j = 0; j < len; j++) {
			min_i = nearest_s(pts[j], cent, n_cluster, 0);
			if (min_i != pts[j].group) {
				changed++;
				pts[j].group = min_i;
			}
		}
	} while (changed > (len >> 10)); /* stop when 99.9% of points are good */

//	for (c = cent, i = 0; i < n_cluster; i++, c++) { c->group = i; }
	for (i = 0; i < n_cluster; i++) { cent[i].group = i; }

//	return cent;
}

void print_eps_s(point_s pts[PTS], int len, point_s cent[K], int n_cluster)
{
#	define W 400
#	define H 400
	int i, j;
	double min_x, max_x, min_y, max_y, scale, cx, cy;
	double *colors = (double *)malloc(sizeof(double) * n_cluster * 3);

	for (i = 0; i < n_cluster; i++) {
		colors[3*i + 0] = (3 * (i + 1) % 11)/11.;
		colors[3*i + 1] = (7 * i % 11)/11.;
		colors[3*i + 2] = (9 * i % 11)/11.;
	}

	max_x = max_y = -(min_x = min_y = HUGE_VAL);
	for (j = 0; j < len; j++) {
		if (max_x < pts[j].x) max_x = pts[j].x;
		if (min_x > pts[j].x) min_x = pts[j].x;
		if (max_y < pts[j].y) max_y = pts[j].y;
		if (min_y > pts[j].y) min_y = pts[j].y;
	}
	scale = W / (max_x - min_x);
	if (scale > H / (max_y - min_y)) scale = H / (max_y - min_y);
	cx = (max_x + min_x) / 2;
	cy = (max_y + min_y) / 2;

	printf("%%!PS-Adobe-3.0\n%%%%BoundingBox: -5 -5 %d %d\n", W + 10, H + 10);
	printf( "/l {rlineto} def /m {rmoveto} def\n"
		"/c { .25 sub exch .25 sub exch .5 0 360 arc fill } def\n"
		"/s { moveto -2 0 m 2 2 l 2 -2 l -2 -2 l closepath "
		"	gsave 1 setgray fill grestore gsave 3 setlinewidth"
		" 1 setgray stroke grestore 0 setgray stroke }def\n"
	);
	for (i = 0; i < n_cluster; i++) {
		printf("%g %g %g setrgbcolor\n",
			colors[3*i], colors[3*i + 1], colors[3*i + 2]);
		for (j = 0; j < len; j++) {
			if (pts[j].group != i) continue;
			printf("%.3f %.3f c\n",
				(pts[j].x - cx) * scale + W / 2,
				(pts[j].y - cy) * scale + H / 2);
		}
		printf("\n0 setgray %g %g s\n",
			(cent[i].x - cx) * scale + W / 2,
			(cent[i].y - cy) * scale + H / 2);
	}
	printf("\n%%%%EOF");
	free(colors);
}

