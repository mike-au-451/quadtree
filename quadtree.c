#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "quadtree.h"

Geom *P_New(float xf, float yf, float zf)
{
	Geom *geom = calloc(1, sizeof(Geom));
	assert(geom);

	geom->tag = GEOM_POINT;

	Pt *pt = &geom->pt;
	pt->xf = xf;
	pt->yf = yf;
	pt->zf = zf;

	return geom;
}

void Q_Init(Quad *quad, int tag, int left, int top, int width, int height)
{
	assert(quad);

	quad->tag = tag;
	quad->left = left;
	quad->top = top;
	quad->width = width;
	quad->height = height;
}

Quad *L_New(int left, int top, int width, int height)
{
	Quad *quad = calloc(1, sizeof(Quad));
	assert(quad);

	Q_Init(quad, QUAD_LEAF, left, top, width, height);

	Leaf *leaf = &quad->leaf;

	leaf->geom = calloc(LEAFMINSIZE, sizeof(Geom *));
	assert(leaf->geom);

	leaf->size = LEAFMINSIZE;

	return quad;
}

Quad *N_New(int left, int top, int width, int height)
{
	Quad *quad = calloc(1, sizeof(Quad));
	assert(quad);

	Q_Init(quad, QUAD_NODE, left, top, width, height);

	return quad;
}

#define LEAFMINSIZE 10

void QL_Resize(Quad *quad, int newsize)
{
	assert(quad);
	assert(quad->tag == QUAD_SMALL);

	Leaf *leaf = &quad->leaf;
	Geom **newgeom;

	if (newsize < LEAFMINSIZE) {
		newsize = LEAFMINSIZE;
	}

	if ((newgeom = calloc(newsize, sizeof(Geom *))) == NULL) {
		fprintf(stderr, "BUG: QL_Resize: no memory\n");
		exit(1);
	}

	if (leaf->size) {
		assert(leaf->geom);
		memcpy(newgeom, leaf->geom, leaf->full * sizeof(Geom *));
		free(leaf->geom);
	}

	leaf->geom = newgeom;
	leaf->size = newsize;
}

void QL_Grow(Quad *quad)
{
	assert(quad);
	assert(quad->tag == QUAD_SMALL);

	Leaf *leaf = &quad->leaf;
	QL_Resize(quad, leaf->size * 3 / 2);
}

void QL_Add(Quad *quad, Geom *geom);

void QL_SplitSmall(Quad *quad)
{
	assert(quad);
	assert(quad->tag == QUAD_LEAF);

	quad->tag = QUAD_SMALL;
}

enum {
	NEWS_NONE,
	NEWS_NW,
	NEWS_NE,
	NEWS_SW,
	NEWS_SE,
	NEWS_LAST
};

int almost(float aa, float bb)
{
	aa = aa - bb;
	aa = aa * aa;
	return aa < 0.01;
}

int News(int centrex, int centrey, float xf, float yf)
{
	if (xf < centrex) {
		if (yf < centrey) {
			return NEWS_NW;
		}
		else {
			return NEWS_SW;
		}
	}
	else {
		if (yf < centrey) {
			return NEWS_NE;
		}
		else {
			return NEWS_SE;
		}
	}
}

void QL_SplitLarge(Quad *quad, int centrex, int centrey)
{
	assert(quad);
	assert(quad->tag == QUAD_LEAF);

	Quad *nw = L_New(quad->left, quad->top, centrex - quad->left, centrey - quad->top);
	Quad *ne = L_New(centrex, quad->top, quad->left + quad->width - centrex, centrey - quad->top);
	Quad *sw = L_New(quad->left, centrey, centrex - quad->left, quad->top + quad->height - centrey);
	Quad *se = L_New(centrex, centrey, quad->left + quad->width - centrex, quad->top + quad->height - centrey);

	// distribute points evenly to the new leaf nodes.
	Leaf *leaf = &quad->leaf;
	Geom *geom;
	Quad *news;
	Pt *pt;

	for (int ii = 0; ii < leaf->full; ii++) {
		geom = leaf->geom[ii];
		assert(geom);
		assert(geom->tag == GEOM_POINT);
		pt = &geom->pt;
		news = NULL;
		switch (News(centrex, centrey, pt->xf, pt->yf)) {
		case NEWS_NW:
			news = nw;
			break;
		case NEWS_NE:
			news = ne;
			break;
		case NEWS_SW:
			news = sw;
			break;
		case NEWS_SE:
			news = se;
			break;
		default:
			fprintf(stderr, "BUG: QL_Split: unknown news\n");
			exit(1);
		}
		assert(news);
		assert(news->tag == QUAD_LEAF);

		QL_Add(news, geom);
	}

	// repurpose the quad as a NODE 
	Node *node = &quad->node;
	memset(node, 0, sizeof(Node));
	node->centrex = centrex;
	node->centrey = centrey;
	node->nw = nw;
	node->ne = ne;
	node->sw = sw;
	node->se = se;

	quad->tag = QUAD_NODE;
}

