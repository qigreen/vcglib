#ifndef __VCGLIB_LINALGEBRA_H
#define __VCGLIB_LINALGEBRA_H

#include <vcg/math/matrix44.h>

namespace vcg
{
	/** \addtogroup math */
	/* @{ */

	/*!
	*
	*/
	template< typename TYPE >
	static void JacobiRotate(Matrix44<TYPE> &A, TYPE s, TYPE tau, int i,int j,int k,int l)
	{
		TYPE g=A[i][j];
		TYPE h=A[k][l];
		A[i][j]=g-s*(h+g*tau);
		A[k][l]=h+s*(g-h*tau); 
	};

	/*!
	*	Computes all eigenvalues and eigenvectors of a real symmetric matrix .
	*	On output, elements of the input matrix above the diagonal are destroyed. 
	* \param d  returns the eigenvalues of a. 
	* \param v  is a matrix whose columns contain, the normalized eigenvectors 
	* \param nrot returns the number of Jacobi rotations that were required. 
	*/
	template <typename TYPE>
		static void Jacobi(Matrix44<TYPE> &w, Point4<TYPE> &d, Matrix44<TYPE> &v, int &nrot) 
	{ 
		int j,iq,ip,i; 
		//assert(w.IsSymmetric());
		TYPE tresh, theta, tau, t, sm, s, h, g, c; 
		Point4<TYPE> b, z; 

		v.SetIdentity();

		for (ip=0;ip<4;++ip)			//Initialize b and d to the diagonal of a. 
		{		
			b[ip]=d[ip]=w[ip][ip]; 
			z[ip]=0.0;							//This vector will accumulate terms of the form tapq as in equation (11.1.14). 
		}
		nrot=0; 
		for (i=0;i<50;i++) 
		{ 
			sm=0.0; 
			for (ip=0;ip<3;++ip)		// Sum off diagonal elements
			{
				for (iq=ip+1;iq<4;++iq) 
					sm += fabs(w[ip][iq]); 
			} 
			if (sm == 0.0)					//The normal return, which relies on quadratic convergence to machine underflow. 
			{				
				return; 
			} 
			if (i < 4) 	
				tresh=0.2*sm/(4*4); //...on the first three sweeps. 
			else 		
				tresh=0.0;				//...thereafter. 
			for (ip=0;ip<4-1;++ip) 
			{  
				for (iq=ip+1;iq<4;iq++) 
				{ 
					g=100.0*fabs(w[ip][iq]); 
					//After four sweeps, skip the rotation if the off-diagonal element is small. 
					if(i>4 && (float)(fabs(d[ip])+g) == (float)fabs(d[ip]) && (float)(fabs(d[iq])+g) == (float)fabs(d[iq])) 
						w[ip][iq]=0.0; 
					else if (fabs(w[ip][iq]) > tresh) 
					{ 
						h=d[iq]-d[ip]; 
						if ((float)(fabs(h)+g) == (float)fabs(h)) 
							t=(w[ip][iq])/h; //t =1/(2#) 
						else 
						{ 
							theta=0.5*h/(w[ip][iq]); //Equation (11.1.10). 
							t=1.0/(fabs(theta)+sqrt(1.0+theta*theta)); 
							if (theta < 0.0) t = -t; 
						} 
						c=1.0/sqrt(1+t*t); 
						s=t*c; 
						tau=s/(1.0+c); 
						h=t*w[ip][iq]; 
						z[ip] -= h; 
						z[iq] += h; 
						d[ip] -= h; 
						d[iq] += h; 
						w[ip][iq]=0.0; 
						for (j=0;j<=ip-1;j++) { //Case of rotations 1 <= j < p. 
							JacobiRotate<TYPE>(w,s,tau,j,ip,j,iq) ;
						} 
						for (j=ip+1;j<=iq-1;j++) { //Case of rotations p < j < q. 
							JacobiRotate<TYPE>(w,s,tau,ip,j,j,iq);
						} 
						for (j=iq+1;j<4;j++) { //Case of rotations q< j <= n. 
							JacobiRotate<TYPE>(w,s,tau,ip,j,iq,j);
						} 
						for (j=0;j<4;j++) { 
							JacobiRotate<TYPE>(v,s,tau,j,ip,j,iq);
						} 
						++nrot; 
					} 
				} 
			} 
			for (ip=0;ip<4;ip++) 
			{ 
				b[ip] += z[ip]; 
				d[ip]=b[ip]; //Update d with the sum of ta_pq , 
				z[ip]=0.0; //and reinitialize z. 
			} 
		} 
	};

	
	// Computes (a^2 + b^2)^(1/2) without destructive underflow or overflow.
	template <typename TYPE>
	inline static TYPE pythagora(TYPE a, TYPE b)
	{
		TYPE abs_a = fabs(a);
		TYPE abs_b = fabs(b);
		if (abs_a > abs_b) 
			return abs_a*sqrt(1.0+sqr(abs_b/abs_a));
		else 
			return (abs_b == 0.0 ? 0.0 : abs_b*sqrt(1.0+sqr(abs_a/abs_b)));
	};

