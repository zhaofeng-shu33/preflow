#include <gtest/gtest.h>
#include <lemon/concepts/digraph.h>
#include <lemon/list_graph.h>
#include "mf_base.h"
using namespace lemon;
TEST(Preflow_Relabel, Constructor) {
    typedef concepts::Digraph Digraph;
    typedef double T;
    typedef Digraph::ArcMap<T> ArcMap;
    Digraph g;
    ArcMap aM(g);
    Preflow_Relabel<Digraph, ArcMap> pf(g, aM, INVALID, INVALID);
}

TEST(RelabelElevator, Constructor) {
    typedef concepts::Digraph Digraph;
    typedef Digraph::Node Item;
    Digraph g;
    RelabelElevator<Digraph, Item> re(g, 0);
}

TEST(RelabelElevator, MoveToFront) {
    typedef ListDigraph Digraph;
    typedef ListDigraph::Node Item;
    typedef RelabelElevator<Digraph, Item> RelabelElevator;
    Digraph g;
    Item a = g.addNode();
    Item b = g.addNode();
    RelabelElevator re(g, 2);
    re.initStart();
    re.initAddItem(a);
    re.initNewLevel();
    re.initAddItem(b);
    re.initFinish();
    EXPECT_EQ(re[a], 0);
    EXPECT_EQ(re[b], 1);
    re.activate(a);
    EXPECT_TRUE(re.active(a));
    re.deactivate(a);
    EXPECT_FALSE(re.active(a));
    RelabelElevator::iterator re_it = re.begin();
    EXPECT_EQ(g.id(*re_it), g.id(a));
    re_it++;
    EXPECT_EQ(g.id(*re_it), g.id(b));
    re.moveToFront(re_it);
    RelabelElevator::iterator re_it_2 = re.begin();
    EXPECT_EQ(g.id(*re_it_2), g.id(b));
    re_it_2++;
    EXPECT_EQ(g.id(*re_it_2), g.id(a));    
}
