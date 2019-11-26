#include <stdlib.h>
#include <string.h>
#include "rsdecode.h"
#include "port_memory.h"

static struct generic_gf_poly **run_euclidean_algorithm(const struct generic_gf *field,
        const struct generic_gf_poly *a, const struct generic_gf_poly *b, const unsigned int R)
{
    struct generic_gf_poly **poly;
    struct generic_gf_poly **sigma, **omiga;
    struct generic_gf_poly *rlast = NULL, *r = NULL;
    struct generic_gf_poly *tlast = NULL, *t = NULL;
    int ret;

    if (field == NULL || a == NULL || b == NULL)
        return NULL;

    poly = (struct generic_gf_poly **)mem_alloc(sizeof(struct generic_gf_poly *) << 1);
    if (poly == NULL)
        return NULL;

    ret = -1;
    sigma = poly;
    omiga = poly + 1;
    *sigma = *omiga = NULL;
    t = generic_gf_poly_dump(field->one);
    tlast = generic_gf_poly_dump(field->zero);
    if (a->degree < b->degree) {
        r = generic_gf_poly_dump(a);
        rlast = generic_gf_poly_dump(b);
    } else {
        r = generic_gf_poly_dump(b);
        rlast = generic_gf_poly_dump(a);
    }
    if (t == NULL || tlast == NULL || r == NULL || rlast == NULL)
        goto release_resource;

    while (r->degree >= (R >> 1)) {
        struct generic_gf_poly *q, *ptemp, *rlastlast = NULL, *tlastlast = NULL;

        if (r->coefficients[0] == 0)
            goto release_resource;

        rlastlast = generic_gf_poly_dump(rlast);
        tlastlast = generic_gf_poly_dump(tlast);
        if (rlastlast == NULL || tlastlast == NULL) {
            generic_gf_poly_release(rlastlast);
            generic_gf_poly_release(tlastlast);
            goto release_resource;
        }

        generic_gf_poly_release(rlast);
        generic_gf_poly_release(tlast);
        rlast = generic_gf_poly_dump(r);
        tlast = generic_gf_poly_dump(t);
        if (rlast == NULL || tlast == NULL) {
            generic_gf_poly_release(rlastlast);
            generic_gf_poly_release(tlastlast);
            goto release_resource;
        }

        generic_gf_poly_release(r);
        r = generic_gf_poly_dump(rlastlast);
        q = generic_gf_poly_dump(field->zero);
        if (r == NULL || q == NULL) {
            generic_gf_poly_release(rlastlast);
            generic_gf_poly_release(tlastlast);
            generic_gf_poly_release(q);
            goto release_resource;
        }

        unsigned int dlt = generic_gf_poly_coeffieient(rlast, rlast->degree);
        unsigned int dlt_inverse = generic_gf_inverse(field, dlt);
        while (r->degree >= rlast->degree && r->coefficients[0] != 0) {
            struct generic_gf_poly *temp[2];
            unsigned int degree_diff = r->degree - rlast->degree;
            unsigned int scale = generic_gf_multiply(field, generic_gf_poly_coeffieient(r, r->degree), dlt_inverse);

            temp[0] = generic_gf_build_monomial(field, degree_diff, scale);
            if (temp[0] == NULL) {
                generic_gf_poly_release(rlastlast);
                generic_gf_poly_release(tlastlast);
                generic_gf_poly_release(q);
                goto release_resource;
            }

            temp[1] = generic_gf_poly_add(q, temp[0]);
            generic_gf_poly_release(temp[0]);
            generic_gf_poly_release(q);
            q = temp[1];
            if (q == NULL) {
                generic_gf_poly_release(rlastlast);
                generic_gf_poly_release(tlastlast);
                goto release_resource;
            }

            temp[0] = generic_gf_poly_multiply_by_monomial(rlast, degree_diff, scale);
            if (temp[0] == NULL) {
                generic_gf_poly_release(rlastlast);
                generic_gf_poly_release(tlastlast);
                generic_gf_poly_release(q);
                goto release_resource;
            }

            temp[1] = generic_gf_poly_add(r, temp[0]);
            generic_gf_poly_release(temp[0]);
            generic_gf_poly_release(r);
            r = temp[1];
            if (r == NULL) {
                generic_gf_poly_release(rlastlast);
                generic_gf_poly_release(tlastlast);
                generic_gf_poly_release(q);
                goto release_resource;
            }
        }

        if (r->degree >= rlast->degree) {
            generic_gf_poly_release(rlastlast);
            generic_gf_poly_release(tlastlast);
            generic_gf_poly_release(q);
            goto release_resource;
        }

        ptemp = generic_gf_poly_multiply(q, tlast);
        generic_gf_poly_release(q);
        if (ptemp == NULL) {
            generic_gf_poly_release(rlastlast);
            generic_gf_poly_release(tlastlast);
            goto release_resource;
        }
        generic_gf_poly_release(t);
        t = generic_gf_poly_add(ptemp, tlastlast);
        generic_gf_poly_release(ptemp);
        generic_gf_poly_release(rlastlast);
        generic_gf_poly_release(tlastlast);
        if (t == NULL) {
            goto release_resource;
        }
    }

    unsigned int sigma_taz = generic_gf_poly_coeffieient(t, 0);
    if (sigma_taz == 0)
        goto release_resource;

    unsigned int inverse = generic_gf_inverse(field, sigma_taz);
    *sigma = generic_gf_poly_multiply_int(t, inverse);
    *omiga = generic_gf_poly_multiply_int(r, inverse);
    if (*sigma == NULL || *omiga == NULL)
        goto release_resource;

    ret = 0;
release_resource:
    generic_gf_poly_release(rlast);
    generic_gf_poly_release(tlast);
    generic_gf_poly_release(r);
    generic_gf_poly_release(t);
    if (ret == -1) {
        generic_gf_poly_release(poly[0]);
        generic_gf_poly_release(poly[1]);
        mem_free(poly);
        poly = NULL;
    }

    return poly;
}

