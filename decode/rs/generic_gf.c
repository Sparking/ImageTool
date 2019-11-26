#include <stdlib.h>
#include <string.h>
#include "generic_gf.h"
#include "port_memory.h"

unsigned int generic_gf_add(const unsigned int a, const unsigned int b)
{
    return a ^ b;
}

unsigned int generic_gf_exp(const struct generic_gf *gf, const unsigned int a)
{
    if (gf == NULL)
        return (unsigned int)~0U;

    return gf->exp_table[a];
}

unsigned int generic_gf_log(const struct generic_gf *gf, const unsigned int a)
{
    if (gf == NULL || a == 0)
        return (unsigned int)~0U;

    return gf->log_table[a];
}

unsigned int generic_gf_inverse(const struct generic_gf *gf, const unsigned int a)
{
    if (gf == NULL || a == 0)
        return (unsigned int)~0U;

    return gf->exp_table[gf->size - gf->log_table[a] - 1];
}

unsigned int generic_gf_multiply(const struct generic_gf *gf, const unsigned int a, const unsigned int b)
{
    if (gf == NULL)
        return (unsigned int)~0U;

    if (a == 0 || b == 0)
        return 0;

    return gf->exp_table[(gf->log_table[a] + gf->log_table[b]) % (gf->size - 1)];
}

struct generic_gf *generic_gf_create(const unsigned int primitive,
        const unsigned int size, const unsigned int base)
{
    struct generic_gf *gf;

    gf = (struct generic_gf *)mem_alloc(sizeof(struct generic_gf) + sizeof(unsigned int) * (size << 1));
    if (gf == NULL)
        return NULL;

    gf->exp_table = (unsigned int *)(((char *)gf) + sizeof(struct generic_gf));
    gf->log_table = (unsigned int *)(((char *)gf->exp_table) + sizeof(unsigned int) * size);
    gf->size = size;
    gf->primitive = primitive;
    gf->generator_base = base;
    
    for (unsigned int x = 1, i = 0; i < size; ++i) {
        gf->exp_table[i] = x;
        x <<= 1;            /* 假设基元alpha为2 */
        if (x >= size) {
            x ^= primitive;
            x &= size - 1;
        }
    }

    /* log_table[0]等于0, 但不应该被使用 */
    for (unsigned int i = 0; i < size - 1; ++i) {
        gf->log_table[gf->exp_table[i]] = i;
    }

    unsigned int x = 0;
    gf->zero = generic_gf_poly_create(gf, &x, 1);
    if (gf->zero == 0) {
        mem_free(gf);
        return NULL;
    }

    x = 1;
    gf->one = generic_gf_poly_create(gf, &x, 1);
    if (gf->zero == 0) {
        generic_gf_poly_release(gf->zero);
        mem_free(gf);
        return NULL;
    }

    return gf;
}

void generic_gf_release(struct generic_gf *gf)
{
    if (gf) {
        generic_gf_poly_release(gf->zero);
        generic_gf_poly_release(gf->one);
        mem_free(gf);
    }
}

struct generic_gf_poly *generic_gf_build_monomial(const struct generic_gf *gf,
    const unsigned int degree, const unsigned int coefficient)
{
    unsigned int *coefficients;
    struct generic_gf_poly *poly = NULL;

    if (coefficient == 0)
        return generic_gf_poly_dump(gf->zero);

    coefficients = (unsigned int *)mem_alloc(sizeof(unsigned int) * (degree + 1));
    if (coefficients == NULL)
        return NULL;

    coefficients[0] = coefficient;
    memset(coefficients + 1, 0, sizeof(unsigned int) * degree);
    poly = generic_gf_poly_create(gf, coefficients, degree + 1);
    mem_free(coefficients);

    return poly;
}

struct generic_gf_poly *generic_gf_poly_create(const struct generic_gf *gf, unsigned int *coefficients, 
        const unsigned int degree)
{
    struct generic_gf_poly *poly;

    if (gf == NULL || coefficients == NULL || degree == 0)
        return NULL;

    poly = (struct generic_gf_poly *)mem_alloc(sizeof(struct generic_gf_poly));
    if (poly == NULL)
        return NULL;

    poly->field = gf;
    if (degree > 1 && coefficients[0] == 0) {
        int first_non_zero = 1;
        while (first_non_zero < degree && coefficients[first_non_zero] == 0) {
            ++first_non_zero;
        }

        if (first_non_zero == degree) {
            poly->coefficients = (unsigned int *)mem_alloc(sizeof(unsigned int));
            if (poly->coefficients == NULL) {
                mem_free(poly);
                return NULL;
            }
            poly->coefficients[0] = 0;
            poly->degree = 0;
        } else {
            int size = sizeof(unsigned int) * (degree - first_non_zero);

            poly->coefficients = (unsigned int *)mem_alloc(size);
            if (poly->coefficients == NULL) {
                mem_free(poly);
                return NULL;
            }

            memcpy(poly->coefficients, coefficients + first_non_zero, size);
            poly->degree = degree - first_non_zero - 1;
        }
    } else {
        int size = sizeof(unsigned int) * degree;
        poly->coefficients = (unsigned int *)mem_alloc(size);
        if (poly->coefficients == NULL) {
            mem_free(poly);
            return NULL;
        }

        memcpy(poly->coefficients, coefficients, size);
        poly->degree = degree - 1;
    }

    return poly;
}

