// Copyright (C) 2017a Amir Vaxman <avaxman@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#ifndef PRINCIPLE_MATCHING_H
#define PRINCIPLE_MATCHING_H
#include <igl/igl_inline.h>
#include <igl/gaussian_curvature.h>
#include <igl/local_basis.h>
#include <igl/triangle_triangle_adjacency.h>
#include <igl/edge_topology.h>
#include <directional/representative_to_raw.h>

#include <Eigen/Core>
#include <vector>
#include <cmath>


namespace directional
{
    // Takes a field in raw form and computes both the principal effort and the consequent principal matching on every edge.
    // Note: equals to rotation_angle*N in N-RoSy fields
    // Important: if the Raw field in not CCW ordered (e.., resulting from all functions X_t_raw() in libdirectional), the result is unpredictable.
    // Input:
    //  V:      #V x 3 vertex coordinates
    //  F:      #F x 3 face vertex indices
    //  EV:     #E x 2 edges to vertices indices
    //  EF:     #E x 2 edges to faces indices
    //  raw:    The directional field, assumed to be ordered CCW, and in xyzxyzxyz...xyz (3*N cols) form. The degree is inferred by the size.
    // Output:
    //  effort: #E principal matching efforts.
    //TODO: also return matching
   IGL_INLINE void principal_matching(const Eigen::MatrixXd& V,
		const Eigen::MatrixXi& F,
		const Eigen::MatrixXi& EV,
		const Eigen::MatrixXi& EF,
		const Eigen::MatrixXi& FE,
		const Eigen::MatrixXd& raw,
		Eigen::VectorXd& effort)
	{

		typedef std::complex<double> Complex;
		using namespace Eigen;
		using namespace std;

		MatrixXd B1, B2, B3;
		igl::local_basis(V, F, B1, B2, B3);

		int N = raw.cols() / 3;

		VectorXcd edgeTransport(EF.rows());  //the difference in the angle representation of edge i from EF(i,0) to EF(i,1)
		MatrixXd edgeVectors(EF.rows(), 3);
		for (int i = 0; i < EF.rows(); i++) {
			if (EF(i, 0) == -1 || EF(i, 1) == -1)
				continue;
			edgeVectors.row(i) = (V.row(EV(i, 1)) - V.row(EV(i, 0))).normalized();
			Complex ef(edgeVectors.row(i).dot(B1.row(EF(i, 0))), edgeVectors.row(i).dot(B2.row(EF(i, 0))));
			Complex eg(edgeVectors.row(i).dot(B1.row(EF(i, 1))), edgeVectors.row(i).dot(B2.row(EF(i, 1))));
			edgeTransport(i) = eg / ef;
		}

		effort = VectorXd::Zero(EF.rows());
		for (int i = 0; i < EF.rows(); i++) {
			if (EF(i, 0) == -1 || EF(i, 1) == -1)
				continue;
			//computing free coefficient effort (a.k.a. Diamanti et al. 2014]
			Complex freeCoeffEffort(1.0, 0.0);
			for (int j = 0; j < N; j++) {
				RowVector3d vecjf = raw.block(EF(i, 0), 3 * j, 1, 3);
				Complex vecjfc = Complex(vecjf.dot(B1.row(EF(i, 0))), vecjf.dot(B2.row(EF(i, 0))));
				RowVector3d vecjg = raw.block(EF(i, 1), 3 * j, 1, 3);
				Complex vecjgc = Complex(vecjg.dot(B1.row(EF(i, 1))), vecjg.dot(B2.row(EF(i, 1))));
				Complex transvecjfc = vecjfc*edgeTransport(i);
				freeCoeffEffort *= vecjgc / transvecjfc;
			}
			effort(i) = arg(freeCoeffEffort);  //arg always returns the principal value
		}

	}

	//representative version
	IGL_INLINE void principal_matching(const Eigen::MatrixXd& V,
		const Eigen::MatrixXi& F,
		const Eigen::MatrixXi& EV,
		const Eigen::MatrixXi& EF,
		const Eigen::MatrixXi& FE,
		const Eigen::MatrixXd& representativeField,
		const int N,
		Eigen::VectorXd& effort)
	{
		Eigen::MatrixXd raw;
		representative_to_raw(V, F, representativeField, N, raw);
		principal_matching(V, F, EV, EF, FE, raw, effort);
	}



	// (V, F) only raw version
	IGL_INLINE void principal_matching(const Eigen::MatrixXd& V,
		const Eigen::MatrixXi& F,
		const Eigen::MatrixXd& raw,
		Eigen::VectorXd& effort)
	{
		Eigen::MatrixXi EV, FE, EF;
		igl::edge_topology(V, F, EV, FE, EF);
		principal_matching(V, F, EV, EF, FE, raw, effort);
	}
    
    // (V,F) only representative version
    IGL_INLINE void principal_matching(const Eigen::MatrixXd& V,
                                    const Eigen::MatrixXi& F,
                                    const Eigen::MatrixXd& representativeField,
                                    const int N,
                                    Eigen::VectorXd& effort)
    {
        Eigen::MatrixXi EV, FE, EF;
        igl::edge_topology(V, F, EV, FE, EF);
        principal_matching(V, F, EV, EF, FE, representativeField, N, effort);
    }
}




#endif


