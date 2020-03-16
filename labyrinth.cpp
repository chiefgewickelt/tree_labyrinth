#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <cmath>
#include <random>
#include <string>
#include <vector>
#include <utility>

enum class Orientation {TOP,RIGHT};
  
std::pair<int,int> seek_parent(int act, std::vector<int> &v){
    int p = v[act];
    int old = act;
    int depth = 0;
    while ( p != act ){
      ++depth;
      act = p;
      p = v[act];
    }
    v[old] = act;
    return std::make_pair(act,depth);
};

void union_( int a , int b , std::vector<int> &v){
    std::pair<int,int> a_p , b_p;
    a_p = seek_parent(a,v);
    b_p = seek_parent(b,v);

    if(a_p.first != b_p.first){
      if(a_p.second > b_p.second){
       	v[b_p.first] = a_p.first;
      }
      else{
	v[a_p.first] = b_p.first;
      }
    }
};

typedef std::mt19937 RNG;
uint32_t seed_val = std::time(0);//other seed possible...
RNG rng;
void initialize(){
  rng.seed(seed_val);
}
std::normal_distribution<double> normal_dist(3.0, 1.0);
std::uniform_int_distribution<uint32_t> uint_dist(0,10000);

struct Edge {
  int x1{};
  int x2{};
  int y1{};
  int y2{};
  double weight{};
  bool used = 0;
  void set(int x, int y , Orientation orientation, int X, int Y) {
    x1 = x;
    y1 = y;
    if(orientation == Orientation::RIGHT){
      x2 = x+1;
      y2 = y; 
    }
    if(orientation == Orientation::TOP) {
      x2 = x;
      y2 = y+1;
    }
    weightometer(x,y,X,Y);
  }
  void weightometer(int x, int y, int X, int Y) {
    weight = std::exp(-((std::pow(x-X/2.0,2.0)+std::pow(y-Y/2.0,2.0))/100.0)) + normal_dist(rng);
  }
  bool operator<(Edge other) { return weight < other.weight;}
};


std::vector<Edge> make_labyrinth(int X, int Y) {
  int number_of_edges = (X-1)*Y+X*(Y-1);
  std::vector<Edge> edges(number_of_edges);
    { int i = 0;
      for(int x = 0; x < X-1; ++x) {
	for(int y = 0; y < Y-1; ++y) {
	  edges[i].set(x,y, Orientation::TOP, X , Y);
	  ++i;
	  edges[i].set(x,y,Orientation::RIGHT, X, Y);
	  ++i;
	}
	edges[i].set(x,Y-1,Orientation::RIGHT, X, Y);
	++i;
      }
      for(int y = 0; y < Y-1; ++y) {
	edges[i].set(X-1,y,Orientation::TOP, X, Y);
	++i;
      }
    }
    std::sort(edges.begin(),edges.end());
    std::vector<int> union_finder(X*Y);
    std::vector<int> open_sides(X*Y,0);
    std::iota(union_finder.begin(),union_finder.end(),0);
    {
      int number_of_unions = 0;
      int a , b;
      int a_parent, b_parent ,a_depth, b_depth;
      for(auto &e : edges) {
	std::pair<int,int> a_parent_depth;
	std::pair<int,int> b_parent_depth;
	
	a = e.y1*X + e.x1;
	b = e.y2*X + e.x2;
	a_parent_depth = seek_parent(a,union_finder);
	b_parent_depth = seek_parent(b,union_finder);
	
	a_parent = a_parent_depth.first;
	a_depth = a_parent_depth.second;
	b_parent = b_parent_depth.first;
	b_depth = b_parent_depth.second;
	if(open_sides[a]<3 and open_sides[b]<3 and a_parent != b_parent) {
	  e.used = 1;
	  //union_(a,b,union_finder);
	  if(a_depth > b_depth) {union_finder[b_parent] = a_parent; }
	  else {union_finder[a_parent] = b_parent; }
	  ++open_sides[a];
	  ++open_sides[b];
	  ++number_of_unions;
	}
	else {
	  if(number_of_unions >= X*Y-1) {
	    std::cout << "all connected\n";
	    break;
	  }
	}
      }
      
    }
    return edges;
}

