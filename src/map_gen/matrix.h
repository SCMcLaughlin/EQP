
#ifndef EQP_MATRIX_H
#define EQP_MATRIX_H

#include "define.h"

STRUCT_DECLARE(Vertex);

STRUCT_DEFINE(Matrix)
{
    float m[16];
};

Matrix  matrix_mult(Matrix* restrict a, Matrix* restrict b);
Matrix  matrix_angle_yz(float yAngle, float zAngle);
Matrix  matrix_scale(float scale);

void    matrix_set_translation(Matrix* m, float x, float y, float z);
void    matrix_transform_vertex(Matrix* restrict m, Vertex* restrict dst, const Vertex* restrict src);

#endif//EQP_MATRIX_H
