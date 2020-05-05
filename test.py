# use subprocess
# to invoke each method
# and compare the result
import subprocess
import os

def get_result(method):
    cmd = ['./lgf_compute', '--method', method, '--filename', 'test_graph.lgf']
    output_obj = subprocess.run(cmd,
                 cwd=os.getcwd(),
                 stdout=subprocess.PIPE)
    return output_obj.stdout.decode('utf-8')

def check_result(string):
    flow_value = string.split(':')[-1].rstrip('\n').lstrip(' ')
    if int(flow_value) != 96:
        raise ValueError(flow_value)
 
if __name__ == '__main__':
    method_list = ['o_hl', 'rtf', 'hl', 'fifo']
    for method in method_list:
        result_str = get_result(method)
        check_result(result_str)
    print("All checks have passed")