/* COPYRIGHT (c) 2015 Umut Acar, Arthur Chargueraud, and Michael
 * Rainey
 * All rights reserved.
 *
 * \file geometryio.hpp
 * \brief Geometry data IO
 *
 */

#include "geometrydata.hpp"
#include "cmdline.hpp"
#include "pbbsio.hpp"
#include "utils.hpp"

#ifndef _PCTL_GEOMETRY_IO_H_
#define _PCTL_GEOMETRY_IO_H_

/***********************************************************************/

namespace pasl {
namespace pctl {

template <class intT>
parray<point2d> load_points2d() {
  parray<point2d> points;
  intT n = (intT)deepsea::cmdline::parse_or_default_long("n", 10);
  deepsea::cmdline::dispatcher d;
  d.add("from_file", [&] {
    std::cout << "todo" << std::endl;
    exit(0);
  });
  d.add("by_generator", [&] {
    deepsea::cmdline::dispatcher d;
    d.add("plummer", [&] {
      points = plummer2d(n);
    });
    d.add("uniform", [&] {
      bool inSphere = deepsea::cmdline::parse_or_default_bool("in_sphere", false);
      bool onSphere = deepsea::cmdline::parse_or_default_bool("on_sphere", false);
      points = uniform2d(inSphere, onSphere, n);
    });
    d.dispatch_or_default("generator", "plummer");
  });
  d.dispatch_or_default("load", "by_generator");
  return points;
}

template <class intT, class uintT>
parray<point3d> load_points3d() {
  parray<point3d> points;
  intT n = (intT)deepsea::cmdline::parse_or_default_long("n", 10);
  deepsea::cmdline::dispatcher d;
  d.add("from_file", [&] {
    std::cout << "todo" << std::endl;
    exit(0);
  });
  d.add("by_generator", [&] {
    deepsea::cmdline::dispatcher d;
    d.add("plummer", [&] {
      points = plummer3d<intT,uintT>(n);
    });
    d.add("uniform", [&] {
      bool inSphere = deepsea::cmdline::parse_or_default_bool("in_sphere", false);
      bool onSphere = deepsea::cmdline::parse_or_default_bool("on_sphere", false);
      points = uniform3d<intT,uintT>(inSphere, onSphere, n);
    });
    d.dispatch_or_default("generator", "plummer");
  });
  d.dispatch_or_default("load", "by_generator");
  return points;
}
  
std::ostream& operator<<(std::ostream& out, const point2d& point) {
  out << "point2d(" << point.x << ", " << point.y << ")";
  return out;
}
  
std::ostream& operator<<(std::ostream& out, const point3d& point) {
  out << "point3d(" << point.x << ", " << point.y << ", " << point.z << ")";
  return out;
}
  
namespace benchIO {
  
inline int xToStringLen(point2d a) {
  return xToStringLen(a.x) + xToStringLen(a.y) + 1;
}

inline void xToString(char* s, point2d a) {
  int l = xToStringLen(a.x);
  xToString(s, a.x);
  s[l] = ' ';
  xToString(s+l+1, a.y);
}

inline int xToStringLen(point3d a) {
  return xToStringLen(a.x) + xToStringLen(a.y) + xToStringLen(a.z) + 2;
}

inline void xToString(char* s, point3d a) {
  int lx = xToStringLen(a.x);
  int ly = xToStringLen(a.y);
  xToString(s, a.x);
  s[lx] = ' ';
  xToString(s+lx+1, a.y);
  s[lx+ly+1] = ' ';
  xToString(s+lx+ly+2, a.z);
}

inline int xToStringLen(triangle a) {
  return xToStringLen(a.vertices[0]) + xToStringLen(a.vertices[1]) + xToStringLen(a.vertices[2]) + 2;
}

inline void xToString(char* s, triangle a) {
  int lx = xToStringLen(a.vertices[0]);
  int ly = xToStringLen(a.vertices[1]);
  xToString(s, a.vertices[0]);
  s[lx] = ' ';
  xToString(s+lx+1, a.vertices[1]);
  s[lx+ly+1] = ' ';
  xToString(s+lx+ly+2, a.vertices[2]);
}
  
using namespace std;
  
using intT = int;

string HeaderPoint2d = "pbbs_sequencePoint2d";
string HeaderPoint3d = "pbbs_sequencePoint3d";
string HeaderTriangles = "pbbs_triangles";

template <class pointT>
int writePointsToFile(pointT* P, intT n, char* fname) {
  string Header = (pointT::dim == 2) ? HeaderPoint2d : HeaderPoint3d;
  int r = writeArrayToFile(Header, P, n, fname);
  return r;
}

template <class pointT>
void parsePoints(char** Str, pointT* P, intT n) {
  int d = pointT::dim;
  parray<double> a(n*d, [&] (intT i) {
    return atof(Str[i]);
  });
  parallel_for((intT)0, n, [&] (intT i) {
    P[i] = pointT(a.begin()+(d*i));
  });
}

template <class pointT>
parray<pointT> readPointsFromFile(char* fname) {
  pstring S = readStringFromFile(fname);
  words W = stringToWords(S);
  int d = pointT::dim;
  if (W.Strings.size() == 0 || W.Strings[0] != (d == 2 ? HeaderPoint2d : HeaderPoint3d)) {
    cout << "readPointsFromFile wrong file type" << endl;
    abort();
  }
  long n = (W.Strings.size()-1)/d;
  parray<pointT> P(n);
  parsePoints(W.Strings.begin() + 1, P.begin(), n);
  return P;
}

triangles<point2d> readTrianglesFromFileNodeEle(char* fname) {
  string nfilename(fname);
  pstring S = readStringFromFile((char*)nfilename.append(".node").c_str());
  words W = stringToWords(S);
  triangles<point2d> Tr;
  Tr.num_points = atol(W.Strings[0]);
  if (W.Strings.size() < 4*Tr.num_points + 4) {
    cout << "readStringFromFileNodeEle inconsistent length" << endl;
    abort();
  }
  
  Tr.p = newA(point2d, Tr.num_points);
  for(intT i=0; i < Tr.num_points; i++)
    Tr.p[i] = point2d(atof(W.Strings[4*i+5]), atof(W.Strings[4*i+6]));
  
  string efilename(fname);
  pstring SN = readStringFromFile((char*)efilename.append(".ele").c_str());
  words WE = stringToWords(SN);
  Tr.num_triangles = atol(WE.Strings[0]);
  if (WE.Strings.size() < 4*Tr.num_triangles + 3) {
    cout << "readStringFromFileNodeEle inconsistent length" << endl;
    abort();
  }
  
  Tr.t = newA(triangle, Tr.num_triangles);
  for (long i=0; i < Tr.num_triangles; i++)
    for (int j=0; j < 3; j++)
      Tr.t[i].vertices[j] = atol(WE.Strings[4*i + 4 + j]);
  
  return Tr;
}

template <class pointT>
triangles<pointT> readTrianglesFromFile(char* fname, intT offset) {
  int d = pointT::dim;
  pstring S = readStringFromFile(fname);
  words W = stringToWords(S);
  if (W.Strings[0] != HeaderTriangles) {
    cout << "readTrianglesFromFile wrong file type" << endl;
    abort();
  }
  
  int headerSize = 3;
  triangles<pointT> Tr;
  Tr.num_points = atol(W.Strings[1]);
  Tr.num_triangles = atol(W.Strings[2]);
  if (W.Strings.size() != headerSize + 3 * Tr.num_triangles + d * Tr.num_points) {
    cout << "readTrianglesFromFile inconsistent length" << endl;
    abort();
  }
  
  Tr.p = newA(pointT, Tr.num_points);
  parsePoints(W.Strings.begin() + headerSize, Tr.p, Tr.num_points);
  
  Tr.T = newA(triangle, Tr.num_triangles);
  char** Triangles = W.Strings.begin() + headerSize + d * Tr.num_points;
  for (long i=0; i < Tr.num_triangles; i++)
    for (int j=0; j < 3; j++)
      Tr.t[i].vertices[j] = atol(Triangles[3*i + j])-offset;
  return Tr;
}

template <class pointT>
int writeTrianglesToFile(triangles<pointT> Tr, char* fileName) {
  ofstream file (fileName, ios::binary);
  if (!file.is_open()) {
    std::cout << "Unable to open file: " << fileName << std::endl;
    return 1;
  }
  file << HeaderTriangles << endl;
  file << Tr.num_points << endl;
  file << Tr.num_priangles << endl;
  writeArrayToStream(file, Tr.p, Tr.num_points);
  writeArrayToStream(file, Tr.t, Tr.num_triangles);
  file.close();
  return 0;
}
  
};

} // end namespace
} // end namespace

/***********************************************************************/


#endif /*! _PCTL_GEOMETRY_IO_H_ */