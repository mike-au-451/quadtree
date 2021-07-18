/*

Geom *P_New(float xf, float yf, float zf)

int almost(int aa, float bb)
int News(int centrex, int centrey, float xf, float yf)
void Q_Init(Quad *quad, int tag, int left, int top, int width, int height)
Quad *L_New(int left, int top, int width, int height)
Quad *N_New(int left, int top, int width, int height)

void QL_Centre(Quad *quad, int *centrex, int *centrey)

void QL_Resize(Quad *quad, int newsize)
void QL_Grow(Quad *quad)

void QL_SplitLarge(Quad *quad, int centrex, int centrey)
void QL_SplitSmall(Quad *quad)
void QL_Split(Quad *quad)

void Q_Add(Quad *quad, Geom *geom)
void QL_Add(Quad *quad, Geom *geom)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "quadtree_test.h"

char *Util_G_Tag(int tag)
{
	switch (tag) {
	case GEOM_NONE:
		return "GEOM_NONE";
	case GEOM_POINT:
		return "GEOM_POINT";
	case GEOM_LAST:
		return "GEOM_LAST";
	default:
		fprintf(stderr, "BUG: Util_G_Tag: unknown tag: %d\n", tag);
		exit(1);
	}
}

void P_Dump(Geom *geom)
{
	assert(geom);
	assert(geom->tag == GEOM_POINT);

	Pt *pt = &geom->pt;
	printf("\t(%f, %f, %f)\n", pt->xf, pt->yf, pt->zf);
}

void G_Dump(Geom *geom)
{
	assert(geom);

	printf("Geom @ %p\n", geom);
	printf("\ttag: %d (%s)\n", geom->tag, Util_G_Tag(geom->tag));

	switch (geom->tag) {
	case GEOM_POINT:
		P_Dump(geom);
		break;
	default:
		fprintf(stderr, "BUG: G_Dump: unknown tag: %d\n", geom->tag);
		exit(1);
	}
}

char *Util_Q_Tag(int tag)
{
	switch (tag) {
	case QUAD_NONE:
		return "QUAD_NONE";
	case QUAD_LEAF:
		return "QUAD_LEAF";
	case QUAD_SMALL:
		return "QUAD_SMALL";
	case QUAD_NODE:
		return "QUAD_NODE";
	case QUAD_LAST:
		return "QUAD_LAST";
	default:
		fprintf(stderr, "BUG: Util_Q_Tag: unknown tag: %d\n", tag);
		exit(1);
	}
}

void QL_Dump(Quad *quad)
{
	assert(quad);
	assert(quad->tag == QUAD_LEAF || quad->tag == QUAD_SMALL);

	Leaf *leaf = &quad->leaf;

	printf("Leaf @ %p\n", leaf);
	printf("\tsize: %d\n", leaf->size);
	printf("\tfull: %d\n", leaf->full);
	printf("\tgeom: %p\n", leaf->geom);

	for (int ii = 0; ii < leaf->full; ii++) {
		G_Dump(leaf->geom[ii]);
	}
}

void Q_Dump(Quad *quad);

void QN_Dump(Quad *quad)
{
	assert(quad);
	assert(quad->tag == QUAD_NODE);

	Node *node = &quad->node;

	printf("Node @ %p\n", node);
	printf("\tcentrex: %d\n", node->centrex);
	printf("\tcentrey: %d\n", node->centrey);
	printf("\tnw: %p\n", node->nw);
	printf("\tne: %p\n", node->ne);
	printf("\tsw: %p\n", node->sw);
	printf("\tse: %p\n", node->se);

	if (node->nw) {
		Q_Dump(node->nw);
	}
	if (node->ne) {
		Q_Dump(node->ne);
	}
	if (node->sw) {
		Q_Dump(node->sw);
	}
	if (node->se) {
		Q_Dump(node->se);
	}
}

void Q_Dump(Quad *quad)
{
	assert(quad);

	printf("Quad @ %p\n", quad);
	printf("\ttag: %d (%s)\n", quad->tag, Util_Q_Tag(quad->tag));
	printf("\tleft: %d\n", quad->left);
	printf("\ttop: %d\n", quad->top);
	printf("\twidth: %d\n", quad->width);
	printf("\theight: %d\n", quad->height);

	switch (quad->tag) {
	case QUAD_SMALL:
	case QUAD_LEAF:
		QL_Dump(quad);
		break;
	case QUAD_NODE:
		QN_Dump(quad);
		break;
	case QUAD_NONE:
	case QUAD_LAST:
		break;
	default:
		fprintf(stderr, "BUG: should be impossible\n");
		exit(1);
	}
}

void Util_LeafAddPoint(Quad *quad, Geom *geom)
{
	assert(quad);
	assert(geom);
	assert(quad->tag == QUAD_LEAF);
	assert(geom->tag == GEOM_POINT);

	Leaf *leaf = &quad->leaf;

	assert(leaf->size > 0);
	assert(leaf->full < leaf->size);

	leaf->geom[leaf->full++] = geom;
}

int Test(void)
{
	int ok = 1;

	return ok;
}

int TestP_New(void)
{
	int ok = 1;

	Geom *geom = P_New(12, 34, 56);

	if (geom->tag != GEOM_POINT) {
		printf("failed to tag point\n");
		return 0;
	}

	if (
		geom->pt.xf != 12 ||
		geom->pt.yf != 34 ||
		geom->pt.zf != 56
	) {
		printf("failed to initialize point\n");
		return 0;
	}

	return ok;
}

int Testalmost(void)
{
	int ok = 1;

	struct test {
		float aa, bb;
		bool expect;
	} tests[] = {
		{  0.0, 0.0,  true },
		{  0.0, 0.1,  false  },
		{  0.0, 0.09, true },
		{ -1.0, 0.0,  false },
	};

	float aa, bb;
	for (int ii = 0; tests[ii].aa >= 0; ii++) {
		aa = tests[ii].aa;
		bb = tests[ii].bb;
		if (almost(aa, bb) != tests[ii].expect) {
			printf("failed: %f, %f\n", aa, bb);
			ok = 0;
		}
	}

	return ok;
}

int TestNews(void)
{
	int ok = 1;

	struct test {
		float xf, yf;
		int expect;
	} tests[] = {
		{ 0, 0, NEWS_NW },
		{ 20, 0, NEWS_NE },
		{ 0, 20, NEWS_SW },
		{ 20, 20, NEWS_SE},
		{ -1, 0, NEWS_NONE },
	};
	int centrex = 10;
	int centrey = 10;
	float xf, yf;

	for (int ii = 0; tests[ii].xf > 0; ii++) {
		if (News(centrex, centrey, xf, yf) != tests[ii].expect) {
			xf = tests[ii].xf;
			yf = tests[ii].yf;
			printf("failed: %f, %f\n", xf, yf);
			ok = 0;
		}
	}

	return ok;
}

int TestL_New(void)
{
	int ok = 1;

	Quad *quad = L_New(12, 34, 56, 78);

	if (quad->tag != QUAD_LEAF) {
		printf("failed to tag leaf\n");
		return 0;
	}

	if (
		quad->left != 12 ||
		quad->top != 34 ||
		quad->width != 56 ||
		quad->height != 78
	) {
		printf("failed to initialize quad\n");
		return 0;
	}

	Leaf *leaf = &quad->leaf;

	if (leaf->geom == NULL) {
		printf("failed to allocate geometry\n");
		return 0;
	}
	if (leaf->size != LEAFMINSIZE) {
		printf("failed to set size\n");
		return 0;
	}
	if (leaf->full != 0) {
		printf("failed to set full\n");
		return 0;
	}

	return ok;
}

int TestN_New(void)
{
	int ok = 1;

	Quad *quad = N_New(12, 34, 56, 78);

	if (quad->tag != QUAD_NODE) {
		printf("failed to tag node\n");
		return 0;
	}

	if (
		quad->left != 12 ||
		quad->top != 34 ||
		quad->width != 56 ||
		quad->height != 78
	) {
		printf("failed to initialize quad\n");
		return 0;
	}

	Node *node = &quad->node;

	if (node->nw != NULL || node->ne != NULL || node->sw != NULL || node->se != NULL) {
		printf("failed to initialize node\n");
		return 0;
	}

	return ok;
}

int TestQL_Centre(void)
{
	int ok = 1;

	int centrex, centrey;
	Quad *quad;
	Geom *geom;

	quad = L_New(0, 0, 0, 0);

	geom = P_New(0.0, 0.0, 0.0);
	Util_LeafAddPoint(quad, geom);
	QL_Centre(quad, &centrex, &centrey);
	if (centrex != 0.0 || centrey != 0.0) {
		printf("failed to centre zero leaf\n");
		return 0;
	}

	geom = P_New(10.0, 10.0, 0.0);
	Util_LeafAddPoint(quad, geom);
	QL_Centre(quad, &centrex, &centrey);
	if (centrex != 5.0 || centrey != 5.0) {
		printf("failed to centre zero leaf\n");
		return 0;
	}

	geom = P_New(11.0, 11.0, 0.0);
	Util_LeafAddPoint(quad, geom);
	QL_Centre(quad, &centrex, &centrey);
	if (centrex != 7.0 || centrey != 7.0) {
		printf("failed to centre zero leaf\n");
		return 0;
	}

	return ok;
}

int TestQL_Resize(void)
{
	int ok = 1;

	Quad *quad = L_New(0, 0, 0, 0);
	Leaf *leaf = &quad->leaf;

	// only small quads should be resized
	quad->tag = QUAD_SMALL;
	QL_Resize(quad, 23);
	if (leaf->size != 23) {
		printf("failed to resize leaf\n");
		return 0;
	}
	if (leaf->full != 0) {
		printf("failed to resize leaf (full)\n");
		return 0;
	}

	return ok;
}

int TestQL_Grow(void)
{
	int ok = 1;

	Quad *quad = L_New(0, 0, 0, 0);
	Leaf *leaf = &quad->leaf;

	// only small quads should be resized
	quad->tag = QUAD_SMALL;
	QL_Grow(quad);
	if (leaf->size != 15) {
		printf("failed to grow leaf\n");
		return 0;
	}
	if (leaf->full != 0) {
		printf("failed to grow leaf (full)\n");
		return 0;
	}

	return ok;
}

// void QL_SplitLarge(Quad *quad, int centrex, int centrey)
// void QL_SplitSmall(Quad *quad)
// void QL_Split(Quad *quad)

int HelpQ_Expect(Quad *quad, Quad *expect)
{
	assert(quad);
	assert(expect);

	if (quad->tag != expect->tag) {
		printf("failed to tag node\n");
		return 0;
	}

	if (
		quad->left != expect->left ||
		quad->top != expect->top ||
		quad->width != expect->width ||
		quad->height != expect->height
	) {
		printf("failed to retain bounds\n");
		return 0;
	}

	switch (quad->tag) {
	case QUAD_NODE:
		break;
	case QUAD_LEAF:
		break;
	default:
		fprintf(stderr, "BUG: HelpQL_SplitLarge: unexpected tag: %d\n", quad->tag);
		exit(1);
	}

	return 1;
}

int HelpQN_Expect(Quad *quad, int left, int top, int width, int height, int centrex, int centrey)
{
	assert(quad);

	Quad expect;

	memset(&expect, 0, sizeof(Quad));
	expect.tag = QUAD_NODE;

	expect.left = left;
	expect.top = top;
	expect.width = width;
	expect.height = height;
	expect.node.centrex = centrex;
	expect.node.centrey = centrey;

	return HelpQ_Expect(quad, &expect);
}

int HelpQL_Expect(Quad *quad, int left, int top, int width, int height, int size, int full)
{
	assert(quad);

	Quad expect;

	memset(&expect, 0, sizeof(Quad));
	expect.tag = QUAD_LEAF;

	expect.left = left;
	expect.top = top;
	expect.width = width;
	expect.height = height;
	expect.leaf.size = size;
	expect.leaf.full = full;

	return HelpQ_Expect(quad, &expect);
}

int TestQL_SplitLarge(void)
{
	int ok = 1;

	Quad *quad = L_New(0, 0, 100, 100);
	Geom *geom;

	for (int ii = 0; ii < LEAFMINSIZE - 4; ii += 4) {
		geom = P_New(ii + 1, ii + 1, 0.0);
		Util_LeafAddPoint(quad, geom);
		geom = P_New(100 - 1 - ii, ii + 1, 0.0);
		Util_LeafAddPoint(quad, geom);
		geom = P_New(ii + 1, 100 - 1 - ii, 0.0);
		Util_LeafAddPoint(quad, geom);
		geom = P_New(100 - 1 - ii, 100 - 1 - ii, 0.0);
		Util_LeafAddPoint(quad, geom);
	}
	// Q_Dump(quad);
	// (1, 1) (99, 1) (1, 99) (99, 99) (5, 5) (95, 5) (5, 95) (95, 95)

	int centrex, centrey;
	QL_Centre(quad, &centrex, &centrey);	
	// printf("...(%d, %d)\n", centrex, centrey);
	// 50, 50
	QL_SplitLarge(quad, centrex, centrey);
	// Q_Dump(quad);
	// exit(1);

	if (!HelpQN_Expect(quad, 0, 0, 100, 100, 50, 50)) {
		printf("failed to create new node\n");
		return 0;
	}

	Node *node = &quad->node;
	if (node->nw == NULL || node->ne == NULL || node->sw == NULL || node->se == NULL) {
		printf("failed to create new leaves\n");
		return 0;
	}

	if (!HelpQL_Expect(node->nw, 0, 0, 50, 50, LEAFMINSIZE, 2)) {
		printf("failed to init nw leaf\n");
		return 0;
	}

	if (!HelpQL_Expect(node->ne, 50, 0, 50, 50, LEAFMINSIZE, 2)) {
		printf("failed to init ne leaf\n");
		return 0;
	}

	if (!HelpQL_Expect(node->sw, 0, 50, 50, 50, LEAFMINSIZE, 2)) {
		printf("failed to init sw leaf\n");
		return 0;
	}

	if (!HelpQL_Expect(node->se, 50, 50, 50, 50, LEAFMINSIZE, 2)) {
		printf("failed to init se leaf\n");
		return 0;
	}

	// TODO: check all leaves and points

	return ok;
}

/*
Add points along diagonals in all the separate quadrants.
*/
void Help_AddPoints(Quad *quad, int cnt)
{
	assert(quad);
	assert(quad->tag == QUAD_LEAF);

	Geom *geom;
	int phase;

	phase = 0;
	for (int ii = 0; ii < cnt; ii++) {
		switch (phase) {
		case 0:
			geom = P_New(ii + 1, ii + 1, ii);
			break;
		case 1:
			geom = P_New(quad->left + quad->width - ii - 1, quad->top + ii + 1, ii);
			break;
		case 2:
			geom = P_New(ii + 1, quad->top + quad->height - ii - 1, ii);
			break;
		case 3:
			geom = P_New(quad->left + quad->width - ii - 1, quad->top + quad->height - ii - 1, ii);
			break;
		}
		Q_Add(quad, geom);

		phase++;
		if (phase > 3) {
			phase = 0;
		}
	}
}

