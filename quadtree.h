#ifndef QUADTREE_H
#define QUADTREE_H

typedef struct tPoint Pt;
typedef struct tGeom Geom;

enum {
	GEOM_NONE,
	GEOM_POINT,
	GEOM_LAST
};

struct tPoint {
	float xf, yf, zf;
};

struct tGeom {
	int tag;
	union {
		Pt pt;
	};
};

typedef struct tLeaf Leaf;
typedef struct tNode Node;
typedef struct tQuad Quad;

enum {
	QUAD_NONE,
	QUAD_LEAF,
	QUAD_SMALL,
	QUAD_NODE,
	QUAD_LAST
};

#define LEAFMINSIZE 10

struct tLeaf {
	int size, full;
	Geom **geom;
};

struct tNode {
	int centrex, centrey;
	Quad *nw, *ne, *sw, *se;
};

struct tQuad {
	int tag;
	int left, top, width, height;
	union {
		Leaf leaf;
		Node node;
	};
};

#endif // QUADTREE_H