	template <typename TYPE>
	inline static TYPE sign(TYPE a, TYPE b)
	{
		return (b >= 0.0 ? fabs(a) : -fabs(a));
	};

	template <typename TYPE>
	inline static TYPE sqr(TYPE a)
	{
		TYPE sqr_arg = a;
		return (sqr_arg == 0 ? 0 : sqr_arg*sqr_arg);
	}


	/*!
	*	Given a matrix <I>A<SUB>m�n</SUB></I>, this routine computes its singular value decomposition,
	*	i.e. <I>A=U�W�V<SUP>T</SUP></I>. The matrix <I>A</I> will be destroyed!
	*	(This is the implementation described in <I>Numerical Recipies</I>).
	*	\param A	the matrix to be decomposed
	*	\param W	the diagonal matrix of singular values <I>W</I>, stored as a vector <I>W[1...N]</I>
	*	\param V	the matrix <I>V</I> (not the transpose <I>V<SUP>T</SUP></I>)
	*	\param max_iters	max iteration number (default = 30).
	*	\return 
	*/
	template <typename MATRIX_TYPE>
		static bool SingularValueDecomposition(MATRIX_TYPE &A, typename MATRIX_TYPE::ScalarType *W, MATRIX_TYPE &V, const int max_iters = 30)
	{
		typedef typename MATRIX_TYPE::ScalarType ScalarType;
		int m = (int) A.RowsNumber();
		int n = (int) A.ColumnsNumber();
		int flag,i,its,j,jj,k,l,nm;
		double anorm, c, f, g, h, s, scale, x, y, z, *rv1;
		bool convergence = true;

		rv1 = new double[n];
		g = scale = anorm = 0; 
		// Householder reduction to bidiagonal form.
		for (i=0; i<n; i++) 
		{
			l = i+1;
			rv1[i] = scale*g;
			g = s = scale = 0.0;
			if (i < m)
			{
				for (k = i; k<m; k++) 
					scale += fabs(A[k][i]);
				if (scale) 
				{
					for (k=i; k<m; k++) 
					{
						A[k][i] /= scale;
						s += A[k][i]*A[k][i];
					}
					f=A[i][i];
					g = -sign<double>( sqrt(s), f );
					h = f*g - s;
					A[i][i]=f-g;
					for (j=l; j<n; j++) 
					{
						for (s=0.0, k=i; k<m; k++) 
							s += A[k][i]*A[k][j];
						f = s/h;
						for (k=i; k<m; k++) 
							A[k][j] += f*A[k][i];
					}
					for (k=i; k<m; k++) 
						A[k][i] *= scale;
				}
			}
			W[i] = scale *g;
			g = s = scale = 0.0;
			if (i < m && i != (n-1)) 
			{
				for (k=l; k<n; k++) 
					scale += fabs(A[i][k]);
				if (scale) 
				{
					for (k=l; k<n; k++) 
					{
						A[i][k] /= scale;
						s += A[i][k]*A[i][k];
					}
					f = A[i][l];
					g = -sign<double>(sqrt(s),f);
					h = f*g - s;
					A[i][l] = f-g;
					for (k=l; k<n; k++) 
						rv1[k] = A[i][k]/h;
					for (j=l; j<m; j++) 
					{
						for (s=0.0, k=l; k<n; k++) 
							s += A[j][k]*A[i][k];
						for (k=l; k<n; k++) 
							A[j][k] += s*rv1[k];
					}
					for (k=l; k<n; k++) 
						A[i][k] *= scale;
				}
			}
			anorm=math::Max( anorm, (fabs(W[i])+fabs(rv1[i])) );
		}
		// Accumulation of right-hand transformations.
		for (i=(n-1); i>=0; i--) 
		{ 
			//Accumulation of right-hand transformations.
			if (i < (n-1)) 
			{
				if (g) 
				{
					for (j=l; j<n;j++) //Double division to avoid possible underflow.
						V[j][i]=(A[i][j]/A[i][l])/g;
					for (j=l; j<n; j++) 
					{
						for (s=0.0, k=l; k<n; k++) 
							s += A[i][k] * V[k][j];
						for (k=l; k<n; k++) 
							V[k][j] += s*V[k][i];
					}
				}
				for (j=l; j<n; j++) 
					V[i][j] = V[j][i] = 0.0;
			}
			V[i][i] = 1.0;
			g = rv1[i];
			l = i;
		}
		// Accumulation of left-hand transformations.
		for (i=math::Min(m,n)-1; i>=0; i--) 
		{
			l = i+1;
			g = W[i];
			for (j=l; j<n; j++) 
				A[i][j]=0.0;
			if (g) 
			{
				g = 1.0/g;
				for (j=l; j<n; j++) 
				{
					for (s=0.0, k=l; k<m; k++) 
						s += A[k][i]*A[k][j];
					f = (s/A[i][i])*g;
					for (k=i; k<m; k++) 
						A[k][j] += f*A[k][i];
				}
				for (j=i; j<m; j++) 
					A[j][i] *= g;
			} 
			else 
				for (j=i; j<m; j++) 
					A[j][i] = 0.0;
			++A[i][i];
		}
		// Diagonalization of the bidiagonal form: Loop over
		// singular values, and over allowed iterations.
		for (k=(n-1); k>=0; k--) 
		{ 
			for (its=1; its<=max_iters; its++) 
			{
				flag=1;
				for (l=k; l>=0; l--) 
				{ 
					// Test for splitting.
					nm=l-1; 
					// Note that rv1[1] is always zero.
					if ((double)(fabs(rv1[l])+anorm) == anorm) 
					{
						flag=0;
						break;
					}
					if ((double)(fabs(W[nm])+anorm) == anorm) 
						break;
				}
				if (flag) 
				{
					c=0.0;  //Cancellation of rv1[l], if l > 1.
					s=1.0;
					for (i=l ;i<=k; i++) 
					{
						f = s*rv1[i];
						rv1[i] = c*rv1[i];
						if ((double)(fabs(f)+anorm) == anorm) 
							break;
						g = W[i];
						h = pythagora<double>(f,g);
						W[i] = h;
						h = 1.0/h;
						c = g*h;
						s = -f*h;
						for (j=0; j<m; j++) 
						{
							y = A[j][nm];
							z = A[j][i];
							A[j][nm]	= y*c + z*s;
							A[j][i]		= z*c - y*s;
						}
					}
				}
				z = W[k];
				if (l == k)  //Convergence.
				{ 
					if (z < 0.0) { // Singular value is made nonnegative.
						W[k] = -z;
						for (j=0; j<n; j++) 
							V[j][k] = -V[j][k];
					}
					break;
				}
				if (its == max_iters)
				{
					printf("no convergence in %d SingularValueDecomposition iterations\n", max_iters);
					convergence = false;
				}
				x = W[l]; // Shift from bottom 2-by-2 minor.
				nm = k-1;
				y = W[nm];
				g = rv1[nm];
				h = rv1[k];
				f = ((y-z)*(y+z) + (g-h)*(g+h))/(2.0*h*y);
				g = pythagora<double>(f,1.0);
				f=((x-z)*(x+z) + h*((y/(f+sign(g,f)))-h))/x;
				c=s=1.0; 
				//Next QR transformation:
				for (j=l; j<= nm;j++) 
				{
					i = j+1;
					g = rv1[i];
					y = W[i];
					h = s*g;
					g = c*g;
					z = pythagora<double>(f,h);
					rv1[j] = z;
					c = f/z;
					s = h/z;
					f = x*c + g*s;
					g = g*c - x*s;
					h = y*s;
					y *= c;
					for (jj=0; jj<n; jj++) 
					{
						x = V[jj][j];
						z = V[jj][i];
						V[jj][j] = x*c + z*s;
						V[jj][i] = z*c - x*s;
					}
					z = pythagora<double>(f,h);
					W[j] = z;
					// Rotation can be arbitrary if z = 0.
					if (z) 
					{
						z = 1.0/z;
						c = f*z;
						s = h*z;
					}
					f = c*g + s*y;
					x = c*y - s*g;
					for (jj=0; jj<m; jj++) 
					{
						y = A[jj][j];
						z = A[jj][i];
						A[jj][j] = y*c + z*s;
						A[jj][i] = z*c - y*s;
					}
				}
				rv1[l] = 0.0;
				rv1[k] = f;
				W[k]	 = x;
			}
		}
		delete []rv1;
		return convergence;
	};  