int TestQ_Add01(void)
{
	int ok = 1;

	Quad *quad = L_New(0, 0, 100, 100);

	Help_AddPoints(quad, 1);

	if (!HelpQL_Expect(quad, 0, 0, 100, 100, LEAFMINSIZE, 1)) {
		printf("failed to add single point\n");
		return 0;
	}

	return ok;
}

int TestQ_Add02(void)
{
	int ok = 1;

	Quad *quad = L_New(0, 0, 100, 100);

	Help_AddPoints(quad, LEAFMINSIZE);

	if (!HelpQL_Expect(quad, 0, 0, 100, 100, LEAFMINSIZE, LEAFMINSIZE)) {
		printf("failed to add LEAFMINSIZE+1 points\n");
		return 0;
	}

	// TODO: check individual points

	return ok;
}

int TestQ_Add03(void)
{
	int ok = 1;

	Quad *quad = L_New(0, 0, 100, 100);

	Help_AddPoints(quad, 100);

	if (!HelpQN_Expect(quad, 0, 0, 100, 100, 44, 16)) {
		printf("failed to add LEAFMINSIZE+1 points\n");
		return 0;
	}

	// TODO: check individual points

	return ok;
}

int TestQ_Add(void)
{
	int ok = 1;

	if (!TestQ_Add01()) {
		printf("failed to add single point\n");
		return 0;
	}
	if (!TestQ_Add02()) {
		printf("failed to add LEAFMINSIZE points\n");
		return 0;
	}
	if (!TestQ_Add03()) {
		printf("failed to add lots of points\n");
		return 0;
	}

	return ok;
}

