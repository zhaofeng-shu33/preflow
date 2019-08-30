#pragma once
#include <map>
#include <lemon/core.h>
#include <lemon/tolerance.h>
#ifdef INTERRUPT
#include "InterruptibleThread/InterruptibleThread.h"
#endif
namespace lemon {
    template <typename GR, typename CAP>
    struct Preflow_AcyclicDefaultTraits {
        typedef GR Digraph;
        typedef CAP CapacityMap;
        typedef typename CapacityMap::Value Value;
        typedef typename Digraph::template ArcMap<Value> FlowMap;
        static FlowMap* createFlowMap(const Digraph& digraph) {
            return new FlowMap(digraph);
        }
        typedef lemon::Tolerance<Value> Tolerance;
    };

    template <typename GR,
        typename CAP = typename GR::template ArcMap<int>,
        typename TR = Preflow_AcyclicDefaultTraits<GR, CAP> >
        class Preflow_Acyclic {
        public:
            typedef TR Traits;
            typedef typename Traits::Digraph Digraph;
            typedef typename Traits::CapacityMap CapacityMap;
            typedef typename Traits::Value Value;
            typedef typename Traits::FlowMap FlowMap;
            typedef typename Traits::Tolerance Tolerance;
        private:
            TEMPLATE_DIGRAPH_TYPEDEFS(Digraph);
            const Digraph& _graph;
            const CapacityMap* _capacity;

            int _node_num;

            FlowMap* _flow;

            typedef typename Digraph::template NodeMap<Value> ExcessMap;
            ExcessMap* _excess;
            Tolerance _tolerance;
            //! minimum sink side cut
            BoolNodeMap _sink_side;

        protected:
            Node _source, _target;
        public:
            void pushRelabel(bool limit_max_level) {
            }
            void createStructures() {
                _node_num = countNodes(_graph);
                if (!_flow) {
                    _flow = Traits::createFlowMap(_graph);
                }
                if (!_excess) {
                    _excess = new ExcessMap(_graph);
                }
            }