	/*!
	*	Solves A�X = B for a vector X, where A is specified by the matrices <I>U<SUB>m�n</SUB></I>, 
	*	<I>W<SUB>n�1</SUB></I> and <I>V<SUB>n�n</SUB></I> as returned by <CODE>SingularValueDecomposition</CODE>.
	*	No input quantities are destroyed, so the routine may be called sequentially with different b�s.
	*	\param x	is the output solution vector (<I>x<SUB>n�1</SUB></I>)
	*	\param b	is the input right-hand side (<I>b<SUB>n�1</SUB></I>)
	*/
	template <typename MATRIX_TYPE>
		static void SingularValueBacksubstitution(const MATRIX_TYPE												&U,
																							const typename MATRIX_TYPE::ScalarType	*W,
																							const MATRIX_TYPE												&V,
																										typename MATRIX_TYPE::ScalarType	*x,
																							const typename MATRIX_TYPE::ScalarType	*b)
	{
		typedef typename MATRIX_TYPE::ScalarType ScalarType;
		unsigned int jj, j, i;
		ScalarType s;
		ScalarType *tmp	=	new ScalarType[U.ColumnsNumber()];
		for (j=0; j<U.ColumnsNumber(); j++) //Calculate U^T * B.
		{			
			s = 0;
			if (W[j]!=0)							//Nonzero result only if wj is nonzero.
			{ 
				for (i=0; i<U.RowsNumber(); i++) 
					s += U[i][j]*b[i];
				s /= W[j];							//This is the divide by wj .
			}
			tmp[j]=s;
		}
		for (j=0;j<U.ColumnsNumber();j++)	//Matrix multiply by V to get answer.
		{			
			s = 0;
			for (jj=0; jj<U.ColumnsNumber(); jj++) 
				s += V[j][jj]*tmp[jj];
			x[j]=s;
		}
		delete []tmp;
	};

	/*! @} */
}; // end of namespace

#endif
