#include <lemon/preflow.h>
namespace lemon{
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

        public:     
            Preflow_Relabel(const Digraph& digraph, const CapacityMap& capacity, Node source, Node target){}
    };

}