int TestQ_Find01(void)
{
	int ok = 1;

	Quad *quad = L_New(0, 0, 100, 100);
	Geom *found = NULL;

	Help_AddPoints(quad, 1);

	if (!Q_Find(quad, 1, 1, &found)) {
		printf("failed to find single point\n");
		return 0;
	}
	if (found == NULL) {
		printf("failed to set found\n");
		return 0;
	}
	if (found->tag != GEOM_POINT) {
		printf("found wrong geom\n");
		return 0;
	}

	Pt *pt = &found->pt;
	if (pt->xf != 1 || pt->yf != 1) {
		printf("found wrong point\n");
		return 0;
	}

	return ok;
}

int TestQ_Find02(void)
{
	int ok = 1;

	Quad *quad = L_New(0, 0, 100, 100);
	Geom *found = NULL;

	int npts = 10000;
	Help_AddPoints(quad, npts);

	float xf, yf;
	int phase = 0;
	for (int ii = 0; ii < npts; ii++) {
		switch (phase) {
		case 0:
			xf = ii + 1;
			yf = ii + 1;
			break;
		case 1:
			xf = quad->left + quad->width - ii - 1;
			yf = quad->top + ii + 1;
			break;
		case 2:
			xf = ii + 1;
			yf = quad->top + quad->height - ii - 1;
			break;
		case 3:
			xf = quad->left + quad->width - ii - 1;
			yf = quad->top + quad->height - ii - 1;
			break;
		default:
			fprintf(stderr, "BUG: bad phase\n");
			exit(1);
		}

		if (!Q_Find(quad, xf, yf, &found)) {
			printf("failed to find (%d, %d)\n", ii, ii);
			Q_Dump(quad);
			exit(1);
			return 0;
		}
		if (found == NULL) {
			printf("failed to set found\n");
			return 0;
		}
		if (found->tag != GEOM_POINT) {
			printf("failed to find point\n");
			return 0;
		}

		Pt *pt = &found->pt;
		if (pt->xf != xf || pt->yf != yf) {
			printf("failed to correct point\n");
			return 0;
		}

		phase++;
		if (phase > 3) {
			phase = 0;
		}
	}

	return ok;
}

int TestQ_Find(void)
{
	int ok = 1;

	if (!TestQ_Find01()) {
		return 0;
	}
	if (!TestQ_Find02()) {
		return 0;
	}

	return ok;
}

struct tTest {
	char *stz;
	int (*test)(void);
};

int main(int argc, char **argv)
{
	int ok = 1;
	char *msg;
	struct tTest tests[] = {
		{ "P_New", TestP_New },
		{ "L_New", TestL_New },
		{ "N_New", TestN_New },
		{ "almost", Testalmost },
		{ "News", TestNews },
		{ "QL_Centre", TestQL_Centre },
		{ "QL_Resize", TestQL_Resize },
		{ "QL_Grow", TestQL_Grow },
		{ "QL_SplitLarge", TestQL_SplitLarge },
		{ "Q_Add", TestQ_Add },
		{ "Q_Find", TestQ_Find },
		{ NULL, NULL }
	};

	for (int ii = 0; tests[ii].stz; ii++) {
		if (tests[ii].test()) {
			msg = "passed";
		}
		else {
			msg = "failed";
			ok = 0;
		}
		printf("%d %s: %s\n", ii, msg, tests[ii].stz);
	}

	return ok ? 0 : 1;
}


