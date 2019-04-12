#include <lemon/list_graph.h>
namespace experiment{
    class Preflow{
        typedef lemon::ListDigraph Digraph;
        typedef lemon::ListDigraph::ArcMap<double> ArcMap;
        typedef ArcMap CapacityMap;
        typedef Digraph::Node Node;
    };

}