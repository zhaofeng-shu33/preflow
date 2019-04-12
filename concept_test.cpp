// test graph concept
#include <lemon/concepts/digraph.h>
#include <lemon/preflow.h>
#include <lemon/elevator.h>
using namespace lemon;
int main(){
   typedef concepts::Digraph Digraph;
   typedef double T;
   typedef Digraph::ArcMap<T> ArcMap;
   Digraph g;
   ArcMap aM(g);
   Preflow<Digraph, ArcMap> pf(g, aM, INVALID, INVALID);
   // the default traint for Preflow is Elevator,
   // suppose we would like to use LinkedElevator, we can do it by
   typedef LinkedElevator<Digraph, Digraph::Node> LinkedElevator;
   Preflow<Digraph, ArcMap>::SetElevator<LinkedElevator>::Create pf2(g, aM, INVALID, INVALID);
}
