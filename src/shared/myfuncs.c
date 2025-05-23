#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <zlib.h>
#include <math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include "mystructs.h"
#include "common.h"
#include "myfuncs.h"

// Function to calculate the critical line/points of vector field and define type of critical points
int find_critic_vec(mesh *M1, double *vec, int num_zero, double **zero_ptxyz2, int **type_zero_ele2, int **type_zero_p)
{

	printf("* Start to find critical line/points on given vector field....\n");
	// find element surround a point
	SAFE_FREE(M1->esurp_ptr);
	SAFE_FREE(M1->esurp);
	CHECK_ERROR(save_esurp(M1->npoin, M1->nelem, M1->elems, &M1->esurp, &M1->esurp_ptr, M1->nredge));
	// find element surround an element
	SAFE_FREE(M1->open);
	SAFE_FREE(M1->esure);
	CHECK_ERROR(save_esure(M1->nelem, M1->elems, M1->esurp_ptr, M1->esurp, &M1->esure, &M1->open, M1->nredge));
	// find centeriod of tri3 element
	double *cen;
	CHECK_ERROR(save_centri3(M1->nelem, M1->elems, M1->ptxyz, &cen));
	// find number of neighbour for each element
	int *num_nei = calloc((size_t)M1->nelem, sizeof(int));
	for (int ele = 0; ele < M1->nelem; ele++)
	{
		for (int i = 0; i < 3; i++)
		{
			if (M1->esure[3 * ele + i] >= 0)
				num_nei[ele]++;
		}
	}
	// calc gradient by least-squares method
	double *grad = calloc((size_t)M1->nelem * 9, sizeof(double));

	for (int ele = 0; ele < M1->nelem; ele++)
	{
		if (M1->presmask[ele] == 0)
			continue;
		// create A matrix:
		double *temp_A = calloc((size_t)(num_nei[ele] + 1) * 4, sizeof(double));
		int nn = -1;
		for (int i = 0; i < 3; i++)
		{
			int nei = M1->esure[3 * ele + i];
			if (nei < 0)
				continue;
			nn++;
			// printf("nei: %d\n",nei);
			temp_A[IDX(nn, 0, 4)] = 1;
			for (int j = 0; j < 3; j++)
				temp_A[IDX(nn, j + 1, 4)] = cen[3 * nei + j];
		}
		temp_A[IDX(num_nei[ele], 0, 4)] = 1;
		for (int j = 0; j < 3; j++)
			temp_A[IDX(num_nei[ele], j + 1, 4)] = cen[3 * ele + j];

		// check the column of A matrix (if all element in one column same it should be remove)
		int sing_col[4] = {0, 1, 1, 1};
		for (int j = 1; j < 4; j++)
		{
			double e1 = temp_A[IDX(0, j, 4)];
			for (int i = 0; i < (num_nei[ele] + 1); i++)
			{
				if (e1 != temp_A[IDX(i, j, 4)])
				{
					sing_col[j] = 0;
					break;
				}
			}
		}
		int num_sing = 0;
		for (int i = 0; i < 4; i++)
		{
			if (sing_col[i] == 1)
				num_sing++;
		}
		double *A = calloc((size_t)(num_nei[ele] + 1) * (size_t)(4 - num_sing), sizeof(double));
		int nj = -1;
		for (int j = 0; j < 4; j++)
		{
			if (sing_col[j] == 1)
				continue;
			nj++;
			for (int i = 0; i < (num_nei[ele] + 1); i++)
			{
				A[IDX(i, nj, (4 - num_sing))] = temp_A[IDX(i, j, 4)];
			}
		}

		free(temp_A); // Free temp_A after creating A

		// Part1 : Least-squares method solution for system of linear equations
		double *AT;
		CHECK_ERROR(transpose(A, num_nei[ele] + 1, 4 - num_sing, &AT));
		double *ATA;
		CHECK_ERROR(mat_mult(AT, 4 - num_sing, num_nei[ele] + 1, A, num_nei[ele] + 1, 4 - num_sing, &ATA));
		double *ATA_inv = calloc((size_t)(4 - num_sing) * (size_t)(4 - num_sing), sizeof(double));
		if (inverse_classic_GEPP(ATA, ATA_inv, 4 - num_sing) != 0)
		{
			if (inverse_matrix_SVD(ATA, ATA_inv, 4 - num_sing) != 0)
			{
				printf("can not inverted matrix for ele: %d\n", ele);
				exit(EXIT_FAILURE);
			}
		}

		for (int cor = 0; cor < 3; cor++)
		{
			double *b = calloc((size_t)(num_nei[ele] + 1), sizeof(double));
			// create b matrix:
			int n = -1;
			for (int i = 0; i < 3; i++)
			{
				int nei = M1->esure[3 * ele + i];
				if (nei < 0)
					continue;
				n++;
				// printf("nei: %d\n",nei);
				b[n] = vec[3 * nei + cor];
			}
			b[num_nei[ele]] = vec[3 * ele + cor];

			// Part2 : Least-squares method solution for system of linear equations

			double *ATb;
			CHECK_ERROR(mat_mult(AT, 4 - num_sing, num_nei[ele] + 1, b, num_nei[ele] + 1, 1, &ATb));
			double *s;
			CHECK_ERROR(mat_mult(ATA_inv, 4 - num_sing, 4 - num_sing, ATb, 4 - num_sing, 1, &s));
			// extract gradient
			int ni = -1;
			for (int i = 0; i < 3; i++)
			{
				if (sing_col[i + 1] == 1)
					continue;
				ni++;
				grad[9 * ele + 3 * cor + i] = s[ni + 1];
			}
			free(ATb);
			free(s);
			free(b);
		}
		free(A);
		free(AT);
		free(ATA);
		free(ATA_inv);
	}
	// Intrapolate the nodal vector field
	double *ssmax_p = calloc((size_t)M1->npoin * 3, sizeof(double));
	for (int pt = 0; pt < M1->npoin; pt++)
	{
		int nei;
		int count = 0;
		for (int i = M1->esurp_ptr[pt + 1]; i < M1->esurp_ptr[pt + 2]; i++)
		{
			nei = M1->esurp[i];
			if (nei < 0)
				continue;
			count++;
			for (int cor = 0; cor < 3; cor++)
			{
				ssmax_p[3 * pt + cor] += vec[3 * nei + cor];
				for (int j = 0; j < 3; j++)
					ssmax_p[3 * pt + cor] += grad[9 * nei + 3 * cor + j] * (M1->ptxyz[3 * pt + j] - cen[3 * nei + j]);
			}
		}
		for (int cor = 0; cor < 3; cor++)
			ssmax_p[3 * pt + cor] /= count;
	}
	// Search for zero magnitude points in each triangle
	double *vector_field_cen = calloc((size_t)M1->nelem * 3, sizeof(double));
	double *zero_ptxyz = calloc((size_t)M1->nelem * 3, sizeof(double));
	int *type_zero = calloc((size_t)M1->nelem, sizeof(int));
	int *type_zero_ele = calloc((size_t)M1->nelem, sizeof(int));
	int n_zero = 0;
	for (int ele = 0; ele < M1->nelem; ele++)
	{
		if (M1->presmask[ele] == 0)
			continue;

		int v0 = M1->elems[3 * ele] - 1;
		int v1 = M1->elems[3 * ele + 1] - 1;
		int v2 = M1->elems[3 * ele + 2] - 1;

		double a[3] = {M1->ptxyz[3 * v0], M1->ptxyz[3 * v0 + 1], M1->ptxyz[3 * v0 + 2]};
		double b[3] = {M1->ptxyz[3 * v1], M1->ptxyz[3 * v1 + 1], M1->ptxyz[3 * v1 + 2]};
		double c[3] = {M1->ptxyz[3 * v2], M1->ptxyz[3 * v2 + 1], M1->ptxyz[3 * v2 + 2]};

		double va[3] = {ssmax_p[3 * v0], ssmax_p[3 * v0 + 1], ssmax_p[3 * v0 + 2]};
		double vb[3] = {ssmax_p[3 * v1], ssmax_p[3 * v1 + 1], ssmax_p[3 * v1 + 2]};
		double vc[3] = {ssmax_p[3 * v2], ssmax_p[3 * v2 + 1], ssmax_p[3 * v2 + 2]};

		double p[3] = {cen[3 * ele], cen[3 * ele + 1], cen[3 * ele + 2]};

		double interp_vec[3];
		interpolate_vector(p, a, b, c, va, vb, vc, interp_vec);

		for (int i = 0; i < 3; i++)
			vector_field_cen[3 * ele + i] = interp_vec[i];

		double zero_point[3];
		if (find_zero_magnitude(a, b, c, va, vb, vc, zero_point))
		{
			// printf("element %d: Found zero magnitude at point (%.4f, %.4f, %.4f)\n", ele, zero_point[0], zero_point[1], zero_point[2]);
			n_zero++;
			for (int i = 0; i < 3; i++)
				zero_ptxyz[3 * (n_zero - 1) + i] = zero_point[i];
			type_zero_ele[ele] = 1;
			// find the type of critical points:
			double matrix[3][3];
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
					matrix[i][j] = grad[9 * ele + 3 * i + j];
			}
			// printf("matrix: \n");
			// for (int i = 0; i < 3; i++)
			// {
			//     printf("%lf %lf %lf\n", matrix[i][0], matrix[i][1], matrix[i][2]);
			// }
			// printf("\n");
			Complex roots[3];
			calc_comp_eigen(matrix, roots);

			int posreal = 0;
			int zeroeign = 0;
			int negreal = 0;
			int zeroreal = 0;
			for (int i = 0; i < 3; i++)
			{

				// printf("eigenvalue: %lf + %lf i\n", roots[i].real, roots[i].imag);
				if (fabs(roots[i].imag) < EPSILON)
				{
					if (roots[i].real > EPSILON)
					{
						posreal++;
					}
					else if (roots[i].real < -EPSILON)
					{
						negreal++;
					}
					else
					{
						zeroeign++;
					}
				}
				else
				{
					if (fabs(roots[i].real) < EPSILON)
						zeroreal++;
				}
			}
			if ((posreal + negreal + zeroeign) == 3)
			{
				if (posreal == (3 - zeroeign))
				{
					type_zero[n_zero - 1] = 1; // source point
											   // type_zero_ele[ele] = 1;
				}
				else if (negreal == (3 - zeroeign))
				{
					type_zero[n_zero - 1] = 2; // sink point
											   // type_zero_ele[ele] = 2;
				}
				else
				{
					type_zero[n_zero - 1] = 3; // saddel point
											   // type_zero_ele[ele] = 3;
				}
			}
			else if (zeroreal == (3 - zeroeign))
			{
				type_zero[n_zero - 1] = 4; // center point
										   // ype_zero_ele[ele] = 4;
			}
			else
			{
				type_zero[n_zero - 1] = 5; // spiral or focus
										   // type_zero_ele[ele] = 5;
			}
			// printf("zeroreal: %d\n", zeroreal);
			// printf("zeroeign: %d\n", zeroeign);
		}
	}

	num_zero = n_zero;
	*type_zero_p = type_zero;
	*type_zero_ele2 = type_zero_ele;
	*zero_ptxyz2 = zero_ptxyz;
	if (num_zero > 0)
	{
		printf("* %d number of critical points find on given vector field.\n", num_zero);
	}
	else
	{
		printf("* Can not find any critical point on given vector field.\n");
	}
	free(cen);	   // Free cen after completing the element loop
	free(num_nei); // Free num_nei after completing the element loop
	free(grad);	   // Free grad after completing the element loop
	free(ssmax_p); // Free ssmax_p after completing nodal field interpolation
	free(vector_field_cen);
	return 0; // successful
}
// Function to normalize a vector
int normalize(double *v)
{
	double magnitude = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (magnitude > 0)
	{
		v[0] /= magnitude;
		v[1] /= magnitude;
		v[2] /= magnitude;
	}
	return 0;
}
// Transpose function
int transpose(double *src, int row, int col, double **dest2)
{
	double *dest = calloc((size_t)row * (size_t)col, sizeof(double));
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			dest[IDX(j, i, row)] = src[IDX(i, j, col)];
		}
	}
	*dest2 = dest;
	return 0;
}
// Matrix multiplication: C = A * B
int mat_mult(double *A, int Arow, int Acol, double *B, int Brow, int Bcol, double **C2)
{
	// Ensure matrix dimensions are compatible for multiplication
	if (Acol != Brow)
	{
		fprintf(stderr, "* ERROR: The number of columns in A should match the number of rows in B\n");
		return 1;
	}

	// Allocate memory for the resulting matrix C
	double *C = calloc((size_t)Arow * (size_t)Bcol, sizeof(double));
	if (!C)
	{
		fprintf(stderr, "* ERROR: Memory allocation failed\n");
		return 1;
	}

	// Perform matrix multiplication
	for (int i = 0; i < Arow; i++)
	{
		for (int j = 0; j < Bcol; j++)
		{
			double sum = 0.0; // Accumulate sum for C[i][j]
			for (int k = 0; k < Acol; k++)
			{
				sum += A[IDX(i, k, Acol)] * B[IDX(k, j, Bcol)];
			}
			C[IDX(i, j, Bcol)] = sum;
		}
	}

	*C2 = C; // Pass the result back to the caller
	return 0;
}
// Function for calculation the ineverse matrix with SVD
// this function use GNU Scientific Library (GSL)
int inverse_matrix_SVD(double *mat, double *mat_inv, int n)
{
	int status;

	gsl_matrix *A = gsl_matrix_alloc((size_t)n, (size_t)n);
	gsl_matrix *A_inv = gsl_matrix_alloc((size_t)n, (size_t)n);

	if (!A || !A_inv)
	{
		fprintf(stderr, "Failed to allocate memory for matrices.\n");
		gsl_matrix_free(A);
		gsl_matrix_free(A_inv);
		return -1;
	}
	// feeding to qsl_matrix struct from double array
	for (size_t i = 0; i < (size_t)n; i++)
	{
		for (size_t j = 0; j < (size_t)n; j++)
		{
			gsl_matrix_set(A, i, j, mat[IDX(i, j, (size_t)n)]);
		}
	}

	gsl_matrix *U = gsl_matrix_alloc((size_t)n, (size_t)n);
	gsl_matrix *V = gsl_matrix_alloc((size_t)n, (size_t)n);
	gsl_vector *S = gsl_vector_alloc((size_t)n);
	gsl_vector *work = gsl_vector_alloc((size_t)n);

	gsl_matrix_memcpy(U, A);

	// Perform SVD
	status = gsl_linalg_SV_decomp(U, V, S, work);
	if (status != 0)
	{
		fprintf(stderr, "SVD decomposition failed.\n");
		return status;
	}

	// Calculate A_inv = V * inv(S) * U^T
	gsl_matrix *S_inv = gsl_matrix_alloc((size_t)n, (size_t)n);
	gsl_matrix_set_zero(S_inv);

	for (size_t i = 0; i < (size_t)n; i++)
	{
		double s = gsl_vector_get(S, i);
		if (s > 1e-10)
		{ // Avoid division by zero for small singular values
			gsl_matrix_set(S_inv, i, i, 1.0 / s);
		}
	}

	gsl_matrix *temp = gsl_matrix_alloc((size_t)n, (size_t)n);
	gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, V, S_inv, 0.0, temp);
	gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, temp, U, 0.0, A_inv);

	// Copy data from A_inv to arr
	for (size_t i = 0; i < (size_t)n; i++)
	{
		for (size_t j = 0; j < (size_t)n; j++)
		{
			mat_inv[IDX(i, j, (size_t)n)] = gsl_matrix_get(A_inv, i, j);
		}
	}

	// Free temporary structures
	gsl_matrix_free(A);
	gsl_matrix_free(A_inv);
	gsl_matrix_free(U);
	gsl_matrix_free(V);
	gsl_vector_free(S);
	gsl_vector_free(work);
	gsl_matrix_free(S_inv);
	gsl_matrix_free(temp);

	return 0; // successful
}
// Function for calculating the invers of square matrix with Guassian Elimination with partial pivoting
int inverse_classic_GEPP(double *matrix, double *inverse, int n)
{
	int i, j, k;
	double temp;

	// Initialize the inverse matrix as an identity matrix
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < n; j++)
		{
			inverse[IDX(i, j, n)] = (i == j) ? 1.0 : 0.0;
		}
	}

	// Apply Gaussian elimination with partial pivoting
	for (i = 0; i < n; i++)
	{
		// Find the pivot element
		double maxElement = fabs(matrix[IDX(i, i, n)]);
		int pivotRow = i;
		for (j = i + 1; j < n; j++)
		{
			if (fabs(matrix[IDX(j, i, n)]) > maxElement)
			{
				maxElement = fabs(matrix[IDX(j, i, n)]);
				pivotRow = j;
			}
		}

		// Check for singularity
		if (fabs(matrix[IDX(pivotRow, i, n)]) < 1e-10)
		{
			// printf("Matrix is singular or nearly singular\n");
			return -1; // Singular matrix
		}

		// Swap rows in both matrix and inverse
		if (pivotRow != i)
		{
			for (k = 0; k < n; k++)
			{
				temp = matrix[IDX(i, k, n)];
				matrix[IDX(i, k, n)] = matrix[IDX(pivotRow, k, n)];
				matrix[IDX(pivotRow, k, n)] = temp;

				temp = inverse[IDX(i, k, n)];
				inverse[IDX(i, k, n)] = inverse[IDX(pivotRow, k, n)];
				inverse[IDX(pivotRow, k, n)] = temp;
			}
		}

		// Scale the pivot row
		temp = matrix[IDX(i, i, n)];
		for (k = 0; k < n; k++)
		{
			matrix[IDX(i, k, n)] /= temp;
			inverse[IDX(i, k, n)] /= temp;
		}

		// Eliminate the current column in other rows
		for (j = 0; j < n; j++)
		{
			if (j != i)
			{
				temp = matrix[IDX(j, i, n)];
				for (k = 0; k < n; k++)
				{
					matrix[IDX(j, k, n)] -= matrix[IDX(i, k, n)] * temp;
					inverse[IDX(j, k, n)] -= inverse[IDX(i, k, n)] * temp;
				}
			}
		}
	}

	return 0; // Successful inversion
}
// Function to calculate the determinant of a square matrix n by n
double determinant(double *matrix, int n)
{
	if (n == 1)
	{
		return matrix[0];
	}
	if (n == 2)
	{
		return (matrix[0] * matrix[3] - matrix[1] * matrix[2]);
	}

	double det = 0;
	double *submatrix = (double *)malloc((size_t)(n - 1) * (size_t)(n - 1) * sizeof(double));

	for (int x = 0; x < n; x++)
	{
		int subi = 0;
		for (int i = 1; i < n; i++)
		{
			int subj = 0;
			for (int j = 0; j < n; j++)
			{
				if (j == x)
					continue;
				submatrix[subi * (n - 1) + subj] = matrix[i * n + j];
				subj++;
			}
			subi++;
		}
		det += (x % 2 == 0 ? 1 : -1) * matrix[x] * determinant(submatrix, n - 1);
	}

	free(submatrix);
	return det;
}
// Compute barycentric coordinates for a point in a triangle in 3D space
void barycentric(double *p, double *a, double *b, double *c, double *u, double *v, double *w)
{
	// Vectors from a to b and a to c
	double v0[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
	double v1[3] = {c[0] - a[0], c[1] - a[1], c[2] - a[2]};
	double v2[3] = {p[0] - a[0], p[1] - a[1], p[2] - a[2]};

	// Dot products
	double d00 = v0[0] * v0[0] + v0[1] * v0[1] + v0[2] * v0[2];
	double d01 = v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
	double d11 = v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2];
	double d20 = v2[0] * v0[0] + v2[1] * v0[1] + v2[2] * v0[2];
	double d21 = v2[0] * v1[0] + v2[1] * v1[1] + v2[2] * v1[2];

	double denom = d00 * d11 - d01 * d01;
	*v = (d11 * d20 - d01 * d21) / denom;
	*w = (d00 * d21 - d01 * d20) / denom;
	*u = 1.0 - *v - *w;
}
// Linear interpolation of the vector field within a 3D triangle
void interpolate_vector(double *p, double *a, double *b, double *c, double *va, double *vb, double *vc, double *result)
{
	double u, v, w;
	barycentric(p, a, b, c, &u, &v, &w);
	result[0] = u * va[0] + v * vb[0] + w * vc[0];
	result[1] = u * va[1] + v * vb[1] + w * vc[1];
	result[2] = u * va[2] + v * vb[2] + w * vc[2];
}
// Search for zero magnitude points within the triangle
int find_zero_magnitude(double *a, double *b, double *c, double *va, double *vb, double *vc, double *zero_point)
{
	int found = 0;
	double step = 0.01; // Step size for grid search
	double min = 10000000000;
	if (min>vector_magnitude(va)) min = vector_magnitude(va); 
	if (min>vector_magnitude(vb)) min = vector_magnitude(vb); 
	if (min>vector_magnitude(vc)) min = vector_magnitude(vc); 
	for (double u = 0; u <= 1; u += step)
	{
		for (double v = 0; v <= 1 - u; v += step)
		{
			double w = 1 - u - v;
			double p[3] = {u * a[0] + v * b[0] + w * c[0], u * a[1] + v * b[1] + w * c[1], u * a[2] + v * b[2] + w * c[2]};
			double interp_vec[3];
			interpolate_vector(p, a, b, c, va, vb, vc, interp_vec);
			double mag = vector_magnitude(interp_vec);
			if (mag < (min/100))
			{ // Threshold for zero magnitude
				zero_point[0] = p[0];
				zero_point[1] = p[1];
				zero_point[2] = p[2];
				found = 1;
				break;
			}
		}
		if (found)
			break;
	}
	return found;
}
double vector_magnitude(double *v)
{
	double mag = 0;
	for (int i = 0; i < 3; i++)
		mag += v[i] * v[i];
	return sqrt(mag);
}
// Function to compute trace of a 3x3 matrix
double trace(double A[3][3])
{
	return A[0][0] + A[1][1] + A[2][2];
}
// Function to compute trace of A^2
double traceA2(double A[3][3])
{
	double A2[3][3] = {0};
	// Compute A^2
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				A2[i][j] += A[i][k] * A[k][j];
			}
		}
	}
	return A2[0][0] + A2[1][1] + A2[2][2];
}
// Function to compute determinant of a 3x3 matrix
double determinant3by3(double A[3][3])
{
	return A[0][0] * (A[1][1] * A[2][2] - A[1][2] * A[2][1]) -
		   A[0][1] * (A[1][0] * A[2][2] - A[1][2] * A[2][0]) +
		   A[0][2] * (A[1][0] * A[2][1] - A[1][1] * A[2][0]);
}
// Function to solve cubic equation using Cardano's method
// Returns the number of real roots and fills the roots array
int solveCubic(double a, double b, double c, Complex roots[3])
{
	double p = b - pow(a, 2) / 3.0;
	double q = 2.0 * pow(a, 3) / 27.0 - a * b / 3.0 + c;
	double discriminant = pow(q / 2.0, 2) + pow(p / 3.0, 3);

	if (fabs(discriminant) < EPSILON)
		discriminant = 0.0;

	if (discriminant > 0)
	{
		double sqrt_disc = sqrt(discriminant);
		double u = cbrt(-q / 2.0 + sqrt_disc);
		double v = cbrt(-q / 2.0 - sqrt_disc);
		roots[0].real = u + v - a / 3.0;
		roots[0].imag = 0.0;
		roots[1].real = -(u + v) / 2.0 - a / 3.0;
		roots[1].imag = (u - v) * sqrt(3.0) / 2.0;
		roots[2].real = roots[1].real;
		roots[2].imag = -roots[1].imag;
		return 1;
	}
	else if (discriminant == 0.0)
	{
		double u = cbrt(-q / 2.0);
		roots[0].real = 2.0 * u - a / 3.0;
		roots[0].imag = 0.0;
		roots[1].real = -u - a / 3.0;
		roots[1].imag = 0.0;
		roots[2].real = roots[1].real;
		roots[2].imag = 0.0;
		return 3;
	}
	else
	{
		double r = sqrt(-pow(p / 3.0, 3));
		double phi = acos(-q / (2.0 * r));
		double t = 2.0 * pow(r, 1.0 / 3.0);
		roots[0].real = t * cos(phi / 3.0) - a / 3.0;
		roots[0].imag = 0.0;
		roots[1].real = t * cos((phi + 2.0 * PI) / 3.0) - a / 3.0;
		roots[1].imag = 0.0;
		roots[2].real = t * cos((phi + 4.0 * PI) / 3.0) - a / 3.0;
		roots[2].imag = 0.0;
		return 3;
	}
}
// Function to calculate complex eigenvalues using characteristic polynomial for 3x3 matrix
int calc_comp_eigen(double A[3][3], Complex roots[3])
{
	double tr = trace(A);
	double trA2 = traceA2(A);
	double det = determinant3by3(A);

	// Polynomial coefficients for lambda^3 - tr * lambda^2 + 0.5 * (tr^2 - trA2) * lambda - det = 0
	double a = tr;
	double b = 0.5 * (tr * tr - trA2);
	double c = -det;

	return solveCubic(a, b, c, roots);
}
double sumarr(double *arr, int size)
{
	double sum = 0.0;
	if (size == 0)
		return sum;
	for (int ele = 0; ele < size; ele++)
	{
		sum += arr[ele];
	}
	return sum;
}
int calc_area_tri3(double *ptxyz, int *elems, int nelem, double **area2)
{
	double p1[3], p2[3], p3[3];
	double u[3], v[3];
	int np1, np2, np3;
	double *area;

	// Allocate memory for area array
	area = (double *)calloc((size_t)nelem, sizeof(*area));
	if (area == NULL)
	{
		fprintf(stderr, "ERROR: Memory allocation failed\n");
		return 1;
	}

	// Loop through each element (triangle)
	for (int ele = 0; ele < nelem; ele++)
	{
		// Get vertex indices (assuming elems is 1-based indexing, hence the -1)
		np1 = elems[3 * ele] - 1;
		np2 = elems[3 * ele + 1] - 1;
		np3 = elems[3 * ele + 2] - 1;

		// Retrieve the coordinates of the vertices
		for (int i = 0; i < 3; i++)
			p1[i] = ptxyz[3 * np1 + i];
		for (int i = 0; i < 3; i++)
			p2[i] = ptxyz[3 * np2 + i];
		for (int i = 0; i < 3; i++)
			p3[i] = ptxyz[3 * np3 + i];

		// Compute vectors u = p2 - p1 and v = p3 - p1
		for (int i = 0; i < 3; i++)
			u[i] = p2[i] - p1[i];
		for (int i = 0; i < 3; i++)
			v[i] = p3[i] - p1[i];

		// Compute the cross product of u and v and find the magnitude
		area[ele] = 0.5 * sqrt(
							  pow(u[1] * v[2] - u[2] * v[1], 2) +
							  pow(u[2] * v[0] - u[0] * v[2], 2) +
							  pow(u[0] * v[1] - u[1] * v[0], 2));

		// Check for invalid (non-positive) area values
		if (area[ele] <= 0)
		{
			fprintf(stderr, "ERROR: the area of ele[%d] : %lf\n", ele, area[ele]);
			free(area); // Free allocated memory in case of error
			return 1;
		}
	}

	// Assign the calculated areas to the output pointer
	*area2 = area;
	printf("* area of each tri3 calculated!\n");
	return 0;
}
int check_winding_order(int nelem, int *elems, double *ptxyz)
{
	double p1[3], p2[3], p3[3], u[3], v[3], normalv[3], face_center[3], outward_check[3];
	double bbox_center[3] = {0, 0, 0};
	double bbox_min[3] = {1e9, 1e9, 1e9};
	double bbox_max[3] = {-1e9, -1e9, -1e9};

	// Calculate the bounding box of the entire mesh
	for (int i = 0; i < nelem * 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			double pt = ptxyz[3 * (elems[i] - 1) + j];
			if (pt < bbox_min[j])
				bbox_min[j] = pt;
			if (pt > bbox_max[j])
				bbox_max[j] = pt;
		}
	}

	// Calculate the center of the bounding box
	for (int j = 0; j < 3; j++)
		bbox_center[j] = (bbox_min[j] + bbox_max[j]) / 2.0;

	// Iterate over all elements (triangles)
	for (int ele = 0; ele < nelem; ele++)
	{
		// Get the three points of the triangle
		for (int i = 0; i < 3; i++)
			p1[i] = ptxyz[3 * (elems[3 * ele] - 1) + i];
		for (int i = 0; i < 3; i++)
			p2[i] = ptxyz[3 * (elems[3 * ele + 1] - 1) + i];
		for (int i = 0; i < 3; i++)
			p3[i] = ptxyz[3 * (elems[3 * ele + 2] - 1) + i];

		// Compute vectors u and v
		for (int i = 0; i < 3; i++)
			u[i] = p2[i] - p1[i];
		for (int i = 0; i < 3; i++)
			v[i] = p3[i] - p1[i];

		// Compute the normal using the cross product
		normalv[0] = u[1] * v[2] - u[2] * v[1];
		normalv[1] = u[2] * v[0] - u[0] * v[2];
		normalv[2] = u[0] * v[1] - u[1] * v[0];

		// Normalize the normal vector
		double mag = sqrt(normalv[0] * normalv[0] + normalv[1] * normalv[1] + normalv[2] * normalv[2]);
		if (mag == 0)
		{
			fprintf(stderr, "Zero magnitude normal vector at element %d!\n", ele);
			return -1;
		}
		for (int i = 0; i < 3; i++)
			normalv[i] /= mag;

		// Compute the face center
		for (int i = 0; i < 3; i++)
			face_center[i] = (p1[i] + p2[i] + p3[i]) / 3.0;

		// Vector from the bounding box center to the face center
		for (int i = 0; i < 3; i++)
			outward_check[i] = face_center[i] - bbox_center[i];

		// Check if the normal points inward or outward
		double dot_product = 0;
		for (int i = 0; i < 3; i++)
			dot_product += normalv[i] * outward_check[i];

		// If dot product is negative, the normal points inward, so flip the winding order
		if (dot_product < 0)
		{
			// Swap p2 and p3 to flip the winding
			int temp = elems[3 * ele + 1];
			elems[3 * ele + 1] = elems[3 * ele + 2];
			elems[3 * ele + 2] = temp;
			printf("Flipped winding order for element %d.\n", ele);
		}
	}

	printf("Winding order check complete.\n");
	return 0;
}
// Function to perform the Jacobi method to find eigenvalues and eigenvectors
int jacobiMethod(int nelem, double *matrix, double **eigenvalues1, double **eigenvectors1)
{
	int e = 0;
	double *eigenvalues, *eigenvectors;
	eigenvalues = calloc(3 * (size_t)nelem, sizeof(*eigenvalues));
	eigenvectors = calloc(9 * (size_t)nelem, sizeof(*eigenvectors));
	for (int ele = 0; ele < nelem; ele++)
	{
		// Initialize eigenvectors to the identity matrix
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				eigenvectors[9 * ele + 3 * i + j] = (i == j) ? 1.0 : 0.0;
			}
		}

		for (int iter = 0; iter < MAX_ITER; iter++)
		{
			// Find the largest off-diagonal element
			int p = 0, q = 1;
			double max = fabs(matrix[9 * ele + 1]);
			for (int i = 0; i < 3; i++)
			{
				for (int j = i + 1; j < 3; j++)
				{
					if (fabs(matrix[9 * ele + 3 * i + j]) > max)
					{
						max = fabs(matrix[9 * ele + 3 * i + j]);
						p = i;
						q = j;
					}
				}
			}

			// If the largest off-diagonal element is below the threshold, we consider the matrix diagonalized
			if (max < EPSILON)
				break;

			// Calculate the Jacobi rotation
			double theta = 0.5 * atan2(2 * matrix[9 * ele + 3 * p + q], matrix[9 * ele + 3 * q + q] - matrix[9 * ele + 3 * p + p]);
			double cosTheta = cos(theta);
			double sinTheta = sin(theta);

			// Update the matrix elements using the rotation
			double a_pp = cosTheta * cosTheta * matrix[9 * ele + 3 * p + p] + sinTheta * sinTheta * matrix[9 * ele + 3 * q + q] - 2 * sinTheta * cosTheta * matrix[9 * ele + 3 * p + q];
			double a_qq = sinTheta * sinTheta * matrix[9 * ele + 3 * p + p] + cosTheta * cosTheta * matrix[9 * ele + 3 * q + q] + 2 * sinTheta * cosTheta * matrix[9 * ele + 3 * p + q];
			matrix[9 * ele + 3 * p + p] = a_pp;
			matrix[9 * ele + 3 * q + q] = a_qq;
			matrix[9 * ele + 3 * p + q] = 0.0;
			matrix[9 * ele + 3 * q + p] = 0.0;

			for (int i = 0; i < 3; i++)
			{
				if (i != p && i != q)
				{
					double a_ip = cosTheta * matrix[9 * ele + 3 * i + p] - sinTheta * matrix[9 * ele + 3 * i + q];
					double a_iq = sinTheta * matrix[9 * ele + 3 * i + p] + cosTheta * matrix[9 * ele + 3 * i + q];
					matrix[9 * ele + 3 * i + p] = a_ip;
					matrix[9 * ele + 3 * p + i] = a_ip;
					matrix[9 * ele + 3 * i + q] = a_iq;
					matrix[9 * ele + 3 * q + i] = a_iq;
				}

				// Update eigenvectors
				double temp = cosTheta * eigenvectors[9 * ele + 3 * i + p] - sinTheta * eigenvectors[9 * ele + 3 * i + q];
				eigenvectors[9 * ele + 3 * i + q] = sinTheta * eigenvectors[9 * ele + 3 * i + p] + cosTheta * eigenvectors[9 * ele + 3 * i + q];
				eigenvectors[9 * ele + 3 * i + p] = temp;
			}
		}

		// The diagonal elements are the eigenvalues
		for (int i = 0; i < 3; i++)
		{
			eigenvalues[3 * ele + i] = matrix[9 * ele + 3 * i + i];
		}
	}
	*eigenvalues1 = eigenvalues;
	*eigenvectors1 = eigenvectors;
	return e;
}
// Comparison functions
char *edit_endline_character(char *line, int buffer, FILE *fptr)
{

	char *str;
	int len;

	str = fgets(line, buffer, fptr);
	len = (int)strlen(str);
	if (str[len - 1] == '\n')
		str[len - 1] = '\0';

	return str;
}
char *edit_endlinegz_character(char *line, int buffer, gzFile fptr)
{

	char *str;
	int len;

	str = gzgets(fptr, line, buffer);
	len = (int)strlen(str);
	if (str[len - 1] == '\n')
		str[len - 1] = '\0';

	return str;
}
int compare_int_min(void *a, void *b)
{
	return (*(int *)a < *(int *)b);
}
int compare_int_max(void *a, void *b)
{
	return (*(int *)a > *(int *)b);
}
int compare_double_min(void *a, void *b)
{
	return (*(double *)a < *(double *)b);
}
int compare_double_max(void *a, void *b)
{
	return (*(double *)a > *(double *)b);
}
void *find_extreme(void *array, size_t element_size, size_t num_elements, compare_func comp)
{
	void *extreme = array;

	for (size_t i = 1; i < num_elements; ++i)
	{
		void *current_element = (char *)array + i * element_size;
		if (comp(current_element, extreme))
		{
			extreme = current_element;
		}
	}

	return extreme;
}
// check the start ID of elements in the mesh file
int checkEIDS(int *elems)
{
	size_t int_size = sizeof(*elems) / sizeof(elems[0]);
	// Find min and max for int array
	int *int_min = (int *)find_extreme(elems, sizeof(int), int_size, compare_int_min);
	// printf("--> ID of elements start from %d!\n",*int_min);
	return *int_min;
}
// assign an integer array to a pointer
int assignIntArray(int **ptr, int *arr, int size)
{
	*ptr = (int *)malloc((size_t)size * sizeof(int));
	for (int i = 0; i < size; i++)
	{
		(*ptr)[i] = arr[i];
	}
	return 0;
}
// assign a double array to a pointer
int assignDoubleArray(double **ptr, double *arr, int size)
{
	*ptr = (double *)malloc((size_t)size * sizeof(double));
	for (int i = 0; i < size; i++)
	{
		(*ptr)[i] = arr[i];
	}
	return 0;
}
// check the start # for the element
// make data structure for elements surrounding a point
int save_esurp(int npoin, int nelem, int *elems, int **esurp2, int **esurp_pointer2, int Nredge)
{
	int e = 0;
	// check the start ID element
	if (checkEIDS(elems) != 1)
	{
		fprintf(stderr, "ERROR: The element ID should start from 1 for save_esurp function!\n");
		return -1;
	}
	// define parameter
	int *pointer, *pointer2, *esurp;
	// allocate memory for pointer
	pointer = calloc((size_t)npoin + 2, sizeof(*(pointer)));
	pointer2 = calloc((size_t)npoin + 2, sizeof(*(pointer2)));
	// find the nr of Elements surround each point
	for (int ele = 0; ele < nelem; ele++)
	{
		for (int i = 0; i < Nredge; i++)
			pointer[elems[Nredge * ele + i] + 1]++;
	}

	for (int pt = 1; pt <= npoin; pt++)
		pointer[pt + 1] += pointer[pt];
	for (int pt = 0; pt <= npoin + 1; pt++)
		pointer2[pt] = pointer[pt];
	// allocate memory for the esurp
	esurp = malloc((size_t)pointer[npoin + 1] * sizeof(*(esurp)));

	// find elements surround each point
	for (int ele = 0; ele < nelem; ele++)
	{
		for (int i = 0; i < Nredge; i++)
			esurp[pointer[elems[Nredge * ele + i]]++] = ele;
	}

	// done
	free(pointer);
	*esurp2 = esurp;
	*esurp_pointer2 = pointer2;
	return e;
}
// find neighbor element arround each element from esurp data structure
int *find_nei_elem3D(int *esurp_pointer, int *esurp, int *num_nei, int *open, int *elems, int ele, int ele_p1, int ele_p2, int Nredge)
{

	int elemnum, *p, *order;
	static int *nei; // if nei[0] is -9999 it means that there is problem to find a neighbour element ----------->nei[1] indicate to the number of neighbour in the nei[0]
	nei = calloc((size_t)2, sizeof(*(nei)));
	nei[0] = -9999;
	p = calloc((size_t)Nredge, sizeof(*p));
	order = calloc(2 * (size_t)Nredge, sizeof(*order));
	for (int i = 1; i < Nredge; i++)
	{
		order[2 * i - 1] = i;
		order[2 * i] = i;
	}
	/* find neighbour of ele */
	int *lesps; // list of element around ele_p1 and ele_p2
	int j = 0;
	int nr = esurp_pointer[ele_p1 + 1] + esurp_pointer[ele_p2 + 1] - esurp_pointer[ele_p1] - esurp_pointer[ele_p2];
	lesps = calloc((size_t)nr, sizeof(*lesps));
	for (int i = esurp_pointer[ele_p1]; i < esurp_pointer[ele_p1 + 1]; i++)
	{
		lesps[j] = esurp[i];
		j++;
	}
	for (int i = esurp_pointer[ele_p2]; i < esurp_pointer[ele_p2 + 1]; i++)
	{
		lesps[j] = esurp[i];
		j++;
	}

	for (int i = 0; i < nr; i++)
	{

		elemnum = lesps[i];
		if (num_nei[elemnum] < Nredge && open[elemnum] == 0)
		{

			if (elemnum == ele)
				continue; // checking the same element ID

			for (int k = 0; k < Nredge; k++)
				p[k] = elems[Nredge * elemnum + k];
			for (int k = 0; k < Nredge; k++)
			{
				if (p[order[2 * k]] == ele_p1 && p[order[2 * k + 1]] == ele_p2)
				{
					nei[0] = elemnum;
					nei[1] = k;
					break;
				}
				if (p[order[2 * k]] == ele_p2 && p[order[2 * k + 1]] == ele_p1)
				{
					nei[0] = elemnum;
					nei[1] = k;
					break;
				}
			}
		}
	}
	free(lesps);
	free(order);
	free(p);
	return nei;
}
// make data structure for elements surrounding an element
int save_esure(int nelem, int *elems, int *esurp_pointer, int *esurp, int **esue2, int **open2, int Nredge)
{
	int e = 0;
	// check the start ID element
	if (checkEIDS(elems) != 1)
	{
		fprintf(stderr, "ERROR: The element ID should start from 1 for save_esurp function!\n");
		return -1;
	}
	// parameters
	int *p, *order, *nei, *num_nei, *open;

	/* Allocate space to nei pointer */
	p = calloc((size_t)Nredge, sizeof(*p));
	order = calloc(2 * (size_t)Nredge, sizeof(*order));
	nei = calloc((size_t)Nredge * (size_t)nelem, sizeof(*(nei)));
	num_nei = calloc((size_t)nelem, sizeof(*(num_nei)));
	open = calloc((size_t)nelem, sizeof(*(open)));
	// initializing
	for (int ele = 0; ele < nelem; ele++)
	{
		for (int j = 0; j < Nredge; j++)
			nei[Nredge * ele + j] = -1;
	}
	for (int i = 1; i < Nredge; i++)
	{
		order[2 * i - 1] = i;
		order[2 * i] = i;
	}

	for (int ele = 0; ele < nelem; ele++)
	{
		for (int j = 0; j < Nredge; j++)
			p[j] = elems[Nredge * ele + j];

		// controller condition
		if (num_nei[ele] == Nredge)
			continue;
		for (int j = 0; j < Nredge; j++)
		{
			if (nei[Nredge * ele + j] == -1)
			{
				int *out = find_nei_elem3D(esurp_pointer, esurp, num_nei, open, elems, ele, p[order[2 * j]], p[order[2 * j + 1]], Nredge);
				if (out[0] != -9999)
				{
					nei[Nredge * ele + j] = out[0];
					num_nei[ele]++;
					num_nei[out[0]]++;
					nei[Nredge * out[0] + out[1]] = ele;
				}
				else
				{
					nei[Nredge * ele + j] = -2;
				}
				free(out); // free `out` returned by `find_nei_elem3D` after each use
			}
		}
		// find the element adjacent to hole
		if (num_nei[ele] < Nredge)
		{
			open[ele] = 1;
			// printf("the element %d is near holes\n",ele);
		}
	}
	// Done;
	free(num_nei);
	free(p);
	free(order);
	*esue2 = nei;
	*open2 = open;
	return e;
}
// find Nr of eadge in the mesh and make data structure for adges surrounding an element
int save_fsure(int nelem, int *esure, int **efid2, int *numf, int Nredge)
{
	int e = 0;
	static int *efid;
	int nei, ele, f;
	int num = 0;

	// allocate memory
	efid = calloc((size_t)Nredge * (size_t)nelem, sizeof(*efid));
	for (int i = 0; i < (Nredge * nelem); i++)
		efid[i] = -1;

	for (ele = 0; ele < nelem; ele++)
	{
		for (f = 0; f < Nredge; f++)
		{
			if (efid[Nredge * ele + f] < 0)
			{
				nei = esure[Nredge * ele + f];
				if (nei >= 0)
				{ // this is not boundary face
					efid[Nredge * ele + f] = num;
					for (int j = 0; j < Nredge; j++)
					{
						if (esure[Nredge * nei + j] == ele)
							efid[Nredge * nei + j] = num;
					}
				}
				else
				{ // this is on the boundary face
					efid[Nredge * ele + f] = nei;
				}
				num++;
			}
		}
	}
	printf("* nr face : %d\n", num);
	*numf = num;
	*efid2 = efid;
	return e;
}
// make data structure for points surrounding an edge
int save_psurf(int nelem, int numf, int *elems, int *esure, int **psurf2, int Nredge)
{
	int e = 0;
	int *psurf, *order;
	int *efid, nei, *p;
	int num = 0;

	// allocate memory
	p = calloc((size_t)Nredge, sizeof(*p));
	order = calloc(2 * (size_t)Nredge, sizeof(*order));
	efid = calloc((size_t)Nredge * (size_t)nelem, sizeof(*efid));
	for (int i = 0; i < (Nredge * nelem); i++)
		efid[i] = -1;
	psurf = calloc(2 * (size_t)numf, sizeof(*psurf));

	for (int i = 1; i < Nredge; i++)
	{
		order[2 * i - 1] = i;
		order[2 * i] = i;
	}

	for (int ele = 0; ele < nelem; ele++)
	{
		for (int f = 0; f < Nredge; f++)
		{
			if (efid[Nredge * ele + f] < 0)
			{
				for (int j = 0; j < Nredge; j++)
					p[j] = elems[Nredge * ele + j];
				for (int j = 0; j < Nredge; j++)
				{
					if (f == j)
					{
						psurf[2 * num] = p[order[2 * j]];
						psurf[2 * num + 1] = p[order[2 * j + 1]];
					}
				}
				nei = esure[Nredge * ele + f];
				if (nei >= 0)
				{ // this is not boundary face
					efid[Nredge * ele + f] = num;
					for (int j = 0; j < Nredge; j++)
					{
						if (esure[Nredge * nei + j] == ele)
							efid[Nredge * nei + j] = num;
					}
				}
				num++;
			}
		}
	}

	*psurf2 = psurf;
	free(efid);
	free(p);
	free(order);
	printf("* psurf is done.\n");
	return e;
}
// make data structure for elements surrounding an edge
int save_esurf(int nelem, int *esure, int numf, int **esurf2, int Nredge)
{

	int e = 0;
	int *esurf;
	int *efid, nei, ele, f;
	int num = 0;

	// allocate memory
	efid = calloc((size_t)Nredge * (size_t)nelem, sizeof(*efid));
	for (int i = 0; i < (Nredge * nelem); i++)
		efid[i] = -1;
	esurf = calloc(2 * (size_t)numf, sizeof(*esurf));

	for (ele = 0; ele < nelem; ele++)
	{
		for (f = 0; f < Nredge; f++)
		{
			if (efid[Nredge * ele + f] < 0)
			{
				nei = esure[Nredge * ele + f];
				esurf[2 * num] = ele;
				esurf[2 * num + 1] = nei;
				if (nei >= 0)
				{ // this is not boundary face
					efid[Nredge * ele + f] = num;
					if (esure[Nredge * nei] == ele)
						efid[Nredge * nei] = num;
					if (esure[Nredge * nei + 1] == ele)
						efid[Nredge * nei + 1] = num;
					if (esure[Nredge * nei + 2] == ele)
						efid[Nredge * nei + 2] = num;
				}
				num++;
			}
		}
	}
	free(efid);
	*esurf2 = esurf;
	printf("* esurf is done!\n");
	return e;
}
// find the normal of each element in 3D
int save_normele(int nelem, int *elems, double *ptxyz, double **norm)
{
	int e = 0;
	double *norm2 = NULL;

	// Allocate memory for norm2
	norm2 = calloc(3 * (size_t)nelem, sizeof(*norm2));
	if (norm2 == NULL)
	{
		// Memory allocation failed
		fprintf(stderr, "Memory allocation failed!\n");
		return -1; // Return an error code
	}

	double p1[3] = {0, 0, 0};
	double p2[3] = {0, 0, 0};
	double p3[3] = {0, 0, 0};
	double u[3] = {0, 0, 0};
	double v[3] = {0, 0, 0};

	for (int ele = 0; ele < nelem; ele++)
	{
		for (int i = 0; i < 3; i++)
			p1[i] = ptxyz[3 * (elems[3 * ele] - 1) + i];
		for (int i = 0; i < 3; i++)
			p2[i] = ptxyz[3 * (elems[3 * ele + 1] - 1) + i];
		for (int i = 0; i < 3; i++)
			p3[i] = ptxyz[3 * (elems[3 * ele + 2] - 1) + i];

		for (int i = 0; i < 3; i++)
			u[i] = p2[i] - p1[i];
		for (int i = 0; i < 3; i++)
			v[i] = p3[i] - p2[i];

		norm2[3 * ele] = u[1] * v[2] - u[2] * v[1];
		norm2[3 * ele + 1] = u[2] * v[0] - u[0] * v[2];
		norm2[3 * ele + 2] = u[0] * v[1] - u[1] * v[0];
		double mag = 0;
		for (int i = 0; i < 3; i++)
			mag += norm2[3 * ele + i] * norm2[3 * ele + i];
		if (mag == 0)
		{
			// zero magnitude normal vetor
			fprintf(stderr, "zero magnitude normal vetor at ele : %d!\n", ele);
			return -1; // Return an error code
		}
		for (int i = 0; i < 3; i++)
			norm2[3 * ele + i] = norm2[3 * ele + i] / sqrt(mag);
	}

	*norm = norm2; // Assign the calculated normals to the output pointer
	printf("* normele is done!\n");

	return e;
}
// calculate the center of surface element
int save_centri3(int nelem, int *elems, double *ptxyz, double **cen2)
{
	// Determine if elems starts from 0 or 1
	int min_elems = elems[0];
	for (int i = 0; i < 3 * nelem; i++)
	{
		if (min_elems > elems[i])
			min_elems = elems[i];
	}
	if (min_elems != 1)
	{
		fprintf(stderr, "Problem in elems array: it starts from %d\n", min_elems);
		return 1;
	}
	// Allocate memory for center points
	double *cen = calloc(3 * (size_t)nelem, sizeof(double));
	if (cen == NULL)
	{
		fprintf(stderr, "Memory allocation failed.\n");
		return 1;
	}

	// Calculate the center points
	for (int ele = 0; ele < nelem; ele++)
	{
		int p1 = elems[3 * ele] - 1;
		int p2 = elems[3 * ele + 1] - 1;
		int p3 = elems[3 * ele + 2] - 1;

		for (int i = 0; i < 3; i++)
		{
			cen[3 * ele + i] = (ptxyz[3 * p1 + i] + ptxyz[3 * p2 + i] + ptxyz[3 * p3 + i]) / 3.0;
		}
	}

	*cen2 = cen;
	return 0;
}
// conver mesh from tri3 to other type of mesh
void tri3_to_tri6(mesh *M1, mesh **M2)
{

	// define type of mesh for M2
	strcpy((*M2)->type, "tri");
	(*M2)->nredge = 3;
	(*M2)->nrpts = 6;
	//  define coordinate and elems for M2
	static double *ptxyz2;
	static int npoin2, *elems2, nelem2;
	// allocate memmory
	nelem2 = M1->nelem;
	npoin2 = M1->numf + M1->npoin;
	ptxyz2 = calloc(3 * (size_t)npoin2, sizeof(*ptxyz2));
	elems2 = calloc(6 * (size_t)M1->nelem, sizeof(*elems2));

	// coordinate of all(new+old) points
	for (int i = 0; i < (3 * M1->npoin); i++)
		ptxyz2[i] = M1->ptxyz[i];
	for (int i = 0; i < M1->numf; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			ptxyz2[3 * M1->npoin + 3 * i + j] = (M1->ptxyz[3 * (M1->psurf[2 * i] - 1) + j] + M1->ptxyz[3 * (M1->psurf[2 * i + 1] - 1) + j]) / 2;
		}
	}
	// 	new connectivity
	for (int i = 0; i < M1->nelem; i++)
	{
		elems2[6 * i + 0] = M1->elems[3 * i + 0];
		elems2[6 * i + 1] = M1->elems[3 * i + 1];
		elems2[6 * i + 2] = M1->elems[3 * i + 2];
		elems2[6 * i + 3] = M1->fsure[3 * i + 0] + M1->npoin + 1;
		elems2[6 * i + 4] = M1->fsure[3 * i + 1] + M1->npoin + 1;
		elems2[6 * i + 5] = M1->fsure[3 * i + 2] + M1->npoin + 1;
	}
	// return:
	(*M2)->npoin = npoin2;
	(*M2)->elems = elems2;
	(*M2)->ptxyz = ptxyz2;
	(*M2)->nelem = nelem2;
	// all other data structure same as M1
	(*M2)->Melem = (int *)malloc((size_t)M1->nelem * sizeof(int));
	memcpy((*M2)->Melem, M1->Melem, (size_t)M1->nelem * sizeof(int));
	(*M2)->rpts = (int *)malloc((size_t)M1->npoin * sizeof(int));
	memcpy((*M2)->rpts, M1->rpts, (size_t)M1->npoin * sizeof(int));
	(*M2)->relems = (int *)malloc((size_t)M1->nelem * sizeof(int));
	memcpy((*M2)->relems, M1->relems, (size_t)M1->nelem * sizeof(int));
	//(*M2)->Melem = M1->Melem;	// wall charectristics from .wall file
	//(*M2)->rpts = M1->rpts;		// pointal value of regional mask     --> read labels_srf.zfem
	//(*M2)->relems = M1->relems; // elemental value of regional mask --> approximate
	printf("* the tri3 mesh converted to the tri6 mesh.\n- new npoin: %d\n- new nelem: %d\n", npoin2, nelem2);
}
void tri3_to_quad4(mesh *M1, mesh **M2)
{
	// define type of mesh for M2
	strcpy((*M2)->type, "quad");
	(*M2)->nredge = 4;
	(*M2)->nrpts = 4;
	//  define coordinate and elems for M2
	double *ptxyz2;
	int npoin2, *elems2, nelem2, *Melem2, *relems2;

	// allocate memmory
	npoin2 = M1->npoin + M1->nelem + M1->numf;
	nelem2 = 3 * M1->nelem;
	ptxyz2 = calloc(3 * (size_t)npoin2, sizeof(*ptxyz2));
	elems2 = calloc(4 * (size_t)nelem2, sizeof(*elems2));
	Melem2 = calloc((size_t)nelem2, sizeof(*Melem2));
	relems2 = calloc((size_t)nelem2, sizeof(*relems2));

	// coordinate of all(new+old) points
	for (int i = 0; i < (3 * M1->npoin); i++)
		ptxyz2[i] = M1->ptxyz[i];
	for (int i = 0; i < M1->nelem; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			ptxyz2[3 * M1->npoin + 3 * i + j] = (M1->ptxyz[3 * (M1->elems[3 * i] - 1) + j] + M1->ptxyz[3 * (M1->elems[3 * i + 1] - 1) + j] + M1->ptxyz[3 * (M1->elems[3 * i + 2] - 1) + j]) / 3;
		}
	}
	for (int i = 0; i < M1->numf; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			ptxyz2[3 * M1->npoin + 3 * M1->nelem + 3 * i + j] = (M1->ptxyz[3 * (M1->psurf[2 * i] - 1) + j] + M1->ptxyz[3 * (M1->psurf[2 * i + 1] - 1) + j]) / 2;
		}
	}
	// 	new connectivity
	for (int i = 0; i < M1->nelem; i++)
	{
		// first quadrilateral
		elems2[12 * i + 0 * 4 + 0] = M1->elems[3 * i + 0];
		elems2[12 * i + 0 * 4 + 1] = M1->fsure[3 * i + 0] + M1->nelem + M1->npoin + 1;
		elems2[12 * i + 0 * 4 + 2] = M1->npoin + i + 1;
		elems2[12 * i + 0 * 4 + 3] = M1->fsure[3 * i + 2] + M1->nelem + M1->npoin + 1;
		Melem2[3 * i] = M1->Melem[i];
		relems2[3 * i] = M1->relems[i];

		// second quadrilateral
		elems2[12 * i + 1 * 4 + 0] = M1->elems[3 * i + 1];
		elems2[12 * i + 1 * 4 + 1] = M1->fsure[3 * i + 1] + M1->nelem + M1->npoin + 1;
		elems2[12 * i + 1 * 4 + 2] = M1->npoin + i + 1;
		elems2[12 * i + 1 * 4 + 3] = M1->fsure[3 * i + 0] + M1->nelem + M1->npoin + 1;
		Melem2[3 * i + 1] = M1->Melem[i];
		relems2[3 * i + 1] = M1->relems[i];

		// third quadrilateral
		elems2[12 * i + 2 * 4 + 0] = M1->elems[3 * i + 2];
		elems2[12 * i + 2 * 4 + 1] = M1->fsure[3 * i + 2] + M1->nelem + M1->npoin + 1;
		elems2[12 * i + 2 * 4 + 2] = M1->npoin + i + 1;
		elems2[12 * i + 2 * 4 + 3] = M1->fsure[3 * i + 1] + M1->nelem + M1->npoin + 1;
		Melem2[3 * i + 2] = M1->Melem[i];
		relems2[3 * i + 2] = M1->relems[i];
	}
	// return:
	(*M2)->npoin = npoin2;
	(*M2)->elems = elems2;
	(*M2)->ptxyz = ptxyz2;
	(*M2)->nelem = nelem2;
	// other data structure :
	(*M2)->Melem = Melem2;
	(*M2)->relems = relems2;

	printf("the tri3 mesh converted to the quad4 mesh.\n- new npoin: %d\n- new nelem: %d\n", npoin2, nelem2);
}
int ConverMesh(mesh *M1, mesh *M2, ConvertorFunc Func)
{
	int e = 0;
	// find element surround a point
	CHECK_ERROR(save_esurp(M1->npoin, M1->nelem, M1->elems, &M1->esurp, &M1->esurp_ptr, M1->nredge));
	// find element surround an element
	CHECK_ERROR(save_esure(M1->nelem, M1->elems, M1->esurp_ptr, M1->esurp, &M1->esure, &M1->open, M1->nredge));
	// find Nr of eadge and given id to adges*/
	CHECK_ERROR(save_fsure(M1->nelem, M1->esure, &M1->fsure, &M1->numf, M1->nredge));
	printf(" the number of face : %d \n", M1->numf);
	// find point surround a face*/
	CHECK_ERROR(save_psurf(M1->nelem, M1->numf, M1->elems, M1->esure, &M1->psurf, M1->nredge));
	// convert M1 mesh to M2
	Func(M1, &M2);
	return e;
}
void SCA_int_VTK(FILE *fptr, char *name, int col, int num, void *field)
{
	int *int_field = (int *)field;
	fprintf(fptr, "SCALARS %s int %d\nLOOKUP_TABLE default\n\n", name, col);
	for (int ie = 0; ie < num; ie++)
	{
		fprintf(fptr, "%d\n", int_field[ie]);
	}
}
void SCA_double_VTK(FILE *fptr, char *name, int col, int num, void *field)
{
	double *double_field = (double *)field;
	fprintf(fptr, "SCALARS %s double %d\nLOOKUP_TABLE default\n\n", name, col);
	for (int ie = 0; ie < num; ie++)
	{
		fprintf(fptr, "%lf\n", double_field[ie]);
	}
}
void VEC_double_VTK(FILE *fptr, char *name, int col, int num, void *field)
{
	double *double_field = (double *)field;
	fprintf(fptr, "VECTORS %s double\n", name);
	for (int ie = 0; ie < num; ie++)
	{
		for (int j = 0; j < col; j++)
			fprintf(fptr, "%lf ", double_field[col * ie + j]);
		fprintf(fptr, "\n");
	}
}
void tri3funcVTK(FILE *fptr, int nelem, int *elems)
{
	fprintf(fptr, "CELLS %d %d\n", nelem, 4 * nelem);
	for (int ie = 0; ie < nelem; ie++)
	{
		fprintf(fptr, "3 %d %d %d\n", elems[3 * ie] - 1, elems[3 * ie + 1] - 1, elems[3 * ie + 2] - 1);
	}
	fprintf(fptr, "\n");

	fprintf(fptr, "CELL_TYPES %d\n", nelem);
	for (int ie = 0; ie < nelem; ie++)
	{
		fprintf(fptr, "5\n");
	}
	fprintf(fptr, "\n");
}
void tri6funcVTK(FILE *fptr, int nelem, int *elems)
{
	fprintf(fptr, "CELLS %d %d\n", nelem, 7 * nelem);
	for (int ie = 0; ie < nelem; ie++)
	{
		fprintf(fptr, "6 %d %d %d %d %d %d\n", elems[6 * ie] - 1, elems[6 * ie + 1] - 1, elems[6 * ie + 2] - 1, elems[6 * ie + 3] - 1, elems[6 * ie + 4] - 1, elems[6 * ie + 5] - 1);
	}
	fprintf(fptr, "\n");

	fprintf(fptr, "CELL_TYPES %d\n", nelem);
	for (int ie = 0; ie < nelem; ie++)
	{
		fprintf(fptr, "22\n");
	}
	fprintf(fptr, "\n");
}
void read_VTK_double(FILE *fptr, int col, int nr, void **field)
{
	void *arr;
	int buffer = 100;
	char line[buffer];
	char *token;
	const char delimiters[] = " \t\n"; // Delimiters: space, tab, and newline
	int nscan;
	arr = malloc((size_t)col * (size_t)nr * sizeof(double));
	for (int iline = 0; iline < nr; iline++)
	{
		if (fgets(line, buffer, fptr) == NULL)
		{
			if (feof(fptr))
			{
				break;
			}
			else
			{
				exit(EXIT_FAILURE);
			}
		}
		nscan = 0;
		// Get the first token
		token = strtok(line, delimiters);

		// Continue getting tokens until NULL is returned
		while (token != NULL)
		{
			// printf("Token: %s\n", token);
			((double *)arr)[iline] = atof(token);
			// sscanf(token, "%lf", &arr[iline]);
			token = strtok(NULL, delimiters);
			nscan++;
		}
		if (nscan != col)
		{
			fprintf(stderr, "ERROR: Incorrect number of coordinates on line %d of POINTS.\n", iline + 1);
			exit(EXIT_FAILURE);
		}
	}
	*field = arr;
}
void read_VTK_int(FILE *fptr, int col, int nr, void **field)
{
	void *arr;
	int buffer = 100;
	char line[buffer];
	char *token;
	const char delimiters[] = " \t\n"; // Delimiters: space, tab, and newline
	int nscan;
	arr = malloc((size_t)col * (size_t)nr * sizeof(int));
	for (int iline = 0; iline < nr; iline++)
	{
		if (fgets(line, buffer, fptr) == NULL)
		{
			if (feof(fptr))
			{
				break;
			}
			else
			{
				exit(EXIT_FAILURE);
			}
		}
		nscan = 0;
		// Get the first token
		token = strtok(line, delimiters);

		// Continue getting tokens until NULL is returned
		while (token != NULL)
		{
			// printf("Token: %s\n", token);
			// sscanf(token, "%d", &arr[iline]);
			((int *)arr)[iline] = atoi(token);
			token = strtok(NULL, delimiters);
			nscan++;
		}
		if (nscan != col)
		{
			fprintf(stderr, "ERROR: Incorrect number of coordinates on line %d of POINTS.\n", iline + 1);
			exit(EXIT_FAILURE);
		}
	}
	*field = arr;
}
int ReadVTK(char *dir, char *filenam, int step, FunctionWithArgs2 *prtfield, int countfield)
{
	int e = 0;
	char num[10];
	sprintf(num, "%d", step);
	char path[500];
	strcpy(path, dir);
	strcat(path, filenam);
	strcat(path, "_");
	strcat(path, num);
	strcat(path, ".vtk");
	/* define File pointer:*/
	FILE *fptr;
	fptr = calloc(1, sizeof(*fptr));
	printf("open file - %s.\n", path);
	/* Opening File */
	fptr = fopen(path, "r");
	if (fptr == NULL)
	{
		fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
		return -1;
	}
	/* Read all lines of the file */
	int buffer = 100;
	char *str;
	char line[buffer];
	int endcount = 0;

	char test1[20], test[20];

	while (1)
	{
		// start reading points:
		str = edit_endline_character(line, buffer, fptr);
		sscanf(str, "%s %s ", test1, test);
		for (int ifield = 0; ifield < countfield; ifield++)
		{
			if (!strcmp(test, prtfield[ifield].name))
			{
				printf("    Reading %s.\n", prtfield[ifield].name);
				/* Read header of field */
				str = edit_endline_character(line, buffer, fptr);
				sscanf(str, "%s", test1);
				if (!strcmp(test1, "LOOKUP_TABLE"))
				{
					str = edit_endline_character(line, buffer, fptr);
					sscanf(str, "%s", test1);
				}
				if (!strcmp(test1, ""))
				{
					str = edit_endline_character(line, buffer, fptr);
				}

				/* Read value of field */
				prtfield[ifield].function(fptr, prtfield[ifield].col, prtfield[ifield].nr, prtfield[ifield].arr);
				endcount += 1;
			}
		}
		if (endcount == countfield)
		{
			printf("  Done Reading all %d fields.\n", countfield);
			break;
		}
	}
	if (fclose(fptr) == EOF)
	{
		// If fclose returns EOF, it means there was an error closing the file
		printf("Error closing %s\n", path);
		return -1;
	}

	return e;
}
int SaveVTK(char *dir, char *filenam, int step, mesh *M, elemVTK elemfunc, FunctionWithArgs elefuncs[], size_t nrelefield, FunctionWithArgs pntfuncs[], size_t nrpntfield)
{
	int e = 0;
	// check the start ID element
	if (checkEIDS(M->elems) != 1)
	{
		fprintf(stderr, "ERROR: The element ID should start from 1 for SaveVTK function!\n");
		return -1;
	}
	char num[10];
	sprintf(num, "%d", step);
	char path[500];
	strcpy(path, dir);
	strcat(path, filenam);
	strcat(path, "_");
	strcat(path, num);
	strcat(path, ".vtk");
	char command[500];
	strcpy(command, "rm ");
	strcat(command, path);
	/* define File pointer:*/
	FILE *fptr;
	fptr = calloc(1, sizeof(*fptr));
	/* Opening File */
	fptr = fopen(path, "w");
	if (fptr == NULL)
	{
		fprintf(stderr, "ERROR: Cannot open file - %s.\n", path);
		return -1;
	}
	/*write the header of file : */
	fprintf(fptr, "# vtk DataFile Version 3.0\n");
	fprintf(fptr, "3D Unstructured Surface Grid  with %s%d mesh type\n", M->type, M->nrpts);
	fprintf(fptr, "ASCII\n\n");
	/*write the position of file : */
	fprintf(fptr, "DATASET UNSTRUCTURED_GRID\n");
	fprintf(fptr, "POINTS %d float\n", M->npoin + M->numExtraPoints);
	for (int ip = 0; ip < M->npoin; ip++)
	{
		fprintf(fptr, "%lf %lf %lf\n", M->ptxyz[3 * ip], M->ptxyz[3 * ip + 1], M->ptxyz[3 * ip + 2]);
	}
	for (int ip = 0; ip < M->numExtraPoints; ip++)
	{
		fprintf(fptr, "%lf %lf %lf\n", M->extra_ptxyz[3 * ip], M->extra_ptxyz[3 * ip + 1], M->extra_ptxyz[3 * ip + 2]);
	}
	fprintf(fptr, "\n");
	/*write the elems and cell type : */
	elemfunc(fptr, M->nelem, M->elems);

	// write SCALER pointal fields in the file:
	if (nrpntfield != 0)
		fprintf(fptr, "POINT_DATA %d\n", M->npoin + M->numExtraPoints);
	for (size_t i = 0; i < nrpntfield; ++i)
	{
		pntfuncs[i].function(fptr, pntfuncs[i].name, pntfuncs[i].col, pntfuncs[i].nr, pntfuncs[i].field); // Call each function with its array and size
	}
	// write SCALER elemental fields in the file:
	if (nrelefield != 0)
		fprintf(fptr, "CELL_DATA %d\n", M->nelem);
	for (size_t i = 0; i < nrelefield; ++i)
	{
		elefuncs[i].function(fptr, elefuncs[i].name, elefuncs[i].col, elefuncs[i].nr, elefuncs[i].field); // Call each function with its array and size
																										  // printf("field %ld done.\n",i);
	}
	if (fclose(fptr) == EOF)
	{
		// If fclose returns EOF, it means there was an error closing the file
		printf("Error closing %s\n", path);
		return -1;
	}
	printf("* wrote %s in the VTK format!\n", path);
	return e;
}
int countline(char *path)
{
	FILE *fp;
	int count = 0; // Initialize line counter
	int c;		   // To store a character read from file

	// Open the file in read mode
	fp = fopen(path, "r");

	// Check if file exists
	if (fp == NULL)
	{
		printf("Error: Could not open file\n");
		return 1;
	}

	// Read contents of file
	while ((c = fgetc(fp)) != EOF)
	{
		if ((char)c == '\n')
		{ // Increment count if newline character is encountered
			count++;
		}
	}

	// Print the number of lines
	// printf("The number of lines in the file is: %d\n", count);

	return count;
}
// Hash function to convert a string key to an index
unsigned int hash(const char *key)
{
	unsigned int hash = 0;
	while (*key)
	{
		hash = (hash << 5) + (unsigned int)(*key++);
	}
	return hash % TABLE_SIZE;
}
// Custom strdup function if strdup is not available
char *my_strdup(const char *s)
{
	size_t len = strlen(s) + 1;
	char *dup = malloc(len);
	if (dup != NULL)
	{
		memcpy(dup, s, len);
	}
	return dup;
}
// Function to create a new entry
Entry *createEntry(const char *key, const char *value)
{
	Entry *entry = (Entry *)malloc(sizeof(Entry));
	entry->key = my_strdup(key);	 // Duplicate the string
	entry->value = my_strdup(value); // Duplicate the string
	entry->next = NULL;
	return entry;
}
// Function to insert a key-value pair into the hash table
void inserthash(HashTable *table, const char *key, const char *value)
{
	unsigned int index = hash(key);
	Entry *entry = table->entries[index];
	if (entry == NULL)
	{
		// No collision, simply insert the new entry
		table->entries[index] = createEntry(key, value);
	}
	else
	{
		// Handle collision by chaining
		while (entry->next != NULL && strcmp(entry->key, key) != 0)
		{
			entry = entry->next;
		}
		if (strcmp(entry->key, key) == 0)
		{
			// Update the value if the key already exists
			free(entry->value);
			entry->value = my_strdup(value);
		}
		else
		{
			// Add a new entry at the end of the chain
			entry->next = createEntry(key, value);
		}
	}
}
// Function to retrieve a value by key
char *gethash(HashTable *table, const char *key)
{
	unsigned int index = hash(key);
	Entry *entry = table->entries[index];
	while (entry != NULL)
	{
		if (strcmp(entry->key, key) == 0)
		{
			return entry->value;
		}
		entry = entry->next;
	}
	return NULL; // Key not found
}
// Function to free the memory allocated for the hash table
void freeTable(HashTable *table)
{
	for (int i = 0; i < TABLE_SIZE; i++)
	{
		Entry *entry = table->entries[i];
		while (entry != NULL)
		{
			Entry *temp = entry;
			entry = entry->next;
			free(temp->key);
			free(temp->value);
			free(temp);
		}
	}
}
// Function to find the maximum size among arrays
int findMaxSize(int numArrays, int sizes[])
{
	int max = sizes[0];
	for (int i = 1; i < numArrays; i++)
	{
		if (sizes[i] > max)
		{
			max = sizes[i];
		}
	}
	return max;
}
// Function to write multiple arrays with headers (names) to a text file
void saveMultipleArraysToFile(const char *path, int numArrays, void *arrays[], int sizes[], DataType types[], const char *headers[])
{
	FILE *file = fopen(path, "w");

	if (file == NULL)
	{
		printf("Error opening file!\n");
		return;
	}

	// Print headers (array names)
	for (int j = 0; j < numArrays; j++)
	{
		fprintf(file, "%s\t", headers[j]); // Print each header name
	}
	fprintf(file, "\n"); // Newline after headers

	int maxRows = findMaxSize(numArrays, sizes); // Find the maximum length of the arrays

	// Loop through each row of the data
	for (int i = 0; i < maxRows; i++)
	{
		// Loop through each array (column)
		for (int j = 0; j < numArrays; j++)
		{
			if (i < sizes[j])
			{
				// Print based on the data type of the array
				if (types[j] == INT_TYPE)
				{
					fprintf(file, "%d\t", ((int *)arrays[j])[i]); // Print int
				}
				else if (types[j] == FLOAT_TYPE)
				{
					fprintf(file, "%8.2f\t", ((float *)arrays[j])[i]); // Print float
				}
				else if (types[j] == DOUBLE_TYPE)
				{
					fprintf(file, "%9.2lf\t", ((double *)arrays[j])[i]); // Print double
				}
				else if (types[j] == CHAR_TYPE)
				{
					fprintf(file, "%c\t", ((char *)arrays[j])[i]); // Print single char
				}
				else if (types[j] == STRING_TYPE)
				{
					fprintf(file, "%s\t", ((char **)arrays[j])[i]); // Print string (array of char*)
				}
			}
			else
			{
				fprintf(file, "\t"); // Print an empty tab for missing elements
			}
		}
		fprintf(file, "\n"); // Newline after each row
	}

	fclose(file);
	printf("* Data written successfully to %s!\n", path);
}
// Function to calculate the mean
double calculate_mean(double *arr, int size, double *weight)
{
	double sum = 0.0;
	double sum_weight = 0.0;
	for (int i = 0; i < size; i++)
	{
		sum += arr[i] * weight[i];
		sum_weight += weight[i];
	}
	return sum / sum_weight;
}
// Function to calculate the median
double calculate_median(double arr[], int size)
{
	sort_array(arr, size);

	if (size % 2 == 0)
	{
		return (arr[size / 2 - 1] + arr[size / 2]) / 2.0;
	}
	else
	{
		return arr[size / 2];
	}
}
// Function to find the maximum value
double find_max(double arr[], int size)
{
	double max = arr[0];
	for (int i = 1; i < size; i++)
	{
		if (arr[i] > max)
		{
			max = arr[i];
		}
	}
	return max;
}
// Function to find the minimum value
double find_min(double arr[], int size)
{
	double min = arr[0];
	for (int i = 1; i < size; i++)
	{
		if (arr[i] < min)
		{
			min = arr[i];
		}
	}
	return min;
}
// Function to calculate the standard deviation
double calculate_stddev(double arr[], int size, double mean, double *weight)
{
	double sum = 0.0;
	double sum_weight = 0.0;
	for (int i = 0; i < size; i++)
	{
		sum += weight[i] * pow(arr[i] - mean, 2);
		sum_weight += weight[i];
	}
	return sqrt(sum / sum_weight);
}

// Function to sort the array (used for calculating the median)
void sort_array(double arr[], int size)
{
	for (int i = 0; i < size - 1; i++)
	{
		for (int j = i + 1; j < size; j++)
		{
			if (arr[i] > arr[j])
			{
				double temp = arr[i];
				arr[i] = arr[j];
				arr[j] = temp;
			}
		}
	}
}