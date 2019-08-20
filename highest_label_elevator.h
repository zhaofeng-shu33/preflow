#include <vector>
namespace lemon {
    template<class GR, class Item>
    class HLElevator {        

    public:
        typedef int Value;
        typedef typename std::list<Item>::iterator iterator;
        typedef typename GR::NodeIt NodeIt;
    private:
        typedef typename ItemSetTraits<GR, Item>::
            template Map<int>::Type IntMap;
        typedef typename ItemSetTraits<GR, Item>::
            template Map<bool>::Type BoolMap;

        const GR& _graph;
        int _max_level;
        int highest_active_level = 0;
        IntMap _level;
        std::vector<std::list<Item>> hl_list;

    public:
        HLElevator(const GR& graph, int max_level)
            : _graph(graph), _max_level(max_level),
            _level(graph), _init_level(0) {
            hl_list.resize(max_level * 2);
        }

        HLElevator(const HLElevator& ele) :
            _graph(ele._graph), _max_level(ele._max_level), hl_list(ele.hl_list),
            _level(ele._graph) {
            for (NodeIt n(_graph); n != INVALID; ++n) {
                _level[n] = ele._level[n];
            }
        }

        void activate(Item i) {
            int level = _level[i];
            if (level > highest_active_level)
                highest_active_level = level;
            if (level < _max_level && level > highest_active_level_limited)
                highest_active_level_limited = level;
            hl_list[level].push_back(i);
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

        bool get_node_with_highest_label(Item& item, bool limit_max_level = false) {
            int* hal;
            if (limit_max_level)
                hal = &highest_active_level_limited;
            else
                hal = &highest_active_level;
            if (*hal < 0)
                return false;
            while (*hal >= 0 ) {
                item_list = hl_list[*hal];
                if(item_list.size() > 0)
                    break;
                (*hal)--;                
            }
            if (*hal < 0)
                return false;
            item = item_list.front();
            item_list.pop_front();
            (*hal)--;
            return true;
        }
    private:

        int _init_level = 0;
        int highest_active_level_limited = 0;
    public:

        void initStart() {
            for (typename ItemSetTraits<GR, Item>::ItemIt i(_graph);
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