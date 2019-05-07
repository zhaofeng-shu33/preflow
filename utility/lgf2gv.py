'''This program converts LGF graph format(LEMON C++) to gv format (graphviz)
currently only digraph convertion is supported.
'''

import argparse
import networkx as nx
import pdb

def convert(filename):
    dot = nx.DiGraph()
    with open(filename) as f:
        st = f.readline()
        st = f.readline()
        while(True):
            if(st.find('@arcs')>=0):
                break
            if(st.find('label')>=0):
                st = f.readline()
                continue
            node_label = st.strip()
            dot.add_node(node_label)
            st = f.readline()
        st = f.readline()
        while(True):
            if(st.find('@attributes')>=0):
                break
            if(st.find('label')>=0):
                st = f.readline()
                continue
            s,t,_,c = st.strip().split('\t')
            dot.add_edge(s,t,label=str(c))
            st = f.readline()
        if(st.find('source')>=0):
            _,s_id = st.strip().split(' ')
            dot.nodes[s_id]['label'] = 's'
        st = f.readline()
        if(st.find('target')>=0):
            _,t_id = st.strip().split(' ')
            dot.nodes[t_id]['label'] = 't'
    nx.drawing.nx_pydot.write_dot(dot, filename.replace('lgf','gv'))
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser('lgf to gv')
    parser.add_argument('filename', help='LGF file to be converted')
    parser.add_argument('--debug', help='enter debug mode', default=False, type=bool, nargs='?', const=True)
    args = parser.parse_args()
    if(args.debug):
        pdb.set_trace()
    convert(args.filename)