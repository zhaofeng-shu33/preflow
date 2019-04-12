#include <lemon/preflow.h>
namespace experiment{
    template <typename GR,
              typename CAP = typename GR::template ArcMap<int>,
              typename TR = lemon::PreflowDefaultTraits<GR, CAP> >
    class Preflow_Relabel{
        private:

            TEMPLATE_DIGRAPH_TYPEDEFS(Digraph);        
        
        public:
            // for linked list we use std::list<Node>
            typedef TR Traits;
            typedef typename Traits::Digraph Digraph;
            typedef typename Traits::CapacityMap CapacityMap;
            typedef typename Traits::Value Value;
            typedef typename Traits::FlowMap FlowMap;
            typedef std::list<Node> Elevator;
            typedef typename Traits::Tolerance Tolerance;        
    };

}