template <typename T>
T calculate_svg_coordinates_from( T input_coordinate) {
  return input_coordinate + 5;
}

typedef void (*Transform) (int X, int Y, double *x, double *y);

/* This is the square transformation.
 *
 */
void transform_square(int X, int Y, double *x, double *y) {

}

/* This is the circle transformation.
 *
 */
void transform_circle(int X, int Y, double *x, double *y) {
  if (*x+*x == X && *y+*y == Y) {
    return;
  }
  double middle_x = X / 2.0;
  double middle_y = Y / 2.0;
  double rel_x = *x - middle_x;
  double rel_y = *y - middle_y;
  double radius = std::max(std::abs(rel_x), std::abs(rel_y));
  /* see https://github.com/chiefgewickelt/tree_labyrinth/issues/3
   *  x ,  y  -> points_on_the_circle
   * 0.5, 0.5 -> 4
   * 1.5, 1.5 -> 12
   * 2.5, 2.5 -> 20
   * 
   * 0.0, 0.0 -> 1
   * 1.0, 1.0 -> 8
   * 2.0, 2.0 -> 16
   * 3.0, 3.0 -> 24
   *
   * 0.5, 0.0 -> 2
   * 1.5, 1.0 -> 10
   * 2.5, 2.0 -> 18
   * 3.5, 3.0 -> 26
   *
   */
  double points_on_the_circle = 2 * radius * 4;
  double radius_x = radius;
  double radius_y = radius;
  if (X % 2 != Y % 2) {
    if (std::ceil(radius) - radius == 0.5) {
      // the radius is detemined by the higher value
      points_on_the_circle -= 2;
      if (X % 2 == 1) {
        radius_y -= 0.5;
        //std::cout << "(1.1) "; // debug
      } else {
        // Y % 2 == 1
        radius_x -= 0.5;
        //std::cout << "(1.2) "; // debug
      }    } else {
      // the radius is detemined by the lower value
      points_on_the_circle += 2;
      if (X % 2 == 1) {
        radius_x += 0.5;
        //std::cout << "(2.1) "; // debug
      } else {
        // Y % 2 == 1
        radius_y += 0.5;
        //std::cout << "(2.2) "; // debug
      }
    }
  }
  double index_on_circle = 0;
  /* +---d---+        
   * |       |
   * c       a
   * |       |
   * +---b---+
   */
  if        (rel_x == radius_x) {
    //std::cout << "a "; // debug
    index_on_circle += 0 * (radius_y + radius_x) + rel_y;
  } else if (rel_y == radius_y) {
    //std::cout << "b "; // debug
    index_on_circle += 1 * (radius_y + radius_x) - rel_x;
  } else if (rel_x == -radius_x) {
    //std::cout << "c "; // debug
    index_on_circle += 2 * (radius_y + radius_x) - rel_y;
  } else if (rel_y == -radius_y) {
    //std::cout << "d "; // debug
    index_on_circle += 3 * (radius_y + radius_x) + rel_x;
  } else {
    std::cout << "error ";
  }
  double angle = 2 * M_PI * index_on_circle / points_on_the_circle;
  *x = std::cos(angle) * radius_x + middle_x;
  *y = std::sin(angle) * radius_y + middle_y;
  //std::cout << "points_on_the_circle=" << points_on_the_circle << " radius_x=" << radius_x << " radius_y=" << radius_y << " rel_x=" << rel_x << " rel_y=" << rel_y << " index_on_circle=" << index_on_circle << " angle=" << angle << " \n"; // debug
}

void insert_svg_line(std::fstream& s, int X, int Y, double x1 , double y1, double x2, double y2, Transform transform, std::string indent = "    " ) {
  auto o = [] (auto input_coordinate) {return calculate_svg_coordinates_from(input_coordinate);};
  transform(X, Y, &x1, &y1);
  transform(X, Y, &x2, &y2);
  s << indent << R"(<line x1=")" << o(x1) << R"(" y1=")" << o(y1) << R"(" x2=")" << o(x2) << R"(" y2=")" << o(y2) << R"("/>)" << '\n';
}

