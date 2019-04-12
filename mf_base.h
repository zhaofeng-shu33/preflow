#include <lemon/preflow.h>
namespace lemon{
    template <typename GR,
              typename CAP = typename GR::template ArcMap<int>,
              typename TR = PreflowDefaultTraits<GR, CAP> >
    class Preflow_Relabel{
        
        public:
            // for linked list we use std::list<Node>
            typedef TR Traits;
            typedef typename Traits::Digraph Digraph;
            typedef typename Traits::CapacityMap CapacityMap;
            typedef typename Traits::Value Value;
            typedef typename Traits::FlowMap FlowMap;
            typedef typename Traits::Tolerance Tolerance; 

        private:  
            TEMPLATE_DIGRAPH_TYPEDEFS(Digraph);
            
        public:
            typedef std::list<Node> Elevator;
        
        private:
            const Digraph& _graph;
            const CapacityMap* _capacity;
            
            int _node_num;
            
            Node _source, _target;
            
            FlowMap* _flow;            
            Elevator* _elevator;
            
            typedef typename Digraph::template NodeMap<Value> ExcessMap;
            ExcessMap* _excess;
            
            Tolerance _tolerance;
            
            void createStructures() {
                _node_num = countNode(_graph);
                if(!_flow){
                    _flow = Traits::createFlowMap(_graph);                   
                }
                if(!_elevator){
                    _elevator = new std::list<Node>;
                }
                if(!_excess){
                    _excess = new ExcessMap(_graph);
                }
            }
            
            void destroyStructures() {
                delete _flow;
                delete _elevator;
                delete _excess;
            }            
        public:     
            Preflow_Relabel(const Digraph& digraph, const CapacityMap& capacity, 
                            Node source, Node target)
                : _graph(digraph), _capacity(capacity),
                  _node_num(0), _source(source), _target(target)
                  _flow(NULL), _elevator(NULL), _excess(NULL),
                  _tolerance(){}
            
            ~Preflow_Relabel(){
                destroyStructures();
            }
            
            void init() {
                createStructures();
                
            }
    };

}
