#include <vector>
#include <atomic>
#ifdef OPENMP
#include <omp.h>
#endif
namespace lemon{
    template<class GR, class Item, class Value = int>
    class ParallelElevator{
        // parallel elevator
    private:
        struct VertexExtraInfo
        {
            Value new_excess;
            int new_level;
            std::atomic_flag discovered = ATOMIC_FLAG_INIT; // avoid duplicate add
        };

    public:
        typedef typename std::list<Item>::iterator iterator; 
		typedef typename GR::NodeIt NodeIt;
        typedef typename GR::Node Node;
    private:
        typedef typename ItemSetTraits<GR, Item>::
        template Map<int>::Type IntMap;
        typedef typename ItemSetTraits<GR, Item>::
        template Map<bool>::Type BoolMap;
        
        const GR &_graph;
        int _max_level;
        IntMap _level;
        std::unique_ptr<VertexExtraInfo[]> _vertices;        
        std::unique_ptr<std::vector<int>[]> _active_local; // thread local structure
        int _thread_cnt;
        std::vector<int> active_nodes;

    public:
		ParallelElevator(const GR& graph, int max_level, int thread_count = 1)
        : _graph(graph), _max_level(max_level),
          _level(graph), _init_level(0), _thread_cnt(thread_count) {
              _vertices = std::make_unique<VertexExtraInfo[]>(countNodes(graph));
              _active_local = std::make_unique<std::vector<int>[]>(thread_count);
		}

        int get_active_count() {
            return active_nodes.size();
        }

        Node get_node(int i) {
            return _graph.nodeFromId(active_nodes[i]);
        }

		ParallelElevator(const ParallelElevator& ele) {

		}
        void concatenate_active_sets() {
            active_nodes.clear();
            for(int i = 0; i < _thread_cnt; i++) {
                active_nodes.insert(active_nodes.end(), _active_local[i].begin(), _active_local[i].end());
                _active_local[i].clear();
            }
        }
        void activate(Item i, int thread_id = 0) {
            _active_local[thread_id].push_back(_graph.id(i));
        }

        inline void deactivate(Item i) {
        }
        
        inline bool is_discovered(Item i) {
            return _vertices[_graph.id(i)].discovered.test_and_set(std::memory_order_relaxed);
        }
        inline void clear_discover(Item i) {
            _vertices[_graph.id(i)].discovered.clear(std::memory_order_relaxed);
        }
        inline bool active(Item i) const { return false; }
#ifdef OPENMP
        inline void add_new_excess(Item i, Value excess_value, omp_lock_t& lock) {
            omp_set_lock(&lock);
            _vertices[_graph.id(i)].new_excess += excess_value;
            omp_unset_lock(&lock);
        }
#else
        inline void add_new_excess(Item i, Value excess_value) {
            _vertices[_graph.id(i)].new_excess += excess_value;
        }
#endif
        inline void clear_new_excess(Item i) {
            _vertices[_graph.id(i)].new_excess = 0;
        }
        inline Value get_new_excess(Item i) {
            return _vertices[_graph.id(i)].new_excess;
        }
        inline void add_new_level(Item i, int level_value) {
             _vertices[_graph.id(i)].new_level = level_value;
        }
        inline int get_new_level(Item i) {
            return  _vertices[_graph.id(i)].new_level;
        }
        int operator[](Item i) const { return _level[i]; }

        void lift(Item i, int new_level) {
            _level[i] = new_level;
        }

        int maxLevel() const {
            return _max_level;
        }
        
    private:
        
        int _init_level = 0;
		bool is_initialized = false;
    public:
        
        void initStart() {
            for(typename ItemSetTraits<GR, Item>::ItemIt i(_graph);
                i != INVALID; ++i) {
                _level[i] = -1;
            }
        }
        void initAddItem(Item i) {
            _level[i] = _init_level;            
        }
        void initNewLevel() {
            ++_init_level;
        }
        void initFinish() {            
            for (typename ItemSetTraits<GR, Item>::ItemIt i(_graph);
                i != INVALID; ++i) {
                if (_level[i] == -1) {
                    _level[i] = _max_level;
                }
            }
        }
    };
	template <typename GR, typename CAP>
	struct Preflow_ParallelDefaultTraits {
		typedef GR Digraph;
		typedef CAP CapacityMap;
		typedef typename CapacityMap::Value Value;
		typedef typename Digraph::template ArcMap<Value> FlowMap;
		static FlowMap* createFlowMap(const Digraph& digraph) {
			return new FlowMap(digraph);
		}
		typedef ParallelElevator<Digraph, typename Digraph::Node, Value> Elevator;
		static Elevator* createElevator(const Digraph& digraph, int max_level) {
            #if OPENMP
            int thread_cnt = omp_get_max_threads();
            #else
            int thread_cnt = 1;
            #endif            
			return new Elevator(digraph, max_level, thread_cnt);
		}
		typedef lemon::Tolerance<Value> Tolerance;
	};    
}