            void destroyStructures() {
                delete _flow;
                delete _excess;
            }
            // push flow from Node u to Node v
            inline void push(const Node& u, const Node& v, const Arc& e) {
#ifdef INTERRUPT
                InterruptibleThread::interruption_point();
#endif
                Value rem = (*_capacity)[e] - (*_flow)[e];
                Value excess = (*_excess)[u];
                if (_tolerance.less(rem, excess)) { // rem + epsilon < excess
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
            inline void push_back(const Node & u, const Node & v, const Arc & e) {
#ifdef INTERRUPT
                InterruptibleThread::interruption_point();
#endif
                Value rem = (*_flow)[e];
                Value excess = (*_excess)[u];
                if (_tolerance.less(rem, excess)) {
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
        protected:
            void discharge(const Node & n) {
            }

        public:
            //! make sure digraph is topologically sorted by node id
            Preflow_Acyclic(const Digraph & digraph, const CapacityMap & capacity,
                Node source, Node target)
                : _graph(digraph), _capacity(&capacity),
                _node_num(countNodes(digraph)), _source(source), _target(target),
                _flow(NULL), _excess(NULL),
                _tolerance(), _sink_side(digraph) {}

            ~Preflow_Acyclic() {
                destroyStructures();
            }
            const FlowMap& flowMap() const {
                return *_flow;
            }
            // after capacity change, reinit the class, used by parametric maximal flow
            void reinit() {
                // update _flow, _excess connected with sink_node
                for (InArcIt e(_graph, _target); e != INVALID; ++e) {
                    if ((*_flow)[e] > (*_capacity)[e]) {
                        Node v = _graph.source(e);
                        (*_excess)[v] += ((*_flow)[e] - (*_capacity)[e]);
                        _flow->set(e, (*_capacity)[e]);
                    }
                }
                // update _flow, _excess connected with source_node
                for (OutArcIt e(_graph, _source); e != INVALID; ++e) {
                    if ((*_capacity)[e] > (*_flow)[e]) {
                        Node u = _graph.target(e);
                        // can we do not distinguish _level->maxLevel() ?
                        if (u != _target) {
                            (*_excess)[u] += ((*_capacity)[e] - (*_flow)[e]);
                            _flow->set(e, (*_capacity)[e]);
                        }
                    }
                }
            }
            bool init(const FlowMap & flowMap) {
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

                std::vector<Node> queue;
                reached[_source] = true;

                queue.push_back(_target);
                reached[_target] = true;
                while (!queue.empty()) {
                    std::vector<Node> nqueue;
                    for (int i = 0; i < int(queue.size()); ++i) {
                        Node n = queue[i];
                        for (InArcIt e(_graph, n); e != INVALID; ++e) {
                            Node u = _graph.source(e);
                            if (!reached[u] &&
                                _tolerance.positive((*_capacity)[e] - (*_flow)[e])) {
                                reached[u] = true;
                                nqueue.push_back(u);
                            }
                        }
                        for (OutArcIt e(_graph, n); e != INVALID; ++e) {
                            Node v = _graph.target(e);
                            if (!reached[v] && _tolerance.positive((*_flow)[e])) {
                                reached[v] = true;
                                nqueue.push_back(v);
                            }
                        }
                    }
                    queue.swap(nqueue);
                }

                for (OutArcIt e(_graph, _source); e != INVALID; ++e) {
                    Value rem = (*_capacity)[e] - (*_flow)[e];
                    if (_tolerance.positive(rem)) {
                        Node u = _graph.target(e);
                        _flow->set(e, (*_capacity)[e]);
                        (*_excess)[u] += rem;
                    }
                }
                for (InArcIt e(_graph, _source); e != INVALID; ++e) {
                    Value rem = (*_flow)[e];
                    if (_tolerance.positive(rem)) {
                        Node v = _graph.source(e);
                        _flow->set(e, 0);
                        (*_excess)[v] += rem;
                    }
                }
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
                for (OutArcIt e(_graph, _source); e != INVALID; ++e) {
                    if (_tolerance.positive((*_capacity)[e])) {
                        Node u = _graph.target(e);
                        _flow->set(e, (*_capacity)[e]);
                        (*_excess)[u] += (*_capacity)[e];
                    }
                }
            }


            Value flowValue() const {
                return (*_excess)[_target];
            }

            inline void startFirstPhase() {
                std::map<int, int> node_id_to_sink_arc_id;
                for (InArcIt e(_graph, _target); e != INVALID; ++e) {
                    int n_id = _graph.id(_graph.source(e));
                    node_id_to_sink_arc_id[n_id] = _graph.id(e);
                }
                for (int i = 1; i < _node_num - 1; i++) {
                    Node n = _graph.nodeFromId(i);
                    std::map<int, int>::iterator mit = node_id_to_sink_arc_id.find(i);
                    if (mit != node_id_to_sink_arc_id.end()) {
                        int arc_id = mit->second;
                        Arc a = _graph.arcFromId(arc_id);
                        push(n, _target, a);
                    }
                    if (!_tolerance.positive((*_excess)[n]))
                        continue;
                    for (OutArcIt e(_graph, n); e != INVALID; ++e) {
                        Node t = _graph.target(e);
                        push(n, t, e);
                        if (!_tolerance.positive((*_excess)[n]))
                            break;
                    }
                }
            }

            // the second phase calculate the minimal cut set
            void startSecondPhase() {
                std::map<int, int> node_id_to_source_arc_id;
                for (OutArcIt e(_graph, _source); e != INVALID; ++e) {
                    int n_id = _graph.id(_graph.target(e));
                    node_id_to_source_arc_id[n_id] = _graph.id(e);
                }
                for (int i = _node_num - 2; i > 0; i--) {
                    Node n = _graph.nodeFromId(i);
                    std::map<int, int>::iterator mit = node_id_to_source_arc_id.find(i);
                    if (mit != node_id_to_source_arc_id.end()) {
                        int arc_id = mit->second;
                        Arc a = _graph.arcFromId(arc_id);
                        push_back(n, _source, a);
                    }
                    if (!_tolerance.positive((*_excess)[n]))
                        continue;
                    for (InArcIt e(_graph, n); e != INVALID; ++e) {
                        Node s = _graph.source(e);
                        push_back(n, s, e);
                        if (!_tolerance.positive((*_excess)[n]))
                            break;
                    }
                }
                get_min_sink_side();
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

            // returns true if node is source side cut of min sink side set
            bool minCut(const Node & node) const {
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