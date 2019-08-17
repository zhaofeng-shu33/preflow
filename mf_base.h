#pragma once
#include <list>
#include <lemon/tolerance.h>
#include <lemon/elevator.h>
#ifdef INTERRUPT
#include <boost/thread.hpp>
#endif
namespace lemon{
    template<class GR, class Item>
    class RelabelElevator{
        //relabel to front elevator
        
    public:
        typedef int Value;
        typedef typename std::list<Item>::iterator iterator; 
		typedef typename GR::NodeIt NodeIt;
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
          _level(graph), _active(graph), _init_level(0){}
        
		RelabelElevator(const RelabelElevator& ele): 
			_graph(ele._graph), _max_level(ele._max_level),relabel_list(ele.relabel_list),
			_level(ele._graph), _active(ele._graph){
			for (NodeIt n(_graph); n != INVALID; ++n) {
				_level[n] = ele._level[n];
			}
			// no need to copy the active nodemap
		}

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
        
        int _init_level = 0;
    
    public:
        
        void initStart() {
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
			bool is_local_elevator = true;
            Tolerance _tolerance;
            //! minimum source side cut
            BoolNodeMap _source_side; 
			//! minimum sink side cut
			BoolNodeMap _sink_side;
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
				if(is_local_elevator)
	                delete _elevator;
                delete _excess;
            }
            // push flow from Node u to Node v
            inline void push(const Node& u, const Node& v, const Arc& e) {
#ifdef INTERRUPT
				boost::this_thread::interruption_point();
#endif
                if(!_elevator->active(v) && v != _target){
                    _elevator->activate(v);
                }
                Value rem = (*_capacity)[e] - (*_flow)[e];
                Value excess = (*_excess)[u];
                if(_tolerance.less(rem, excess)){ // rem + epsilon < excess
					// saturating push
					(*_excess)[u] -= rem;
					(*_excess)[v] += rem;
					_flow->set(e, (*_capacity)[e]);
                }
                else {
					// non-saturating push
					(*_excess)[u] = 0;
					(*_excess)[v] += excess;
					_flow->set(e, (*_flow)[e] + excess);
                }
            }
            // push flow back from Node u to Node v
            inline void push_back(const Node& u, const Node& v, const Arc& e) {
#ifdef INTERRUPT
				boost::this_thread::interruption_point();
#endif
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
                while(_tolerance.positive((*_excess)[n])){
                    int new_level = 2 * _elevator->maxLevel();
                    for(OutArcIt e(_graph, n); e != INVALID; ++e){
                        Node v = _graph.target(e);
                        if (_tolerance.positive((*_capacity)[e] - (*_flow)[e])){
                            if((*_elevator)[n] == (*_elevator)[v] + 1){
                                push(n, v, e);
                            }
							if ((*_excess)[n] == 0)
								break;
                            if(new_level > (*_elevator)[v]){
                                new_level = (*_elevator)[v];
                            }
                        }
                    }
                    if ((*_excess)[n] == 0)
                        break;
                    for(InArcIt e(_graph, n); e != INVALID; ++e) {
                        Node v = _graph.source(e);

                        if (_tolerance.positive((*_flow)[e])){
                            if((*_elevator)[n] == (*_elevator)[v] + 1) {
                                push_back(n, v, e); // push back the flow
                            }
							if ((*_excess)[n] == 0)
								break;
                            if (new_level > (*_elevator)[v]) {
                                new_level = (*_elevator)[v];
                            }
                        }
                    }
                    if ((*_excess)[n] == 0)
                        break;
					if (new_level + 1 < 2 * _elevator->maxLevel())
						relabel(n, new_level);
					else{
						// lift to maximal, no other node can push (or pushback to Node n)
						_elevator->lift(n, 2 * _elevator->maxLevel() - 1);
						break;
					}
                }
				(*_excess)[n] = 0;
                _elevator->deactivate(n);
            }
            
        public:     
            Preflow_Relabel(const Digraph& digraph, const CapacityMap& capacity, 
                            Node source, Node target)
                : _graph(digraph), _capacity(&capacity),
                  _node_num(0), _source(source), _target(target),
                  _flow(NULL), _elevator(NULL), _excess(NULL),
                  _tolerance(), _source_side(digraph), _sink_side(digraph){}
            
