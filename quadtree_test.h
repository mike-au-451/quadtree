#ifndef QUADTREE_TEST_H
#define QUADTREE_TEST_H

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

enum {
        NEWS_NONE,
        NEWS_NW,
        NEWS_NE,
        NEWS_SW,
        NEWS_SE,
        NEWS_LAST
};

Geom *P_New(float xf, float yf, float zf);
int almost(int aa, float bb);
int News(int centrex, int centrey, float xf, float yf);
Quad *L_New(int left, int top, int width, int height);
Quad *N_New(int left, int top, int width, int height);
void Q_Add(Quad *quad, Geom *geom);
void Q_Init(Quad *quad, int tag, int left, int top, int width, int height);
void QL_Add(Quad *quad, Geom *geom);
void QL_Add(Quad *quad, Geom *geom);;
void QL_Centre(Quad *quad, int *centrex, int *centrey);
void QL_Grow(Quad *quad);
void QL_Resize(Quad *quad, int newsize);
void QL_SplitLarge(Quad *quad, int centrex, int centrey);
void QL_Split(Quad *quad);
void QL_SplitSmall(Quad *quad);
int Q_Find(Quad *quad, float xf, float yf, Geom **found);

#endif //  QUADTREE_TEST_H

