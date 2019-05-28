Sometimes we need plot the graph computed in C++. Currently our method is to dump the LEMON graph file in lgf format and use an utility script to convert the format to graphviz format.

sample usage:

```python
python lgf2gv.py graph_dump.lgf
```