            ~Preflow_Relabel(){
                destroyStructures();
            }
			const FlowMap& flowMap() const {
				return *_flow;
			}
			Elevator* elevator() {
				if (is_local_elevator)
					return new Elevator(*_elevator);
				return _elevator;
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
			bool init(const FlowMap& flowMap) {
				createStructures();

				for (ArcIt e(_graph); e != INVALID; ++e) {
					_flow->set(e, flowMap[e]);
				}

				for (NodeIt n(_graph); n != INVALID; ++n) {
					Value excess = 0;
					for (InArcIt e(_graph, n); e != INVALID; ++e) {
						excess += (*_flow)[e];
					}
					for (OutArcIt e(_graph, n); e != INVALID; ++e) {
						excess -= (*_flow)[e];
					}
					if (_tolerance.negative(excess) && n != _source) return false;
					(*_excess)[n] = excess;
				}

				typename Digraph::template NodeMap<bool> reached(_graph, false);
				Elevator* _level = _elevator;
				_level->initStart();
				_level->initAddItem(_target);

				std::vector<Node> queue;
				reached[_source] = true;

				queue.push_back(_target);
				reached[_target] = true;
				while (!queue.empty()) {
					_level->initNewLevel();
					std::vector<Node> nqueue;
					for (int i = 0; i < int(queue.size()); ++i) {
						Node n = queue[i];
						for (InArcIt e(_graph, n); e != INVALID; ++e) {
							Node u = _graph.source(e);
							if (!reached[u] &&
								_tolerance.positive((*_capacity)[e] - (*_flow)[e])) {
								reached[u] = true;
								_level->initAddItem(u);
								nqueue.push_back(u);
							}
						}
						for (OutArcIt e(_graph, n); e != INVALID; ++e) {
							Node v = _graph.target(e);
							if (!reached[v] && _tolerance.positive((*_flow)[e])) {
								reached[v] = true;
								_level->initAddItem(v);
								nqueue.push_back(v);
							}
						}
					}
					queue.swap(nqueue);
				}
				_level->initFinish();

				for (OutArcIt e(_graph, _source); e != INVALID; ++e) {
					Value rem = (*_capacity)[e] - (*_flow)[e];
					if (_tolerance.positive(rem)) {
						Node u = _graph.target(e);
						if ((*_level)[u] == _level->maxLevel()) continue;
						_flow->set(e, (*_capacity)[e]);
						(*_excess)[u] += rem;
					}
				}
				for (InArcIt e(_graph, _source); e != INVALID; ++e) {
					Value rem = (*_flow)[e];
					if (_tolerance.positive(rem)) {
						Node v = _graph.source(e);
						if ((*_level)[v] == _level->maxLevel()) continue;
						_flow->set(e, 0);
						(*_excess)[v] += rem;
					}
				}
				for (NodeIt n(_graph); n != INVALID; ++n)
					if (n != _source && n != _target && _tolerance.positive((*_excess)[n]))
						_level->activate(n);

				return true;
			}
			bool init(const FlowMap& flowMap, Elevator* ele) {
				is_local_elevator = false;
				_elevator = ele; // elevator is not initialized
				createStructures();

				for (ArcIt e(_graph); e != INVALID; ++e) {
					_flow->set(e, flowMap[e]);
				}

				for (NodeIt n(_graph); n != INVALID; ++n) {
					Value excess = 0;
					for (InArcIt e(_graph, n); e != INVALID; ++e) {
						excess += (*_flow)[e];
					}
					for (OutArcIt e(_graph, n); e != INVALID; ++e) {
						excess -= (*_flow)[e];
					}
					if (_tolerance.negative(excess) && n != _source) 
						return false;
					(*_excess)[n] = excess;
				}

				for (OutArcIt e(_graph, _source); e != INVALID; ++e) {
					Value rem = (*_capacity)[e] - (*_flow)[e];
					if (_tolerance.positive(rem)) {
						Node u = _graph.target(e);
						if ((*_elevator)[u] == _elevator->maxLevel()) continue;
						_flow->set(e, (*_capacity)[e]);
						(*_excess)[u] += rem;
					}
				}
				for (InArcIt e(_graph, _source); e != INVALID; ++e) {
					Value rem = (*_flow)[e];
					if (_tolerance.positive(rem)) {
						Node v = _graph.source(e);
						if ((*_elevator)[v] == _elevator->maxLevel()) continue;
						_flow->set(e, 0);
						(*_excess)[v] += rem;
					}
				}
				for (NodeIt n(_graph); n != INVALID; ++n)
					if (n != _source && n != _target && _tolerance.positive((*_excess)[n]))
						_elevator->activate(n);

				return true;
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
            void startSecondPhase(bool getSourceSide = false) {
                pushRelabel(false);
				if (getSourceSide)
					get_min_source_side();
				else
					get_min_sink_side();
            }
			void get_min_source_side() {
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
							if (!_source_side[u] && _tolerance.positive((*_capacity)[e] - (*_flow)[e])) {
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
			void get_min_sink_side() {
				for (NodeIt n(_graph); n != INVALID; ++n) {
					_sink_side[n] = false;
				}
				std::vector<Node> queue;
				queue.push_back(_target);
				_sink_side[_target] = true;
				// breadth-first search
				while (!queue.empty()) {
					std::vector<Node> nqueue;
					for (int i = 0; i < int(queue.size()); i++) {
						Node n = queue[i];
						for (OutArcIt e(_graph, n); e != INVALID; ++e) {
							Node u = _graph.target(e);
							if (!_sink_side[u] && _tolerance.positive((*_flow)[e])) {
								_sink_side[u] = true;
								nqueue.push_back(u);
							}
						}
						for (InArcIt e(_graph, n); e != INVALID; ++e) {
							Node u = _graph.source(e);
							if (!_sink_side[u] && _tolerance.positive((*_capacity)[e] - (*_flow)[e])) {
								_sink_side[u] = true;
								nqueue.push_back(u);
							}
						}
					}
					queue.swap(nqueue);
				}
			}
			// source side minCut
			bool minCutSource(const Node& node) const {
				return _source_side[node];
			}
			// returns true if node is source side cut of min sink side set
            bool minCut(const Node& node) const {
                return !_sink_side[node];
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
