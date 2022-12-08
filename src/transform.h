#pragma once

//! \file
//! \brief 3D transform object.
//! Translation, scale, and rotation.

#include "mat4f.h"
#include "quatf.h"
#include "vec3f.h"

//! \brief Transform object.
typedef struct transform_t
{
	vec3f_t translation;
	vec3f_t scale;
	quatf_t rotation;
} transform_t;

//! \brief Make a transform with no rotation, unit scale, and zero position.
void transform_identity(transform_t* transform);

//! \brief Convert a transform to a matrix representation.
void transform_to_matrix(const transform_t* transform, mat4f_t* output);

//! \brief Combine to transforms -- result and t -- and store the output in result.
void transform_multiply(transform_t* result, const transform_t* t);

//! \brief Compute a transform's inverse in translation, scale, and rotation.
void transform_invert(transform_t* transform);

//! \brief Transform a vector by a transform object.
//! \return resulting vector.
vec3f_t transform_transform_vec3(const transform_t* transform, vec3f_t v);
