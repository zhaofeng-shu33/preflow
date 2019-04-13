#include <gtest/gtest.h>
#include <lemon/concepts/digraph.h>
#include <lemon/list_graph.h>
#include <lemon/preflow.h>
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

TEST(Preflow_Relabel, Run){
    typedef ListDigraph Digraph;
    typedef int T;
    typedef Digraph::ArcMap<T> ArcMap;    
    typedef ListDigraph::Node Node; 
    typedef ListDigraph::Arc Arc;
    Digraph g;
    Node n0 = g.addNode();
    Node n1 = g.addNode();
    Node n2 = g.addNode();
    Node n3 = g.addNode();
    Node n4 = g.addNode();
    Node n5 = g.addNode();   
    ArcMap aM(g);
    Arc a1 = g.addArc(n0, n1);
    Arc a2 = g.addArc(n0, n2);
    Arc a3 = g.addArc(n1, n2);
    Arc a4 = g.addArc(n2, n4);
    Arc a5 = g.addArc(n3, n5);
    Arc a6 = g.addArc(n4, n5);
    aM[a1] = 2;
    aM[a2] = 9;
    aM[a3] = 1;
    aM[a4] = 7;
    aM[a5] = 7;
    aM[a6] = 4;    
    Preflow_Relabel<Digraph, ArcMap> pf_relabel(g, aM, n0, n5);  
    pf_relabel.runMinCut();
    Preflow<Digraph, ArcMap> pf(g, aM, n0, n5);
    pf.runMinCut();
    EXPECT_EQ(pf_relabel.flowValue(), pf.flowValue());
}