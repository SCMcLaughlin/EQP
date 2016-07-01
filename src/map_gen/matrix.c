
#include "matrix.h"
#include "map_gen.h"

static float matrix_to_radians(float degrees)
{
    return degrees * 3.14159f / 180.0f;
}

Matrix matrix_mult(Matrix* restrict a, Matrix* restrict b)
{
    Matrix ret;
    float* restrict m = a->m;
    float* restrict o = b->m;
    
    ret.m[ 0] = m[ 0] * o[ 0] + m[ 4] * o[ 1] + m[ 8] * o[ 2] + m[12] * o[ 3];
    ret.m[ 1] = m[ 1] * o[ 0] + m[ 5] * o[ 1] + m[ 9] * o[ 2] + m[13] * o[ 3];
    ret.m[ 2] = m[ 2] * o[ 0] + m[ 6] * o[ 1] + m[10] * o[ 2] + m[14] * o[ 3];
    ret.m[ 3] = m[ 3] * o[ 0] + m[ 7] * o[ 1] + m[11] * o[ 2] + m[15] * o[ 3];
    
    ret.m[ 4] = m[ 0] * o[ 4] + m[ 4] * o[ 5] + m[ 8] * o[ 6] + m[12] * o[ 7];
    ret.m[ 5] = m[ 1] * o[ 4] + m[ 5] * o[ 5] + m[ 9] * o[ 6] + m[13] * o[ 7];
    ret.m[ 6] = m[ 2] * o[ 4] + m[ 6] * o[ 5] + m[10] * o[ 6] + m[14] * o[ 7];
    ret.m[ 7] = m[ 3] * o[ 4] + m[ 7] * o[ 5] + m[11] * o[ 6] + m[15] * o[ 7];
    
    ret.m[ 8] = m[ 0] * o[ 8] + m[ 4] * o[ 9] + m[ 8] * o[10] + m[12] * o[11];
    ret.m[ 9] = m[ 1] * o[ 8] + m[ 5] * o[ 9] + m[ 9] * o[10] + m[13] * o[11];
    ret.m[10] = m[ 2] * o[ 8] + m[ 6] * o[ 9] + m[10] * o[10] + m[14] * o[11];
    ret.m[11] = m[ 3] * o[ 8] + m[ 7] * o[ 9] + m[11] * o[10] + m[15] * o[11];
    
    ret.m[12] = m[ 0] * o[12] + m[ 4] * o[13] + m[ 8] * o[14] + m[12] * o[15];
    ret.m[13] = m[ 1] * o[12] + m[ 5] * o[13] + m[ 9] * o[14] + m[13] * o[15];
    ret.m[14] = m[ 2] * o[12] + m[ 6] * o[13] + m[10] * o[14] + m[14] * o[15];
    ret.m[15] = m[ 3] * o[12] + m[ 7] * o[13] + m[11] * o[14] + m[15] * o[15];
    
    return ret;
}

static Matrix matrix_angle_y(float angle)
{
    float cosY;
    float sinY;
    Matrix m;
    
    angle   = matrix_to_radians(angle);
    cosY    = cos(angle);
    sinY    = sin(angle);

    m.m[ 0] = cosY;
    m.m[ 1] = 0;
    m.m[ 2] = -sinY;
    m.m[ 3] = 0;

    m.m[ 4] = 0;
    m.m[ 5] = 1;
    m.m[ 6] = 0;
    m.m[ 7] = 0;

    m.m[ 8] = sinY;
    m.m[ 9] = 0;
    m.m[10] = cosY;
    m.m[11] = 0;

    m.m[12] = 0;
    m.m[13] = 0;
    m.m[14] = 0;
    m.m[15] = 1;

    return m;
}

static Matrix matrix_angle_z(float angle)
{
    float cosZ;
    float sinZ;
    Matrix m;
    
    angle   = matrix_to_radians(angle);
    cosZ    = cos(angle);
    sinZ    = sin(angle);

    m.m[ 0] = cosZ;
    m.m[ 1] = sinZ;
    m.m[ 2] = 0;
    m.m[ 3] = 0;

    m.m[ 4] = -sinZ;
    m.m[ 5] = cosZ;
    m.m[ 6] = 0;
    m.m[ 7] = 0;

    m.m[ 8] = 0;
    m.m[ 9] = 0;
    m.m[10] = 1;
    m.m[11] = 0;

    m.m[12] = 0;
    m.m[13] = 0;
    m.m[14] = 0;
    m.m[15] = 1;

    return m;
}

Matrix matrix_angle_yz(float yAngle, float zAngle)
{
    Matrix y = matrix_angle_y(yAngle);
    Matrix z = matrix_angle_z(zAngle);
    
    return matrix_mult(&y, &z);
}

Matrix matrix_scale(float v)
{
    Matrix m;
    
    m.m[ 0] = v;
    m.m[ 1] = 0;
    m.m[ 2] = 0;
    m.m[ 3] = 0;
    
    m.m[ 4] = 0;
    m.m[ 5] = v;
    m.m[ 6] = 0;
    m.m[ 7] = 0;
    
    m.m[ 8] = 0;
    m.m[ 9] = 0;
    m.m[10] = v;
    m.m[11] = 0;
    
    m.m[12] = 0;
    m.m[13] = 0;
    m.m[14] = 0;
    m.m[15] = 1;
    
    return m;
}

void matrix_set_translation(Matrix* m, float x, float y, float z)
{
    m->m[12] = x;
    m->m[13] = y;
    m->m[14] = z;
}

void matrix_transform_vertex(Matrix* restrict matrix, Vertex* restrict dst, const Vertex* restrict src)
{
    float* restrict m = matrix->m;
    float x = src->x;
    float y = src->y;
    float z = src->z;
    
    dst->x = x * m[ 0] + y * m[ 4] + z * m[ 8] + m[12];
    dst->y = x * m[ 1] + y * m[ 5] + z * m[ 9] + m[13];
    dst->z = x * m[ 2] + y * m[ 6] + z * m[10] + m[14];
}
