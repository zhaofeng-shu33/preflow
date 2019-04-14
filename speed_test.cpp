/**
 * this file constructs a scalable regular graph, used for testing the speed of preflow algorithm
 */
#include <string>
#include <map>
#include <lemon/list_graph.h>
namespace lemon{
class ScalableGraph{
  public:
    typedef ListDigraph Digraph;
    typedef int T;
    typedef Digraph::ArcMap<T> ArcMap;
    typedef ListDigraph::Node Node;
    typedef ListDigraph::Arc Arc;
    typedef std::map<std::string, float> Report;
    ScalableGraph(int layer_num, int layer_size, bool verbose = false):
      _layer_num(layer_num), _layer_size(layer_size), _verbose(verbose),
      aM(_graph) {}
    void init() {
    
    }
    
    //! run the algorithm with timer support
    void run(){
        
    }
    
    //! get the time used to calculate the minimum cut set
    const Report& get_report(){
        return report;
    }
    
  private:
    int _layer_num;
    int _layer_size;
    bool _verbose;
    Digraph _graph;
    ArcMap aM;
    Report report;
};
}
using namespace lemon;
int main(){
    ScalableGraph sg(4,3);
    sg.init();
    sg.run();
    ScalableGraph::Report r = sg.get_report();
}