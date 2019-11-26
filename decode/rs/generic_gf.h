#pragma once

#ifdef __cplusplus
extern "C" {
#endif

struct generic_gf_poly;

struct generic_gf {
    unsigned int *exp_table;        /* 指数表 */
    unsigned int *log_table;        /* 对数表 */
    struct generic_gf_poly *zero;
    struct generic_gf_poly *one;
    unsigned int size;              /* 域的大小 */
    unsigned int primitive;         /* 本原多项式 */
    unsigned int generator_base;    /* 伽罗华域基数 */
};

struct generic_gf_poly {
    const struct generic_gf *field;
    unsigned int *coefficients;
    unsigned int degree;
};

/**
 * @brief generic_gf_create 创建伽罗华域
 * @param primitive 本原多项式
 * @param size 域的大小
 * @param base 基元
 *  (g(x) = (x+a^base)(x+a^(base+1))...(x+a^(base+2t-1))).
 */
extern struct generic_gf *generic_gf_create(const unsigned int primitive,
        const unsigned int size, const unsigned int base);
extern void generic_gf_release(struct generic_gf *gf);
extern unsigned int generic_gf_add(const unsigned int a, const unsigned int b);
extern unsigned int generic_gf_exp(const struct generic_gf *gf, const unsigned int a);
extern unsigned int generic_gf_log(const struct generic_gf *gf, const unsigned int a);
extern unsigned int generic_gf_inverse(const struct generic_gf *gf, const unsigned int a);
extern unsigned int generic_gf_multiply(const struct generic_gf *gf, const unsigned int a, const unsigned int b);

extern struct generic_gf_poly *generic_gf_build_monomial(const struct generic_gf *gf,
        const unsigned int degree, const unsigned int coefficient);

extern struct generic_gf_poly *generic_gf_poly_create(const struct generic_gf *gf, unsigned int *coefficients, 
        const unsigned int degree);
extern struct generic_gf_poly *generic_gf_poly_dump(const struct generic_gf_poly *a);
extern unsigned int generic_gf_poly_degree(const struct generic_gf_poly *poly);
extern unsigned int generic_gf_poly_coeffieient(const struct generic_gf_poly *poly, const unsigned int degree);
extern unsigned int generic_gf_poly_evaluateAt(const struct generic_gf_poly *poly, const unsigned int a);
extern struct generic_gf_poly *generic_gf_poly_add(const struct generic_gf_poly *a, const struct generic_gf_poly *b);
extern struct generic_gf_poly *generic_gf_poly_multiply(const struct generic_gf_poly *a, const struct generic_gf_poly *b);
extern struct generic_gf_poly *generic_gf_poly_multiply_int(const struct generic_gf_poly *a, const unsigned int b);
extern struct generic_gf_poly *generic_gf_poly_multiply_by_monomial(const struct generic_gf_poly *a,
            const unsigned int degree, const unsigned int coefficient);
extern void generic_gf_poly_release(struct generic_gf_poly *poly);

#ifdef __cplusplus
}
#endif
