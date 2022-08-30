import argparse
import sys
import os


def import_vertex(data_dir):
    csv_file = data_dir + '/vertex.csv'
    conf_file = data_dir + '/vertex.conf'
    db_dir = data_dir + '/testdb'
    header = 'LABEL=vertex\nuri:STRING:ID\n'
    f = open(conf_file, 'w+')
    f.write(header)
    f.close()
    f = open(data_dir + '/import.config', 'w+')
    f.write('[vertex.csv]\n')
    f.write(header)
    f.close()
    cmd = 'lgraph_import -c %s -f %s --dir %s' % (conf_file, csv_file, db_dir)
    print
    cmd
    sys.stdout.flush()
    os.system(cmd)


def escape(file_name):
    pos = file_name.find('$')
    if pos < 0: return file_name
    return file_name[:pos] + '\\' + file_name[pos:]


def import_edge(data_dir):
    for file in os.listdir(data_dir):
        if os.path.splitext(file)[-1] != ".csv" or file == 'vertex.csv':
            continue
        csv_file = data_dir + '/' + file
        conf_file = data_dir + '/edge.conf'
        header = 'LABEL=%s,SRC_ID=vertex:uri,DST_ID=vertex:uri\n' \
                 'SRC_ID,DST_ID\n' % (os.path.splitext(file)[0])
        f = open(conf_file, 'w+')
        f.write(header)
        f.close()
        f = open(data_dir + '/import.config', 'a+')
        f.write('\n[' + file + ']\n')
        f.write(header)
        f.close()
        csv_file = escape(csv_file)
        db_dir = data_dir + '/testdb'
        cmd = 'lgraph_import -c %s -f %s --dir %s' % (conf_file, csv_file, db_dir)
        print
        cmd
        sys.stdout.flush()
        os.system(cmd)


def import_neo4j(data_dir):
    v_csv = data_dir + '/vertex.csv'
    v_head = data_dir + '/vertex-header.csv'
    header = 'uri:ID,:LABEL\n'
    f = open(v_head, 'w+')
    f.write(header)
    f.close()
    e_csv = data_dir + '/edge.csv'
    e_head = data_dir + '/edge-header.csv'
    header = ':START_ID,:END_ID,:TYPE'
    f = open(e_head, 'w+')
    f.write(header)
    f.close()
    cmd = 'neo4j-admin import --nodes "%s,%s" --relationships "%s,%s"' % (v_head, v_csv, e_head, e_csv)
    print
    cmd
    sys.stdout.flush()
    os.system(cmd)


ap = argparse.ArgumentParser(description='Import CSV Files into LGraph or Neo4j')
ap.add_argument('-i', dest='input', action='store', help='input directory')
ap.add_argument('-n', '--neo4j', action='store_true', help='import into neo4j')
args = ap.parse_args()
data_dir = args.input

if args.neo4j:
    import_neo4j(data_dir)
else:
    import_vertex(data_dir)
    import_edge(data_dir)
