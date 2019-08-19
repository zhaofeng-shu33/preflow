#include <list>
namespace lemon{
    template<class GR, class Item>
    class FIFOElevator{
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
		iterator first_phase_item_pointer;
        IntMap _level;
        std::list<Item> fifo_list;

    public:
		FIFOElevator(const GR& graph, int max_level)
        : _graph(graph), _max_level(max_level),
          _level(graph), _init_level(0){
			first_phase_item_pointer = fifo_list.begin();
		}
        
		FIFOElevator(const FIFOElevator& ele):
			_graph(ele._graph), _max_level(ele._max_level),fifo_list(ele.fifo_list),
			_level(ele._graph){
			for (NodeIt n(_graph); n != INVALID; ++n) {
				_level[n] = ele._level[n];
			}			
		}

        void activate(Item i) {
            fifo_list.push_back(i);
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

         bool getFront(Item& item, bool limit_max_level = false) {
			 if (fifo_list.size() == 0)
				 return false;
			if (limit_max_level) {
				if (is_initialized == false) {
					first_phase_item_pointer = fifo_list.begin();
					is_initialized = true;
				}
				while (first_phase_item_pointer != fifo_list.end()) {
					item = *first_phase_item_pointer;
					if (_level[item] < _max_level) {						
						first_phase_item_pointer = fifo_list.erase(first_phase_item_pointer);
						if (first_phase_item_pointer == fifo_list.end())
							is_initialized = false;
						return true;
					}
					first_phase_item_pointer++;					
				}
				return false;
			}
			else {
				item = fifo_list.front();
				fifo_list.pop_front();
				return true;
			}
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
}