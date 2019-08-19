#pragma once
#include <lemon/core.h>
#include <lemon/tolerance.h>
#ifdef INTERRUPT
#include <boost/thread.hpp>
#endif
#include "relabel_to_front_elevator.h"
#include "fifo_elevator.h"
namespace lemon{

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
	template <typename GR, typename CAP>
	struct Preflow_FIFODefaultTraits {
		typedef GR Digraph;
		typedef CAP CapacityMap;
		typedef typename CapacityMap::Value Value;
		typedef typename Digraph::template ArcMap<Value> FlowMap;
		static FlowMap* createFlowMap(const Digraph& digraph) {
			return new FlowMap(digraph);
		}
		typedef FIFOElevator<Digraph, typename Digraph::Node> Elevator;
		static Elevator* createElevator(const Digraph& digraph, int max_level) {
			return new Elevator(digraph, max_level);
		}
		typedef lemon::Tolerance<Value> Tolerance;
	};

    template <typename GR,
              typename CAP,
              typename TR>
    class Preflow_Base{
        
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
            
            FlowMap* _flow;                        
            
            typedef typename Digraph::template NodeMap<Value> ExcessMap;
            ExcessMap* _excess;
			bool is_local_elevator = true;
            Tolerance _tolerance;
            //! minimum source side cut
            BoolNodeMap _source_side; 
			//! minimum sink side cut
			BoolNodeMap _sink_side;

		protected:
			Elevator* _elevator;
			Node _source, _target;

		private:
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
		protected:
			void discharge(const Node& n) {
                while(_tolerance.positive((*_excess)[n])){
                    int new_level = 2 * _elevator->maxLevel();
                    for(OutArcIt e(_graph, n); e != INVALID; ++e){
                        Node v = _graph.target(e);
                        if (_tolerance.positive((*_capacity)[e] - (*_flow)[e])){
                            if((*_elevator)[n] == (*_elevator)[v] + 1){
                                push(n, v, e);
                            }
							else if (new_level > (*_elevator)[v]) {
								new_level = (*_elevator)[v];
							}
							if ((*_excess)[n] == 0)
								break;
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
							else if (new_level > (*_elevator)[v]) {
								new_level = (*_elevator)[v];
							}
							if ((*_excess)[n] == 0)
								break;                            
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
            Preflow_Base(const Digraph& digraph, const CapacityMap& capacity, 
                            Node source, Node target)
                : _graph(digraph), _capacity(&capacity),
                  _node_num(0), _source(source), _target(target),
                  _flow(NULL), _elevator(NULL), _excess(NULL),
                  _tolerance(), _source_side(digraph), _sink_side(digraph){}
            
            ~Preflow_Base(){
                destroyStructures();
            }
			virtual void pushRelabel(bool limit_max_level) = 0;
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
	template <typename GR,
		typename CAP = typename GR::template ArcMap<int>,
		typename TR = Preflow_RelabelDefaultTraits<GR, CAP> >
		class Preflow_Relabel : public Preflow_Base<GR, CAP, TR> {
		public:
			typedef TR Traits;
			typedef typename Traits::Digraph Digraph;
			typedef typename Traits::CapacityMap CapacityMap;
			typedef typename Traits::Value Value;
			typedef typename Traits::FlowMap FlowMap;
			typedef typename Traits::Tolerance Tolerance;
			typedef typename Traits::Elevator Elevator;
		private:
			TEMPLATE_DIGRAPH_TYPEDEFS(Digraph);
		public:
			Preflow_Relabel(const Digraph& digraph, const CapacityMap& capacity,
				Node source, Node target) : Preflow_Base<GR, CAP,TR>(digraph, capacity, source, target) {}
			void pushRelabel(bool limit_max_level) {
				typename Elevator::iterator ele_it = this->_elevator->begin();
				while (ele_it != this->_elevator->end()) {
					if (limit_max_level && (*(this->_elevator))[*ele_it] >= this->_elevator->maxLevel()) {
						ele_it++;
						continue;
					}
					if (*ele_it == this->_source || *ele_it == this->_target || !this->_elevator->active(*ele_it)) {
						ele_it++;
						continue;
					}
					Value old_label = (*(this->_elevator))[*ele_it];
					this->discharge(*ele_it);
					if ((*(this->_elevator))[*ele_it] > old_label) {
						this->_elevator->moveToFront(ele_it);
						ele_it = this->_elevator->begin();
					}
					else {
						ele_it++;
					}
				}
			}

	};
	template <typename GR,
		typename CAP = typename GR::template ArcMap<int>,
		typename TR = Preflow_FIFODefaultTraits<GR, CAP> >
		class Preflow_FIFO : public Preflow_Base<GR, CAP, TR> {
		public:
			typedef TR Traits;
			typedef typename Traits::Digraph Digraph;
			typedef typename Traits::CapacityMap CapacityMap;
			typedef typename Traits::Value Value;
			typedef typename Traits::FlowMap FlowMap;
			typedef typename Traits::Tolerance Tolerance;
			typedef typename Traits::Elevator Elevator;
		private:
			TEMPLATE_DIGRAPH_TYPEDEFS(Digraph);
		public:
			Preflow_FIFO(const Digraph& digraph, const CapacityMap& capacity,
				Node source, Node target) : Preflow_Base<GR, CAP,TR>(digraph, capacity, source, target) {}
			void pushRelabel(bool limit_max_level) {
				Node current_discharge_node;
				while (this->_elevator->getFront(current_discharge_node, limit_max_level)) {
					this->discharge(current_discharge_node);
				}				
			}

	};
}