static unsigned int *find_error_locations(const struct generic_gf *field, const struct generic_gf_poly *error_locator, unsigned int *length)
{
    unsigned int num, e, i;
    unsigned int *res;

    if (length == NULL)
        return NULL;

    if (field == NULL || error_locator == NULL) {
        *length = 0;
        return NULL;
    }

    num = error_locator->degree;
    res = (unsigned int *)mem_alloc(sizeof(unsigned int) * num);
    if (res == NULL) {
        *length = 0;
        return NULL;
    }

    *length = num;
    if (num == 1) {
        res[0] = error_locator->coefficients[1];
        return res;
    }

    memset(res, 0, sizeof(unsigned int) * num);
    for (e = 0, i = 1; i < field->size; ++i) {
        if (generic_gf_poly_evaluateAt(error_locator, i) == 0) {
            res[e] = generic_gf_inverse(field, i);
            e++;
        }
    }

    if (e != num) {
        *length = 0;
        mem_free(res);
        res = NULL;
    }

    return res;
}

static unsigned int *find_error_magnitudes(const struct generic_gf *field, const struct generic_gf_poly *error_eva, unsigned int *err_loc, const unsigned int s)
{
    unsigned int *mag;

    mag = (unsigned int *)mem_alloc(sizeof(unsigned int) * s);
    if (mag == NULL)
        return NULL;

    for (unsigned int i = 0; i < s; ++i) {
        unsigned int xi_inverse = generic_gf_inverse(field, err_loc[i]);
        int denominator = 1;
        for (unsigned int j = 0; j < s; ++j) {
            if (i != j) {
                unsigned int term = generic_gf_multiply(field, err_loc[j], xi_inverse);
                unsigned int term1 = ((term & 0x01) == 0) ? term | 1 : term & (unsigned int)~1U;
                denominator = generic_gf_multiply(field, denominator, term1);
            }
        }
        mag[i] = generic_gf_multiply(field, generic_gf_poly_evaluateAt(error_eva, xi_inverse), generic_gf_inverse(field, denominator));
        if (field->generator_base != 0) {
            mag[i] = generic_gf_multiply(field, mag[i], xi_inverse);
        }
    }

    return mag;
}

int rsdecode(const struct generic_gf *field, unsigned int *received, unsigned int length, unsigned int twos)
{
    struct generic_gf_poly *poly;
    unsigned int *sync;
    unsigned char no_error = 1;

    poly = generic_gf_poly_create(field, received, length);
    if (poly == NULL)
        return -1;

    sync = (unsigned int *)mem_alloc(sizeof(unsigned int) * twos);
    if (sync == NULL) {
        generic_gf_poly_release(poly);
        return -1;
    }
    memset(sync, 0, sizeof(unsigned int) * twos);

    for (unsigned int i = 0; i < twos; ++i) {
        unsigned int eval = generic_gf_poly_evaluateAt(poly,
                generic_gf_exp(field, i + field->generator_base));
        sync[twos - 1 - i] = eval;
        if (eval != 0)
            no_error = 0;
    }

    if (no_error) {
        generic_gf_poly_release(poly);
        mem_free(sync);
        return 0;
    }

    int ret = -1;
    struct generic_gf_poly **psigma = NULL;
    unsigned int *error_location = NULL, error_location_length = 0;
    unsigned int *error_magnitudes = NULL;
    struct generic_gf_poly *synd = generic_gf_poly_create(field, sync, twos);
    mem_free(sync);
    struct generic_gf_poly *fbm = generic_gf_build_monomial(field, twos, 1);
    if (synd == NULL || fbm == NULL) {
        goto release_resource;
    }

    psigma = run_euclidean_algorithm(field, fbm, synd, twos);
    if (psigma == NULL)
        goto release_resource;

    error_location = find_error_locations(field, psigma[0], &error_location_length);
    if (error_location == NULL)
        goto release_resource;

    error_magnitudes = find_error_magnitudes(field, psigma[1], error_location, error_location_length);
    if (error_magnitudes == NULL)
        goto release_resource;

    for (unsigned int i = 0; i < error_location_length; ++i) {
        int position = ((int)length) - 1 - ((int)generic_gf_log(field, error_location[i]));
        if (position < 0)
            goto release_resource;

        received[position] = generic_gf_add(received[position], error_magnitudes[i]);
    }

    ret = 0;
release_resource:
    if (error_location != NULL)
        mem_free(error_location);
    if (error_magnitudes != NULL)
        mem_free(error_magnitudes);
    if (psigma != NULL) {
        generic_gf_poly_release(psigma[0]);
        generic_gf_poly_release(psigma[1]);
        mem_free(psigma);
    }
    generic_gf_poly_release(synd);
    generic_gf_poly_release(fbm);
    generic_gf_poly_release(poly);

    return ret;
}
