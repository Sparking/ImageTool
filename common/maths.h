#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdbool.h>
/* 定义Π的值 */
#define M_PIf               acosf(-1.0f)

#define DEGREE2RAD(degree)	((degree) * (M_PIf / 180.0f))
#define RAD2DEGREE(rad)		((rad) * (180.0f / M_PIf))

struct point {  /* 点 */
    int x;
    int y;
};

struct fpoint {
    float x;
    float y;
};

struct vector { /* 向量 */
    int i;
    int j;
};

struct polygon {           /* 凸多边形 */
    unsigned int  n;       /* 顶点数 */
    struct point *vertex;  /* 顶点数组 */
};

struct line {           /* 有向线段 */
    struct point start; /* 线的起始点 */
    struct point end;   /* 线的终点 */
};

/**
 * @brief find_max_common_divisor 寻找出两个整数的最大公约数
 */
extern unsigned int find_max_common_divisor(const unsigned int a, const unsigned int b);

/**
 * @brief 求出向量的点积
 * @param vec1, vec2 两个向量
 */
extern int vector_dot(const struct vector *vec1, const struct vector *vec2);

/**
 * @brief vector_cross_product 向量的叉乘
 * @param vec1, vec2 两个二维的向量进行叉乘, vec1 x vec2
 * @return 返回叉乘后结果
 * @note 将vec1和vec2扩展成两个三维向量, k分量为0, 两个向量叉乘后i, j分量为0, 所以函数
 *      返回的值是新向量的k分量大小
 */
extern int vector_cross_product(const struct vector *vec1, const struct vector *vec2);

/**
 * @brief vector_cross_product3 三个向量的叉乘
 * @param vec1, vec2, vec3 三个二维的向量进行叉乘, vec1 x vec2 x vec3
 * @return 返回叉乘后结果
 * @note 将vec1, vec2和vec3扩展成三维向量, k分量为0, 两个向量叉乘后i, j分量为0, 所以函数
 *      返回的值是新向量的k分量大小
 */
extern int vector_cross_product3(const struct vector *vec1, const struct vector *vec2,
    const struct vector *vec3);

/**
 * @brief 求解出向量的长度
 * @param vec 向量
 */
extern float vector_length(const struct vector *vec);

/**
 * @brief points_distance 计算点和点之间的距离
 * @param p1, p2 两个点
 */
extern float points_distance(const struct point *p1, const struct point *p2);

/**
 * @brief points_coincide 判断两点是否重合
 * @param p1, p2 两个点
 */
static inline bool points_coincide(const struct point *p1, const struct point *p2)
{
    return p1->x == p2->x && p1->y == p2->y;
}

/**
 * @brief point_position_to_line 判断点和线的位置关系
 * @param p 点
 * @param l 线
 * @return < 0：点在线的左侧
 *         = 0：点在线上
 *         > 0：点在线的右侧 
 */
extern int point_position_to_line(const struct point *p, const struct line *l);

/**
 * @brief point_position_to_polygon 判断点和凸多边形的位置关系
 * @param p 点
 * @param s 凸多边形
 * @return < 0：点在多边形外面(-1：点在边缘的切线上; -2：点不在边缘的切线上)
 *         = 0：点在多边形边上
 *         > 0：点在多边形里面
 */
extern int point_position_to_polygon(const struct point *p, const struct polygon *s);

/**
 * @brief unsigned_diff 返回两个无符号整数的差值
 */
static inline unsigned int unsigned_diff(const unsigned int a, const unsigned int b)
{
    return a >= b ? a - b : b - a;
}

/**
 * @brief bits_count 计算出一个整数中1的位数
 */
extern unsigned char bits_count(unsigned int i);

/**
 * @brief gcd 求解最大公约数
 */
extern int gcd(const int a, const int b);

/**
 * @brief gaussian_elimination 高斯消元法求解
 * @param matrix 矩阵, 增广矩阵[a * x | b], 其中 [a * x]是个rows*rows行的方阵, [b]是个长度位rows的列向量
 * @param res 解的存放位置, 是个长度为rows的数组
 * @param rows 增广矩阵的行数
 * @return 方程有解返回，如果没有解，返回-1
 */
extern int gaussian_elimination(float *matrix, float *res, const unsigned int rows);

/**
 * @brief least_square_method_fit_line 最小二乘法拟合直线 a = y - b * x
 * @param a 截距
 * @param b 斜率
 * @param p 拟合直线的点
 * @param n 点的个数
 * @return 返回值为0时, 直线的表达式为 a = y - b * x
 *         返回值为1时, 直线没有斜率, 表达式变为 x = a
 *         返回值为-1时, 拟合直线失败
 */
extern int least_square_method_fit_line(float *a, float *b, const struct point *p,
        const unsigned int n);
/**
 * @brief line_cross_point 求出直线的交点
 * @param p 记录直线的交点坐标
 * @param a1 直线1的截距
 * @param b1 直线1的斜率
 * @param a2 直线2的截距
 * @param b2 直线2的斜率
 * @return 返回值为0时, 交点的坐标存放的点p中
 *         返回值为-1时, 表示直线没有唯一的交点
 */
extern int line_cross_point(struct point *p, const float a1, const float b1,
        const float a2, const float b2);

#ifdef __cplusplus
}
#endif