unsigned int generic_gf_poly_degree(const struct generic_gf_poly *poly)
{
    if (poly == NULL)
        return (unsigned int)~0U;

    return poly->degree;
}

unsigned int generic_gf_poly_coeffieient(const struct generic_gf_poly *poly, const unsigned int degree)
{
    if (poly == NULL)
        return (unsigned int)~0U;

    return poly->coefficients[poly->degree - degree];
}

unsigned int generic_gf_poly_evaluateAt(const struct generic_gf_poly *poly, const unsigned int a)
{
    unsigned int result, i;

    if (poly == NULL) {
        result = (unsigned int)~0U;
    } else if (a == 0) {
        result = generic_gf_poly_coeffieient(poly, 0);
    } else if (a == 1) {
        for (result = i = 0; i <= poly->degree; ++i)
            result = generic_gf_add(result, poly->coefficients[i]);
    } else {
        result = poly->coefficients[0];
        for (i = 1; i <= poly->degree; ++i)
            result = generic_gf_add(generic_gf_multiply(poly->field, a, result), poly->coefficients[i]);
    }

    return result;
}

struct generic_gf_poly *generic_gf_poly_dump(const struct generic_gf_poly *a)
{
    struct generic_gf_poly *poly;

    if (a == NULL)
        return NULL;

    poly = (struct generic_gf_poly *)mem_alloc(sizeof(struct generic_gf_poly));
    if (poly == NULL)
        return NULL;

    memcpy(poly, a, sizeof(*poly));
    poly->coefficients = (unsigned int *)mem_alloc(sizeof(unsigned int) * (a->degree + 1));
    if (poly->coefficients == NULL) {
        mem_free(poly);
        return NULL;
    }
    memcpy(poly->coefficients, a->coefficients, sizeof(unsigned int) * (a->degree + 1));

    return poly;
}

struct generic_gf_poly *generic_gf_poly_add(const struct generic_gf_poly *a, const struct generic_gf_poly *b)
{
    struct generic_gf_poly *result;
    const unsigned int *smaller, *larger;
    unsigned int *sum, length_diff, i, lx;

    if (a == NULL || b == NULL)
        return NULL;

    if (a->field->size != b->field->size || a->field->primitive != b->field->primitive ||
                a->field->generator_base != b->field->generator_base)
        return NULL;

    if (a->coefficients[0] == 0)
        return generic_gf_poly_dump(b);

    if (b->coefficients[0] == 0)
        return generic_gf_poly_dump(a);

    if (a->degree > b->degree) {
        larger = a->coefficients;
        smaller = b->coefficients;
        lx = a->degree + 1;
        length_diff = a->degree - b->degree;
    } else {
        smaller = a->coefficients;
        larger = b->coefficients;
        lx = b->degree + 1;
        length_diff = b->degree - a->degree;
    }

    sum = (unsigned int *)mem_alloc(sizeof(unsigned int) * lx);
    if (sum == NULL)
        return NULL;

    memcpy(sum, larger, sizeof(unsigned int) * length_diff);
    for (i = length_diff; i < lx; ++i)
        sum[i] = generic_gf_add(smaller[i - length_diff], larger[i]);

    result = generic_gf_poly_create(a->field, sum, lx);
    mem_free(sum);

    return result;
}

struct generic_gf_poly *generic_gf_poly_multiply(const struct generic_gf_poly *a, const struct generic_gf_poly *b)
{
    struct generic_gf_poly *res;
    const unsigned int *c[2];
    unsigned int *product, l[3];

    if (a == NULL || b == NULL)
        return NULL;

    if ((a->field->size != b->field->size || a->field->primitive != b->field->primitive ||
            a->field->generator_base != b->field->generator_base))
        return NULL;

    if (b->coefficients[0] == 0 || a->coefficients[0] == 0)
        return generic_gf_poly_dump(a->field->zero);

