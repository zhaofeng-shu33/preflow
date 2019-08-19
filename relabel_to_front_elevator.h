#include <list>

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
}