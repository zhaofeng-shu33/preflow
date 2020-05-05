#include <vector>
#include <atomic>

namespace lemon{
    struct VertexExtraInfo
    {
        std::atomic<int> new_excess;
        int new_level;
        std::atomic_flag discovered = ATOMIC_FLAG_INIT; // avoid duplicate add
    };
    template<class GR, class Item>
    class ParallelElevator{
        // parallel elevator

    public:
        typedef int Value;
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
        
        inline bool active(Item i) const { return false; }
        
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
		typedef ParallelElevator<Digraph, typename Digraph::Node> Elevator;
		static Elevator* createElevator(const Digraph& digraph, int max_level) {
			return new Elevator(digraph, max_level);
		}
		typedef lemon::Tolerance<Value> Tolerance;
	};    
}