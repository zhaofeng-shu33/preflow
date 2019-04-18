/**
 * this file constructs a scalable regular graph, used for testing the speed of preflow algorithm
 */
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <iostream>
#include <cassert>

#include <boost/program_options.hpp>
#include <lemon/list_graph.h>
#include <lemon/preflow.h>
#include "mf_base.h"

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
        _source = _graph.addNode();
        std::vector<Node> node_container;
        for (int i = 0; i < _layer_size; i++) {
            Node tmp = _graph.addNode();
            Arc a = _graph.addArc(_source, tmp);
            aM[a] = _layer_num + 1;
            node_container.push_back(tmp);
        }
        for(int j = 0; j < _layer_num - 1; j++) {
            for (int i = 0; i < _layer_size; i++) {
                Node tmp_new = _graph.addNode();
                Node tmp = node_container[i];
                Arc a = _graph.addArc(tmp, tmp_new);
                aM[a] = _layer_num - j;
                node_container[i] = tmp_new;
            }
        }
        _target = _graph.addNode();
        for (int i = 0; i < _layer_size; i++) {            
            Node tmp = node_container[i];
            Arc a = _graph.addArc(tmp, _target);
            aM[a] = 1;
        }
        if (_verbose) {
            std::cout << "G(V=" << countNodes(_graph) << ',' << 
                "E=" << countArcs(_graph) << ')' << std::endl;            
        }
    }
    
    //! run the algorithm with timer support
    void run(){
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point end_time;
        std::chrono::system_clock::duration dtn;
        float time_used;

        start_time = std::chrono::system_clock::now();
        Preflow_Relabel<Digraph, ArcMap> pf_relabel(_graph, aM, _source, _target);
        pf_relabel.run();
        end_time = std::chrono::system_clock::now();
        dtn = end_time - start_time;
        time_used = std::chrono::duration_cast<std::chrono::milliseconds>(dtn).count()/1000.0;
        if (_verbose) {
            std::cout << time_used << std::endl;
        }
        report["relabel"] = time_used;

        start_time = std::chrono::system_clock::now();
        Preflow<Digraph, ArcMap> pf(_graph, aM, _source, _target);
        pf.run();
        end_time = std::chrono::system_clock::now();
        dtn = end_time - start_time;
        time_used = std::chrono::duration_cast<std::chrono::milliseconds>(dtn).count() / 1000.0;
        if (_verbose) {
            std::cout << time_used << std::endl;
        }
        report["highest"] = time_used;

        // check the flowvalue
        int pf_relabel_flow_value = pf_relabel.flowValue();
        assert(pf_relabel_flow_value == _layer_size);
        int pf_flow_value = pf.flowValue();
        assert(pf_flow_value == _layer_size);
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
    Node _source;
    Node _target;
    Report report;
};
}
using namespace lemon;
void check_positive(int a) {
    if (a <= 0) {
        std::cout << "gamma must be positive number";
        exit(0);
    }
}
int main(int argc, const char *argv[]){
    boost::program_options::options_description desc;
    desc.add_options()
        ("help,h", "Show this help screen")
        ("layer_num", boost::program_options::value<int>()->default_value(4)->notifier(check_positive), "the number of layer")
        ("layer_size", boost::program_options::value<int>()->default_value(3)->notifier(check_positive), "the number of nodes per layer");
    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);
    if (vm.count("help")) {
        std::cout << desc << '\n';
        return 0;
    }
    int layer_num = vm["layer_num"].as<int>();
    int layer_size = vm["layer_size"].as<int>();

    ScalableGraph* sg = new ScalableGraph(layer_num, layer_size);
    sg->init();
    sg->run();
    ScalableGraph::Report r = sg->get_report();
    for (ScalableGraph::Report::iterator it = r.begin(); it != r.end(); it++) {
        std::cout << it->first << ':' << it->second << '\n';
    }
    delete sg;
}