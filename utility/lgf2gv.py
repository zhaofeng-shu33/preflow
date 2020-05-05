'''This program converts LGF graph format(LEMON C++) to gv format (graphviz)
currently only digraph convertion is supported.
'''
import argparse
import networkx as nx

def get_readline_generator(str):
    str_inner = str
    if(str[-1] != '\n'):
        str_inner += '\n'
    str_inner += '@attributes'
    for i in str_inner.split('\n'):
        yield i
        
def toNetworkX(contents):
    dot = nx.DiGraph()
    f = get_readline_generator(contents)
    f.__next__()
    st = f.__next__()
    while(True):
        if(st.find('@arcs') >= 0):
            break
        if(st.find('label') >= 0):
            st = f.__next__()
            continue
        node_label = st.strip()
        dot.add_node(node_label)
        st = f.__next__()
    st = f.__next__()
    while(True):
        if(st.find('@attributes') >= 0):
            break
        if(st.find('label') >= 0):
            st = f.__next__()
            continue
        s,t,_,c = st.strip().split('\t')
        dot.add_edge(s, t, label=str(c))
        st = f.__next__()
    if(st.find('source') >= 0):
        _,s_id = st.strip().split(' ')
        dot.nodes[s_id]['label'] = 's'    
        st = f.__next__()
        if(st.find('target') >= 0):
            _,t_id = st.strip().split(' ')
            dot.nodes[t_id]['label'] = 't'
    return dot

def get_contents(filename):
    with open(filename) as f:
        st = f.read()
    return st
    
def convert(filename):
    contents = get_contents(filename)    
    dot = toNetworkX(contents)
    nx.drawing.nx_pydot.write_dot(dot, filename.replace('lgf','gv'))
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser('lgf to gv')
    parser.add_argument('filename', help='LGF file to be converted')
    args = parser.parse_args()
    convert(args.filename)