void edges_to_svg(std::vector<Edge> & edges, std::fstream & s, int X , int Y, Transform transform) {
  auto o = [] (auto input_coordinate) {return calculate_svg_coordinates_from(input_coordinate);};
  
  s << "<?xml version=\"1.0\" standalone=\"yes\"?>\n";
  s << R"(
<svg width=")" << 10*X << R"(" height=")" << 10*Y << R"(" viewBox="0 0 )" << o(o(X)) << ' ' <<  o(o(Y)) << R"(" version="1.1"
   xmlns="http://www.w3.org/2000/svg">
<desc>labyrinth of size )" << o(X) << R"(x)" << o(Y) << R"(</desc>
  <rect x="0" y="0" width=")" << o(o(X)) << R"(" height=")" << o(o(Y)) << R"("
      fill="white" />
  <g stroke="black"  stroke-width="0.1"> )" << '\n';
  for (int x = 0; x < X; x++) {
    insert_svg_line(s,X,Y,x,Y,x+1,Y, transform);
    insert_svg_line(s,X,Y,x,0,x+1,0, transform);
  }
  for (int y = 0; y < Y; y++) {
    if (y != Y-1) insert_svg_line(s,X,Y,0,y,0,y+1, transform);
    if (y != 0) insert_svg_line(s,X,Y,X,y,X,y+1, transform);
  }
  
  for(auto& e : edges) {
    if(e.used) continue;
    if(e.x1 == e.x2) {//horizontal line
      insert_svg_line(s,X,Y,e.x1,e.y2, e.x1+1,e.y2, transform);
    }
    else {
      insert_svg_line(s,X,Y,e.x2,e.y1,e.x2,e.y2+1, transform);
    }
  }
  s << "</g>" << "\n";
  s << "</svg>\n";
}

void edges_to_gp(std::vector<Edge> & edges, std::fstream & s, int X , int Y) {

  s << "set terminal epslatex size 10,10 standalone\n";
  s << "set output 'last_labyrinth.tex'\n";
  s << "set key off\n";
  s << "unset ytics\n";
  s << "unset xtics\n";
  s << "set yrange [-2:" << Y+1 << "]\n";
  s << "set xrange [-2:" << X+1 << "]\n";
  s << "set arrow from -0.5," << Y-1.5 << " to -0.5," << -0.5 << " nohead\n";
  s << "set arrow from " << X-0.5 << "," << Y-0.5 << " to " << X-0.5 << "," << 1-0.5 << "nohead\n";
  s << "set arrow from " << X-0.5 << "," << Y-0.5 << " to " << 0-0.5 << "," << Y-0.5 << "nohead\n";
  s << "set arrow from -0.5,-0.5 to " << X-0.5 << "," << 0-0.5 << " nohead\n";
  
  for(auto& e : edges) {
    if (e.used) continue;
    if(e.x1 == e.x2){//horizontal line
      s << "set arrow from " << e.x1-0.5 << ',' << e.y1+0.5 << " to " << e.x2+0.5 << ',' << e.y2-0.5 << " nohead\n";
    }
    else {//verical line
      s << "set arrow from " << e.x1+0.5 << ',' << e.y1+0.5 << " to " << e.x2 - 0.5 << ',' << e.y2 - 0.5 << " nohead\n";
    }
  }
  //  s << "set arrow from 1,1 to 5,8 nohead\n";
  s << "plot -2\n";
}

void help(char **argv) {
  std::cout << "please provide all arguments\n";
  std::cout << argv[0] << " <square|circle> <width> <height>\n";
}

int main(int argc, char** argv) {
  if (argc != 4) {
    help(argv);
    return 1;
  }
  std::string choice = argv[1];
  Transform transform;
  if (choice.compare("circle") == 0) {
    transform = transform_circle;
  } else if (choice.compare("square") == 0) {
    transform = transform_square;
  } else {
    help(argv);
    return 1;
  }
  int X = std::stoi(argv[2]);
  int Y = std::stoi(argv[3]);
  initialize();
  std::string filename = std::to_string(X) + "x" + std::to_string(Y) + ".svg";
  std::fstream s(filename,s.out);
  if (!s.is_open()) {
    std::cout << filename << " could not be created\n";
    return -1;
  }
  std::vector<Edge> edges = make_labyrinth(X,Y);
  edges_to_svg(edges,s,X,Y, transform);
  s.close();
}