// Find a centre such that that points are evenly distributed
void QL_Centre(Quad *quad, int *centrex, int *centrey)
{
	assert(quad);
	assert(centrex);
	assert(centrey);
	assert(quad->tag == QUAD_LEAF);
	assert(quad->leaf.full > 0);

	float xfsum, yfsum;
	Leaf *leaf = &quad->leaf;
	Geom *geom;
	Pt *pt;

	xfsum = yfsum = 0.0;
	for (int ii = 0; ii < leaf->full; ii++) {
		geom = leaf->geom[ii];
		assert(geom);
		assert(geom->tag == GEOM_POINT);
		pt = &geom->pt;
		xfsum += pt->xf;
		yfsum += pt->yf;
	}

	*centrex = (int) xfsum / leaf->full;
	*centrey = (int) yfsum / leaf->full;
}

#define QUADMINEXTENT 10

void QL_Split(Quad *quad)
{
	assert(quad);
	assert(quad->tag == QUAD_LEAF);

	int centrex, centrey;
	QL_Centre(quad, &centrex, &centrey);

	if (
		centrex - quad->left < QUADMINEXTENT ||
		quad->left + quad->width - centrex < QUADMINEXTENT ||
		centrey - quad->top < QUADMINEXTENT ||
		quad->top + quad->height - centrey < QUADMINEXTENT
	) {
		QL_SplitSmall(quad);
	}
	else {
		QL_SplitLarge(quad, centrex, centrey);
	}
}

void QL_Add(Quad *quad, Geom *geom)
{
	assert(quad);
	assert(geom);
	assert(quad->tag == QUAD_LEAF || quad->tag == QUAD_SMALL);
	assert(geom->tag == GEOM_POINT);

	Leaf *leaf = &quad->leaf;

	assert(leaf->full < leaf->size);
	leaf->geom[leaf->full++] = geom;
}

int QL_Find(Quad *quad, float xf, float yf, Geom **found)
{
	assert(quad);
	assert(quad->tag == QUAD_LEAF || quad->tag == QUAD_SMALL);

	Leaf *leaf = &quad->leaf;
	Geom *geom;
	int ii;
	for (ii = 0; ii < leaf->full; ii++) {
		geom = leaf->geom[ii];
		assert(geom);
		assert(geom->tag == GEOM_POINT);
		if (almost(geom->pt.xf, xf) && almost(geom->pt.yf, yf)) {
			*found = geom;
			break;
		}
	}

	return ii < leaf->full;
}

void Q_Add(Quad *quad, Geom *geom)
{
	assert(quad);
	assert(geom);
	assert(geom->tag == GEOM_POINT);

	Leaf *leaf = &quad->leaf;
	Node *node = &quad->node;
	Pt *pt = &geom->pt;
	Quad *news = NULL;

	switch (quad->tag) {
	case QUAD_LEAF:
		if (leaf->full == LEAFMINSIZE) {
			QL_Split(quad);
			Q_Add(quad, geom);
		}
		else {
			QL_Add(quad, geom);
		}
		break;
	case QUAD_SMALL:
		if (leaf->full == leaf->size) {
			QL_Grow(quad);
		}
		QL_Add(quad, geom);
		break;
	case QUAD_NODE:
		switch (News(node->centrex, node->centrey, pt->xf, pt->yf)) {
		case NEWS_NW:
			news = node->nw;
			break;
		case NEWS_NE:
			news = node->ne;
			break;
		case NEWS_SW:
			news = node->sw;
			break;
		case NEWS_SE:
			news = node->se;
			break;
		default:
			fprintf(stderr, "BUG: Q_Add: unknown news\n");
			exit(1);
		}
		Q_Add(news, geom);
		break;
	default:
		fprintf(stderr, "BUG: Q_Add: unknown tag: %d\n", quad->tag);
		exit(1);
	}
}

int Q_Find(Quad *quad, float xf, float yf, Geom **found)
{
	assert(quad);
	assert(found);

	switch (quad->tag) {
	case QUAD_NODE:
		Node *node = &quad->node;
		switch (News(node->centrex, node->centrey, xf, yf)) {
		case NEWS_NW:
			quad = node->nw;
			break;
		case NEWS_NE:
			quad = node->ne;
			break;
		case NEWS_SW:
			quad = node->sw;
			break;
		case NEWS_SE:
			quad = node->se;
			break;
		default:
			fprintf(stderr, "BUG: Q_Find: unknown news\n");
			exit(1);
		}
		return Q_Find(quad, xf, yf, found);
	case QUAD_LEAF:
	case QUAD_SMALL:
		return QL_Find(quad, xf, yf, found);
	default:
		fprintf(stderr, "BUG: Q_Find: unknown tag: %d\n", quad->tag);
		exit(1);
	}
}
