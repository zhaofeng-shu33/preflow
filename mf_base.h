#include <list>
#include <lemon/preflow.h>
namespace lemon{
    template<class GR, class Item>
    class RelabelElevator{
        //relabel to front elevator
        
    public:
        typedef int Value;
        typedef typename std::list<Item>::iterator iterator; 
    private:
        typedef typename ItemSetTraits<GR, Item>::
        template Map<int>::Type IntMap;
        typedef typename ItemSetTraits<GR, Item>::
        template Map<bool>::Type BoolMap;
        
        const GR &_graph;
        int _max_level;
        IntMap _level;
        BoolMap _active;
        std::list<Item> relabel_list;

    public:
        RelabelElevator(const GR& graph, int max_level)
        : _graph(graph), _max_level(max_level),
          _level(graph), _active(graph) {}
          
        void activate(Item i) {
            _active[i] = true;
        }
        
        void deactivate(Item i) {
            _active[i] = false;
        }
        
        bool active(Item i) const { return _active[i]; }
        
        int operator[](Item i) const { return _level[i]; }
        
        // move the Item to the front of relabel_list
        void moveToFront(iterator item_it) {
            Item item = *item_it;
            relabel_list.erase(item_it);
            relabel_list.push_front(item);
        }
    
    private:
        
        int _init_level;
    
    public:
        
        void initStart() {
            _init_level = 0;
            for(typename ItemSetTraits<GR, Item>::ItemIt i(_graph);
                i != INVALID; ++i) {
                _level[i] = _max_level;
                _active[i] = false;
            }
        }
        void initAddItem(Item i) {
            _level[i] = _init_level;
            relabel_list.push_back(i);
        }
        void initNewLevel() {
            ++_init_level;
        }
        void initFinish() {            
        }
    };

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
                : _graph(digraph), _capacity(&capacity),
                  _node_num(0), _source(source), _target(target),
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
