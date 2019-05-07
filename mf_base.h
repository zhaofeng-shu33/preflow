#pragma once
#include <list>
#include <lemon/tolerance.h>
#include <lemon/elevator.h>
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

        void lift(Item i, int new_level) {
            _level[i] = new_level;
        }

        int maxLevel() const {
            return _max_level;
        }

        // move the Item to the front of relabel_list
        void moveToFront(iterator item_it) {
            Item item = *item_it;
            relabel_list.erase(item_it);
            relabel_list.push_front(item);
        }
        iterator begin() {
            return relabel_list.begin();
        }
        iterator end() {
            return relabel_list.end();
        }    
    private:
        
        int _init_level;
    
    public:
        
        void initStart() {
            _init_level = 0;
            for(typename ItemSetTraits<GR, Item>::ItemIt i(_graph);
                i != INVALID; ++i) {
                _level[i] = -1;
                _active[i] = false;
            }
        }
        void initAddItem(Item i) {
            _level[i] = _init_level;
            relabel_list.push_front(i);
        }
        void initNewLevel() {
            ++_init_level;
        }
        void initFinish() {            
            for (typename ItemSetTraits<GR, Item>::ItemIt i(_graph);
                i != INVALID; ++i) {
                if (_level[i] == -1) {
                    relabel_list.push_back(i);
                    _level[i] = _max_level;
                }
            }

        }
    };

    template <typename GR, typename CAP>
    struct Preflow_RelabelDefaultTraits {
        typedef GR Digraph;
        typedef CAP CapacityMap;
        typedef typename CapacityMap::Value Value;
        typedef typename Digraph::template ArcMap<Value> FlowMap;
        static FlowMap* createFlowMap(const Digraph& digraph) {
            return new FlowMap(digraph);
        }
        typedef RelabelElevator<Digraph, typename Digraph::Node> Elevator;
        static Elevator* createElevator(const Digraph& digraph, int max_level) {
            return new Elevator(digraph, max_level);
        }
        typedef lemon::Tolerance<Value> Tolerance;
    };

    template <typename GR,
              typename CAP = typename GR::template ArcMap<int>,
              typename TR = Preflow_RelabelDefaultTraits<GR, CAP> >
    class Preflow_Relabel{
        
        public:
            // for linked list we use std::list<Node>
            typedef TR Traits;
            typedef typename Traits::Digraph Digraph;
            typedef typename Traits::CapacityMap CapacityMap;
            typedef typename Traits::Value Value;
            typedef typename Traits::FlowMap FlowMap;
            typedef typename Traits::Tolerance Tolerance; 
            typedef typename Traits::Elevator Elevator;
        private:  
            TEMPLATE_DIGRAPH_TYPEDEFS(Digraph);
            
            const Digraph& _graph;
            const CapacityMap* _capacity;
            
            int _node_num;
            
            Node _source, _target;
            
            FlowMap* _flow;            
            Elevator* _elevator;
            
            typedef typename Digraph::template NodeMap<Value> ExcessMap;
            ExcessMap* _excess;
            
            Tolerance _tolerance;
            
            BoolNodeMap _source_side;

            void createStructures() {
                _node_num = countNodes(_graph);
                if(!_flow){
                    _flow = Traits::createFlowMap(_graph);                   
                }
                if(!_elevator){
                    _elevator = Traits::createElevator(_graph, _node_num);
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
            // push flow from Node u to Node v
            inline void push(const Node& u, const Node& v, const Arc& e) {
                if(!_elevator->active(v) && v != _target){
                    _elevator->activate(v);
                }
                Value rem = (*_capacity)[e] - (*_flow)[e];
                Value excess = (*_excess)[u];
                if(_tolerance.less(rem, excess)){
                    // saturating push
                    (*_excess)[u] -= rem;
                    (*_excess)[v] += rem;
                    _flow->set(e, (*_capacity)[e]);
                }
                else {
                    // no saturating push
                    (*_excess)[u] = 0;
                    (*_excess)[v] += excess;
                    _flow->set(e, (*_flow)[e] + excess);
                }
            }
            // push flow back from Node u to Node v
            inline void push_back(const Node& u, const Node& v, const Arc& e) {
                if(!_elevator->active(v) && v != _source){
                    _elevator->activate(v);
                }
                Value rem = (*_flow)[e];
                Value excess = (*_excess)[u];
                if(_tolerance.less(rem, excess)){
                    // saturating push
                    (*_excess)[u] -= rem;
                    (*_excess)[v] += rem;
                    _flow->set(e, 0);
                }
                else {
                    // no saturating push
                    (*_excess)[u] = 0;
                    (*_excess)[v] += excess;
                    _flow->set(e, (*_flow)[e] - excess);
                }                
            }
            inline void relabel(const Node& n, int new_level) {
                _elevator->lift(n, new_level + 1);
            }
            void discharge(const Node& n) {
                while((*_excess)[n] > 0){
                    int new_level = 2 * _elevator->maxLevel();
                    for(OutArcIt e(_graph, n); e != INVALID; ++e){
                        Node v = _graph.target(e);
                        if (_tolerance.less((*_flow)[e], (*_capacity)[e])){
                            if((*_elevator)[n] == (*_elevator)[v] + 1){
                                push(n, v, e);
                            }
                            else if(new_level > (*_elevator)[v]){
                                new_level = (*_elevator)[v];
                            }
                        }
                        if ((*_excess)[n] == 0)
                            break;
                    }
                    if ((*_excess)[n] == 0)
                        break;
                    for(InArcIt e(_graph, n); e != INVALID; ++e) {
                        Node v = _graph.source(e);

                        if (_tolerance.positive((*_flow)[e])){
                            if((*_elevator)[n] == (*_elevator)[v] + 1) {
                                push_back(n, v, e); // push back the flow
                            }
                            else if (new_level > (*_elevator)[v]) {
                                new_level = (*_elevator)[v];
                            }
                        }
                        if ((*_excess)[n] == 0)
                            break;
                    }
                    if ((*_excess)[n] == 0)
                        break;
                    relabel(n, new_level);
                }
                _elevator->deactivate(n);
            }
            
        public:     
            Preflow_Relabel(const Digraph& digraph, const CapacityMap& capacity, 
                            Node source, Node target)
                : _graph(digraph), _capacity(&capacity),
                  _node_num(0), _source(source), _target(target),
                  _flow(NULL), _elevator(NULL), _excess(NULL),
                  _tolerance(), _source_side(digraph){}
            
            ~Preflow_Relabel(){
                destroyStructures();
            }
            // after capacity change, reinit the class, used by parametric maximal flow
            // only second phase is needed to run
            void reinit() {
                // update _flow, _excess connected with sink_node
                for (InArcIt e(_graph, _target); e != INVALID; ++e) {
                    if ((*_flow)[e] > (*_capacity)[e]) {
                        Node v = _graph.source(e);
                        (*_excess)[v] += ((*_flow)[e] - (*_capacity)[e]);
                        _flow->set(e, (*_capacity)[e]);
                        _elevator->activate(v);
                    }
                }
                // update _flow, _excess connected with source_node
                for (OutArcIt e(_graph, _source); e != INVALID; ++e) {
                    if ((*_capacity)[e] > (*_flow)[e]) {
                        Node u = _graph.target(e);
                        // can we do not distinguish _level->maxLevel() ?
                        if (u != _target && (*_elevator)[u] <= 1 + _elevator->maxLevel()) {
                            (*_excess)[u] += ((*_capacity)[e] - (*_flow)[e]);
                            _flow->set(e, (*_capacity)[e]);
                            _elevator->activate(u);
                        }
                    }
                }
            }
            void init() {
                createStructures();

                for (NodeIt n(_graph); n != INVALID; ++n) {
                    (*_excess)[n] = 0;
                }

                for (ArcIt e(_graph); e != INVALID; ++e) {
                    _flow->set(e, 0);
                }                
                // use breadth-first search to add item
                typename Digraph::template NodeMap<bool> reached(_graph, false);
                reached[_target] = true;
				reached[_source] = true;
                _elevator->initStart();
                _elevator->initAddItem(_target);
                std::vector<Node> queue;
                queue.push_back(_target);
                while (!queue.empty()) {
                    _elevator->initNewLevel();
                    std::vector<Node> nqueue;
                    for (int i = 0; i < queue.size(); i++) {
                        Node n = queue[i];
                        for (InArcIt e(_graph, n); e != INVALID; ++e) {
                            Node u = _graph.source(e);
                            if (!reached[u] && _tolerance.positive((*_capacity)[e])) {
                                reached[u] = true;
                                _elevator->initAddItem(u);
                                nqueue.push_back(u);
                            }
                        }
                    }
                    queue.swap(nqueue);
                }
                _elevator->initFinish();
                
                for(OutArcIt e(_graph, _source); e != INVALID; ++e){
                    if(_tolerance.positive((*_capacity)[e])){
                        Node u = _graph.target(e);
                        _flow->set(e, (*_capacity)[e]);
                        (*_excess)[u] += (*_capacity)[e];
                        if(u != _target && !_elevator->active(u)){
                            _elevator->activate(u);
                        }
                    }
                }
            }
            
            void pushRelabel(bool limit_max_level) {
                typename Elevator::iterator ele_it = _elevator->begin();
                while(ele_it != _elevator->end()){
                    if (limit_max_level && (*_elevator)[*ele_it] >= _elevator->maxLevel()) {
                        ele_it++;
                        continue;
                    }
                    if (*ele_it == _source || *ele_it == _target || !_elevator->active(*ele_it)) {
                        ele_it++;
                        continue;
                    }
                    Value old_label = (*_elevator)[*ele_it];
                    discharge(*ele_it);
                    if((*_elevator)[*ele_it] > old_label){
                        _elevator->moveToFront(ele_it);
                        ele_it = _elevator->begin();
                    }
                    else{
                        ele_it++;
                    }
                }
            }
            
            Value flowValue() const {
                return (*_excess)[_target];
            }

            inline void startFirstPhase() {
                pushRelabel(true);
            }

            // the second phase calculate the minimal cut set
            void startSecondPhase() {
                pushRelabel(false);
                for (NodeIt n(_graph); n != INVALID; ++n) {
                    _source_side[n] = false;
                }
                std::vector<Node> queue;
                queue.push_back(_source);
                _source_side[_source] = true;
                // breadth-first search
                while (!queue.empty()) {
                    std::vector<Node> nqueue;
                    for (int i = 0; i < int(queue.size()); i++) {
                        Node n = queue[i];
                        for (OutArcIt e(_graph, n); e != INVALID; ++e) {
                            Node u = _graph.target(e);
                            if(!_source_side[u] && _tolerance.less((*_flow)[e], (*_capacity)[e])){
                                _source_side[u] = true;
                                nqueue.push_back(u);
                            }
                        }
                        for (InArcIt e(_graph, n); e != INVALID; ++e) {
                            Node u = _graph.source(e);
                            if (!_source_side[u] && _tolerance.positive((*_flow)[e])) {
                                _source_side[u] = true;
                                nqueue.push_back(u);
                            }
                        }
                    }
                    queue.swap(nqueue);
                }
            }

            // source side minCut
            bool minCut(const Node& node) const {
                return _source_side[node];
            }

            void runMinCut() {
                init();
                startFirstPhase();
            }

            void run() {
                init();
                startFirstPhase();
                startSecondPhase();
            }
    };

}
