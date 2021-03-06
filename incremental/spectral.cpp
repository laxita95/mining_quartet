#include "spectral.h"
#include "k_means.hpp"
#include <fstream>

ostream& operator<<(ostream& o, const point& p) {
    o << "(" << p.first << ", " << p.second << ")";
    return o;
}

double similar_euclidean(const point& p, const point& q) {
    return  (double) pow(int(p.first) - int(q.first), 2) + 
            (double) pow(int(p.second) - int(q.second) , 2);
}

similarity_function build_euclidean_metric(double sigma2) {
    return [&sigma2](const point& p, const point& q) {
        return exp(-similar_euclidean(p, q)/sigma2);
    };
}

/* Is w among v's k nearest neighbors? */ 
bool is_k_nearest_neighbor(const matrix& S, uint v, uint w, uint k) {
    uint i, n = S.cols(), c = 0;
    double d = S(v, w);
    forn(i, n) if(S(v, i) > d) c++;
    return c <= k;
}

matrix k_nearest_neighbors(const matrix& S, uint k, const vpoint& p) {
    uint i, j, n = S.rows(), c = 0;
    matrix T = matrix::Zero(n, n);
    forn(i, n) {
        forn(j, n) {
            if( is_k_nearest_neighbor(S, i, j, k) || is_k_nearest_neighbor(S, j, i, k)) {
                T(i, j) = S(i, j);                
            } else c++;
        }
    }
    return T;
}

matrix similarity_matrix(const vpoint& points, similarity_function f, uint k) {
    uint n = points.size();
    uint i, j;
    matrix S(n, n);
    forn(i, n) forn(j, n) S(i, j) = f(points[i], points[j]);
    forn(i, n) S(i, i) = 0;
    return k_nearest_neighbors(S, k, points);
}

matrix diagonal(const matrix& S) {
    int i, j, n = S.rows();
    double c;
    matrix D = matrix::Zero(n, n);
    forn(i, n) {
        c = 0;
        forn(j, n) c += S(i, j);
        D(i, i) = c;
    }
    return D;
}

decomposition solve_generalized(const vpoint& points, similarity_function f, uint k) {
    matrix W = similarity_matrix(points, f, k);
    matrix D = diagonal(W);
    GeneralizedSelfAdjointEigenSolver<matrix> solve(D-W, D);
    
    ofstream myfile; // saving Egen vectors to file
    myfile.open("/home/sonalkumar/Desktop/test/AggriInc7/eigenvec1.csv");
    myfile << solve.eigenvectors();
    myfile.close();
    
    //cout << "EigenValue";
    //cout << solve.eigenvalues();
    
    ofstream myVfile; // saving Egen vectors to file
    myVfile.open("/home/sonalkumar/Desktop/test/AggriInc7/eigenval.csv");
    myVfile << solve.eigenvalues();
    myVfile.close();
    
    //cout << "EigenVector";
    //cout << solve.eigenvectors();
    
    return decomposition(solve.eigenvalues(), solve.eigenvectors());
}

vcluster spectral(const vpoint& points, uint k, double threshold, uint neighbors, double sigma2, uint retries) {
    int i, j, n = points.size();
    decomposition&& D = solve_generalized(points, build_euclidean_metric(sigma2), neighbors);
    vector<vector<double>> eigenvectors(n, vector<double>(k));
    forn(i, n) forn(j, int(k)) eigenvectors[i][j] = (D.second)(i, j); 

    return k_means(eigenvectors, k, threshold, retries);
}

vcluster just_k_means(const vpoint& points, uint k, double threshold, uint retries) {
    int i, n = points.size();
    vector<vector<int>> values(n, vector<int>(2));
    forn(i, n) {
        values[i][0] = points[i].first;
        values[i][1] = points[i].second;
    }
    
    return k_means(values, k, threshold, retries);
}
