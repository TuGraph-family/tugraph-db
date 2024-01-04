import json
import random
import csv


def get_random_vertex_id(label, vertex_db):
    if label not in vertex_db:
        return None
    random_index = random.randint(0, len(vertex_db[label]) - 1)
    return vertex_db[label][random_index]


def handle_vertex(vertex_conf, vertex_db):
    file_name = vertex_conf['path'].replace('../../test/resource/data/mini_snb/', '')
    label = vertex_conf['label']
    if label not in vertex_db:
        vertex_db[label] = []
    with open(file_name, 'r') as f:
        spamreader = csv.reader(f, skipinitialspace=True)
        for line in spamreader:
            vertex_id = line[0]
            vertex_db[label].append(vertex_id)


def handle_edge(edge_conf, vertex_db):
    file_name = edge_conf['path'].replace('../../test/resource/data/mini_snb/', '')
    src_label = edge_conf['SRC_ID']
    dst_label = edge_conf['DST_ID']
    src_index = edge_conf['columns'].index('SRC_ID')
    dst_index = edge_conf['columns'].index('DST_ID')
    link_list = {
        "comment_hasCreator_person.csv": "Comment",
        "comment_isLocatedIn_place.csv": "Comment",
        "forum_containerOf_post.csv": "Post",
        "forum_hasModerator_person.csv": "Forum",
        "organisation_isLocatedIn_place.csv": "Organisation",
        "person_isLocatedIn_place.csv": "Person",
        "post_hasCreator_person.csv": "Post",
        "post_isLocatedIn_place.csv": "Post",
        "tag_hasType_tagclass.csv": "Tag"
    }
    link_label = None
    if file_name in link_list:
        link_label = link_list[file_name]
    with open(file_name, 'r') as f:
        new_edge = []
        spamreader = csv.reader(f, skipinitialspace=True)
        for line in spamreader:
            if link_label is None or link_label != src_label:
                line[src_index] = get_random_vertex_id(src_label, vertex_db)
            if link_label is None or link_label != dst_label:
                line[dst_index] = get_random_vertex_id(dst_label, vertex_db)
            for i, field in enumerate(line):
                if ',' in field:
                    line[i] = "\"" + field + "\""
            new_edge.append(','.join(line))
    with open(file_name, 'w') as f:
        f.write('\n'.join(new_edge))


def handle_file(file_conf, vertex_db):
    if 'SRC_ID' not in file_conf:
        handle_vertex(file_conf, vertex_db)
    else:
        handle_edge(file_conf, vertex_db)


if __name__ == '__main__':
    conf = 'mini_snb.json'
    vertex_db = {}
    with open(conf, 'r') as f:
        content = f.read()
        j = json.loads(content)
        for file in j['files']:
            handle_file(file, vertex_db)