    c[0] = a->coefficients;
    l[0] = a->degree + 1;
    c[1] = b->coefficients;
    l[1] = b->degree + 1;
    l[2] = l[0] + l[1] - 1;
    product = (unsigned int *)mem_alloc(sizeof(unsigned int) * l[2]);
    if (product == NULL)
        return NULL;

    memset(product, 0, sizeof(unsigned int) * l[2]);
    for (unsigned int j, i = 0; i < l[0]; ++i) {
        unsigned int ac = c[0][i];
        for (j = 0; j < l[1]; ++j) {
            product[i + j] = generic_gf_add(product[i + j],
                    generic_gf_multiply(a->field, ac, c[1][j]));
        }
    }

     res = generic_gf_poly_create(a->field, product, l[2]);
     mem_free(product);

     return res;
}

struct generic_gf_poly *generic_gf_poly_multiply_int(const struct generic_gf_poly *a, const unsigned int b)
{
    unsigned int size = a->degree + 1;
    unsigned int *product;
    struct generic_gf_poly *p;

    if (b == 0) {
        return generic_gf_poly_dump(a->field->zero);
    } else if (b == 1) {
        return generic_gf_poly_dump(a);
    }

    product = (unsigned int *)mem_alloc(sizeof(unsigned int) * size);
    if (product == NULL)
        return NULL;

    for (unsigned int i = 0; i < size; ++i) {
        product[i] = generic_gf_multiply(a->field, a->coefficients[i], b);
    }

    p = generic_gf_poly_create(a->field, product, size);
    mem_free(product);

    return p;
}

struct generic_gf_poly *generic_gf_poly_multiply_by_monomial(const struct generic_gf_poly *a, const unsigned int degree, const unsigned int coefficient)
{
    struct generic_gf_poly *res;
    unsigned int *product, size, i;

    if (coefficient == 0)
        return generic_gf_poly_dump(a->field->zero);

    size = a->degree + 1;
    product = (unsigned int *)mem_alloc(sizeof(unsigned int) * (size + degree));
    if (product == NULL)
        return NULL;

    for (i = 0; i < size; ++i)
        product[i] = generic_gf_multiply(a->field, a->coefficients[i], coefficient);
    memset(product + size, 0, sizeof(unsigned int) * degree);

    res  = generic_gf_poly_create(a->field, product, size + degree);
    mem_free(product);

    return res;
}

struct generic_gf_poly *generic_gf_poly_divide(const struct generic_gf_poly *a, const struct generic_gf_poly *b, struct generic_gf_poly **premainder)
{
    struct generic_gf_poly *quotient, *remainder;

    if (a == NULL || b == NULL)
        return NULL;

    if ((a->field->size != b->field->size || a->field->primitive != b->field->primitive ||
            a->field->generator_base != b->field->generator_base))
        return NULL;

    if (b->coefficients[0] == 0)
        return NULL;

    remainder = generic_gf_poly_dump(a);
    if (remainder == NULL)
        return NULL;
    quotient = generic_gf_poly_dump(b->field->zero);
    if (quotient == NULL) {
        generic_gf_poly_release(remainder);
        return NULL;
    }

    int denominator = b->coefficients[b->degree];
    int inverse_denominator = generic_gf_inverse(a->field, denominator);

    while (remainder->degree >= b->degree && remainder->coefficients[0] != 0) {
        struct generic_gf_poly *tmp, *itq, *term;
        unsigned int degree_diff = remainder->degree - b->degree;
        unsigned int scale = generic_gf_multiply(a->field,
                remainder->coefficients[remainder->degree], inverse_denominator);

        term = generic_gf_poly_multiply_by_monomial(b, degree_diff, scale);
        if (term == NULL)
            return NULL;

        tmp = generic_gf_poly_add(remainder, term);
        if (tmp == NULL) {
            generic_gf_poly_release(term);
            return NULL;
        }
        generic_gf_poly_release(term);
        generic_gf_poly_release(remainder);
        remainder = tmp;

        itq = generic_gf_build_monomial(a->field, degree_diff, scale);
        if (itq == NULL) {
            generic_gf_poly_release(remainder);
            return NULL;
        }
        tmp = generic_gf_poly_add(quotient, itq);
        generic_gf_poly_release(quotient);
        quotient = tmp;
    }

    *premainder = remainder;
    return quotient;
}

void generic_gf_poly_release(struct generic_gf_poly *poly)
{
    if (poly) {
        if (poly->coefficients)
            mem_free(poly->coefficients);
        mem_free(poly);
    }
}
